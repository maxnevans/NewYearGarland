#pragma once

#include "Logger.h"
#include "pch.h"

class GarlandApp
{
public:
    using ReportStoppingFunc = void (*)(DWORD);
    struct ClientProcArguments
    {
        ClientProcArguments(const std::wstring& pipeName, Logger& logger)
            :
            pipe(pipeName),
            logger(logger)
        {}
        NamedPipe pipe;
        Logger& logger;
    };

    using ClientThread = Thread<ClientProcArguments*>;

    struct Client
    {
        Client(ClientThread::ThreadProc proc, const std::wstring& pipeName, Logger& logger)
            :
            args(pipeName, logger),
            thread(proc, &args)
        {
        }
        ClientThread thread;
        ClientProcArguments args;
    };

    struct ServerProcArguments
    {
        ServerProcArguments(Logger& logger,
            std::vector<std::shared_ptr<Client>>& clients)
            :
            logger(logger),
            clients(clients)
        {
        }
        Logger& logger;
        std::vector<std::shared_ptr<Client>>& clients;
        Mutex clientsMutex;
    };
    
    using ServerThread = Thread<ServerProcArguments*>;

    struct Server
    {
        Server(ServerThread::ThreadProc proc, Logger& logger, 
            std::vector<std::shared_ptr<Client>>& clients)
            :
            args(logger, clients),
            thread(proc, &args)
        {
        }

        ServerThread thread;
        Mutex mutex;
        ServerProcArguments args;
    };
    
public:
    GarlandApp(Logger& logger);
    GarlandApp(const GarlandApp&) = delete;
    GarlandApp& operator=(const GarlandApp&) = delete;
    void main(Event& stopEvent, ReportStoppingFunc reportStopping);
    static void clientThreadProc(Event& stopEvent, ClientProcArguments* args);
    static void serverThreadProc(Event& stopEvent, ServerProcArguments* args);
    void createServer(Event& stopEvent);
    bool stopServers(DWORD waitMilliseconds = 3000);

private:
    static constexpr const wchar_t* PIPE_NAME = L"NewYearGarlandService";
    Logger& m_Logger;
    std::vector<std::shared_ptr<Client>> m_Clients;
    std::vector<std::shared_ptr<Server>> m_Servers;
};