#include "pch.h"
#include "GarlandApp.h"

#define THREAD_MSG(thread, msg) L"Thread ID (" + std::to_wstring(thread.getId()) + L") " + msg

GarlandApp::GarlandApp(Logger& logger)
    :
    m_Logger(logger),
    m_Garland(logger)
{
}

void GarlandApp::main(Event& stopEvent, ReportStoppingFunc reportStopping)
{
    try
    {
        m_Logger.info(L"Creating server.");
        createServer(stopEvent);

        m_Logger.info(L"Starting stop wait loop.");

        m_Logger.info(L"Staring garland.");

        m_Garland.start(stopEvent);

        const DWORD stoppingTimout = 3000;
        reportStopping(stoppingTimout);

        m_Logger.info(L"Stopping servers.");
        if (!stopServers(stoppingTimout))
            m_Logger.warn(L"Failed to stop servers in " + std::to_wstring(stoppingTimout));

        m_Logger.info(L"Stopping garland.");
        m_Garland.stop();

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

    auto onLightOn = [&logger, &pipe, &garland, &thread]()
    {
        {
            ServerMessage msg;
            msg.type = ServerMessageType::LIGHT;
            msg.light.isPowered = true;

            logger.info(THREAD_MSG(thread, L"Sending `onLightOn` message to client."));
            pipe.write(msg);
            logger.info(THREAD_MSG(thread, L"Message sent."));
        }
        
    };

    auto onLightOff = [&logger, &pipe, &thread]()
    {
        ServerMessage msg;
        msg.type = ServerMessageType::LIGHT;
        msg.light.isPowered = false;

        logger.info(THREAD_MSG(thread, L"Sending `onLightOff` message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    };

    auto onColor = [&logger, &pipe, &garland, &thread](const Garland::Color& color)
    {
        ServerMessage msg;
        msg.type = ServerMessageType::COLOR;
        msg.color = { color.r, color.g, color.b };

        logger.info(THREAD_MSG(thread, L"Sending `onColor` message to client."));
        pipe.write(msg);
        logger.info(THREAD_MSG(thread, L"Message sent."));
    };

    try
    {
        auto id = garland.registerLightbulb(stopEvent, onColor, onLightOn, onLightOff);
        logger.info(THREAD_MSG(thread, L"Lightbulb registered. Delegating thread to garland."));
        garland.delegate(id);

        logger.info(THREAD_MSG(thread, L"Garland stop this lighbulb."));
    }
    catch (const Exception& ex)
    {
        logger.error(THREAD_MSG(thread, L"Lighbulb: Garland says that exception happend: " + ex.what()));
    }
    catch (...)
    {
        logger.error(THREAD_MSG(thread, L"Lightbulb: Unknown exception happend when tried to register and start garland lighbulb."));
    }

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
        }
        else
        {
            logger.info(THREAD_MSG(thread, L"Client takes new index in clients vector."));
            MutexGuard m(clientsMutex);
            client->index = clients.size();
            clients.push_back(client);
        }

        client->thread.onException([&client](const Exception& ex) {
            client->logger.error(THREAD_MSG(client->thread, L"Client thread exception happend." + ex.what()));
        });

        client->thread.start();
    }

    logger.info(THREAD_MSG(thread, L"Server stopped."));
}

void GarlandApp::createServer(Event& stopEvent)
{
    auto& server = m_Servers.emplace_back(new Server(serverThreadProc, m_Logger, m_Clients, m_UnusedClientsStack, m_Garland));
    server->thread.onException([this, &server](const Exception& ex) {
        m_Logger.error(THREAD_MSG(server->thread, L"Server thread exception happend." + ex.what()));
    });
    server->thread.start();
}

bool GarlandApp::stopServers(DWORD waitMilliseconds)
{
    auto savePoint = GetTickCount64();

    // Stopping clients
    m_Logger.debug(L"Garland::stopServers: stopping clients.");
    if (!p_StopEntityThreads(m_Clients, waitMilliseconds))
        return false;

    auto timeElapsed = GetTickCount64() - savePoint;
    long long timeRemaining = waitMilliseconds - timeElapsed;

    if (timeRemaining <= 0)
        return false;

    // Stopping servers
    m_Logger.debug(L"Garland::stopServers: stopping servers.");
    if (!p_StopEntityThreads(m_Servers, static_cast<DWORD>(timeRemaining)))
        return false;

    return true;
}
