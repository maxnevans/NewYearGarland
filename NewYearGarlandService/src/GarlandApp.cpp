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
    auto& pipe = client->args.pipe;
    auto& logger = client->args.logger;

    logger.info(THREAD_MSG(thread, L"Client thread started."));

    logger.info(THREAD_MSG(thread, L"Waiting for client to send message."));
    ClientMessage msg = pipe.read<ClientMessage>();
    logger.info(THREAD_MSG(thread, L"Message retrieved."));

    ServerMessage answer;
    answer.type = ServerMessageType::CONNECT;
    answer.connect.color = { 10, 20, 30 };

    logger.info(THREAD_MSG(thread, L"Sending message to client."));
    pipe.write(answer);
    logger.info(THREAD_MSG(thread, L"Message sent."));
    logger.info(THREAD_MSG(thread, L"Connection closed."));
}

void GarlandApp::serverThreadProc(Event& stopEvent, Server* server)
{
    auto& logger = server->args.logger;
    auto& clients = server->args.clients;
    auto& clientsMutex = server->args.clientsMutex;
    auto& thread = server->thread;

    logger.info(THREAD_MSG(thread, L"Server started."));

    while (!stopEvent.check())
    {
        auto client = std::make_shared<Client>(clientThreadProc, PIPE_NAME, logger);

        logger.info(THREAD_MSG(thread, L"Listenining for client to connect..."));
        client->args.pipe.listen();
        logger.info(THREAD_MSG(thread, L"Client connected."));

        {
            MutexGuard m(clientsMutex);
            clients.push_back(client);
        }

        client->thread.start();
    }

    logger.info(THREAD_MSG(thread, L"Server stopped."));
}

void GarlandApp::createServer(Event& stopEvent)
{
    auto& server = m_Servers.emplace_back(new Server(serverThreadProc, m_Logger, m_Clients));
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
