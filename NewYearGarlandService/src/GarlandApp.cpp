#include "pch.h"
#include "GarlandApp.h"

#define THREAD_MSG(thread, msg) L"Thread ID (" + std::to_wstring(thread.getId()) + L") " + msg

GarlandApp::GarlandApp(Logger& logger)
    :
    m_Logger(logger)
{
}

void GarlandApp::main(Event& stopEvent, ReportStoppingFunc reportStopping)
{
    try
    {
        m_Logger.info(L"Creating server.");
        createServer(stopEvent);

        m_Logger.info(L"Starting stop wait loop.");

        while (!stopEvent.wait(1000));

        m_Logger.info(L"Stopping servers.");

        const DWORD stoppingTimout = 3000;

        reportStopping(stoppingTimout);
        if (!stopServers(stoppingTimout))
            m_Logger.warn(L"Failed to stop servers in " + std::to_wstring(stoppingTimout));

        m_Logger.info(L"Servers successfully stopped.");
    }
    catch (Win32Exception& ex)
    {
        m_Logger.error(L"(GarlandApp::main): " + ex.what());
    }
}

void GarlandApp::clientThreadProc(Event& stopEvent, Client* client)
{
    auto& thread = client->thread;
    auto& pipe = client->pipe;
    auto& logger = client->logger;
    auto& unusedClientsQueue = client->unusedClientsStack;
    auto& unusedClientsStackMutex = client->unusedClientsStackMutex;
    auto& garland = client->garland;

    logger.info(THREAD_MSG(thread, L"Client thread started."));

    DWORD pid = NULL;

    {
        logger.info(THREAD_MSG(thread, L"Waiting for client to send message."));
        ClientMessage msg = pipe.read<ClientMessage>();
        logger.info(THREAD_MSG(thread, L"Message retrieved."));
        pid = msg.pid;
    }

    {
        ServerMessage msg;
        msg.type = ServerMessageType::CONNECT;
        const auto color = garland.getColor(pid);
        msg.connect.color = { color.r, color.g, color.b };

        logger.info(THREAD_MSG(thread, L"Sending message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    }

    garland.start(pid);

    {
        ServerMessage msg;
        msg.type = ServerMessageType::LIGHT;
        msg.light.isPowered = true;

        logger.info(THREAD_MSG(thread, L"Sending message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    }

    garland.wait();

    {
        ServerMessage msg;
        msg.type = ServerMessageType::LIGHT;
        msg.light.isPowered = false;

        logger.info(THREAD_MSG(thread, L"Sending message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    }

    garland.stop();

    {
        ServerMessage msg;
        msg.type = ServerMessageType::DISCONNECT;

        logger.info(THREAD_MSG(thread, L"Sending message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    }

    {
        MutexGuard m(unusedClientsStackMutex);
        unusedClientsQueue.push(client->index);
    }

    logger.info(THREAD_MSG(thread, L"Thread added to unusedClientsQueue."));
    logger.info(THREAD_MSG(thread, L"Connection closed."));
}

void GarlandApp::serverThreadProc(Event& stopEvent, Server* server)
{
    auto& logger = server->logger;
    auto& clients = server->clients;
    auto& clientsMutex = server->clientsMutex;
    auto& thread = server->thread;

    logger.info(THREAD_MSG(thread, L"Server started."));

    while (!stopEvent.check())
    {
        auto client = std::make_shared<Client>(clientThreadProc, PIPE_NAME, 
            logger, 0, server->unusedClientsStack, server->unusedClientsStackMutex, server->garland);

        logger.info(THREAD_MSG(thread, L"Listenining for client to connect..."));
        client->pipe.listen();
        logger.info(THREAD_MSG(thread, L"Client connected."));

        if (!server->unusedClientsStack.empty())
        {
            logger.info(THREAD_MSG(thread, L"Client takes unused index in clients vector."));
            MutexGuard m(server->unusedClientsStackMutex);
            if (!server->unusedClientsStack.empty())
            {
                MutexGuard m(clientsMutex);
                client->index = server->unusedClientsStack.top();
                clients[client->index] = client;
            }
            server->unusedClientsStack.pop();
        }
        else
        {
            logger.info(THREAD_MSG(thread, L"Client takes new index in clients vector."));
            MutexGuard m(clientsMutex);
            client->index = clients.size();
            clients.push_back(client);
        }

        client->thread.start();
    }

    logger.info(THREAD_MSG(thread, L"Server stopped."));
}

void GarlandApp::createServer(Event& stopEvent)
{
    auto& server = m_Servers.emplace_back(new Server(serverThreadProc, m_Logger, m_Clients, m_UnusedClientsStack, m_Garland));
    server->thread.start();
}

bool GarlandApp::stopServers(DWORD waitMilliseconds)
{
    std::for_each(std::execution::par_unseq, m_Servers.begin(), m_Servers.end(), [](const std::shared_ptr<Server>& server) {
        CancelSynchronousIo(server->thread.getHandle());
    });

    auto serverThreadHandlers = std::vector<HANDLE>{};

    std::for_each(m_Servers.begin(), m_Servers.end(), [&serverThreadHandlers](const std::shared_ptr<Server>& server) {
        serverThreadHandlers.emplace_back(server->thread.getHandle());
    });

    switch (WaitForMultipleObjects(serverThreadHandlers.size(),
        serverThreadHandlers.data(), TRUE, waitMilliseconds))
    {
        case WAIT_OBJECT_0:
            return true;
        case WAIT_FAILED:
            throw Win32Exception(L"WaitForMultipleObjects");
        case WAIT_TIMEOUT:
            return false;
        case WAIT_ABANDONED:
            throw Win32Exception(L"WaitForMultipleObjects");
    }

    throw Win32Exception(L"WaitForMultipleObjects", L"unhandled unknown return code");
}
