#pragma once

#include "Logger.h"
#include "pch.h"

class GarlandApp
{
public:
    using ReportStoppingFunc = void (*)(DWORD);

    struct Client;
    using ClientThread = Thread<Client*>;

    struct Client
    {
        Client(ClientThread::ThreadProc proc, const std::wstring& pipeName, 
            Logger& logger, size_t index, std::stack<size_t>& unusedClientsQueue, Mutex& unusedClientsStackMutex)
            :
            pipe(pipeName),
            logger(logger),
            thread(proc, this),
            index(index),
            unusedClientsStack(unusedClientsQueue),
            unusedClientsStackMutex(unusedClientsStackMutex)
        {
        }
        ClientThread thread;
        NamedPipe pipe;
        Logger& logger;
        size_t index;
        Mutex& unusedClientsStackMutex;
        std::stack<size_t>& unusedClientsStack;
    };
    
    struct Server;
    using ServerThread = Thread<Server*>;

    struct Server
    {
        Server(ServerThread::ThreadProc proc, Logger& logger, 
            std::vector<std::shared_ptr<Client>>& clients, std::stack<size_t>& unusedClientsStack)
            :
            logger(logger), 
            clients(clients),
            thread(proc, this),
            unusedClientsStack(unusedClientsStack)
        {
        }

        ServerThread thread;
        Mutex mutex;
        Logger& logger;
        Mutex clientsMutex;
        std::vector<std::shared_ptr<Client>>& clients;
        Mutex unusedClientsStackMutex;
        std::stack<size_t>& unusedClientsStack;
    };
    
public:
    GarlandApp(Logger& logger);
    GarlandApp(const GarlandApp&) = delete;
    GarlandApp& operator=(const GarlandApp&) = delete;
    void main(Event& stopEvent, ReportStoppingFunc reportStopping);
    static void clientThreadProc(Event& stopEvent, Client* args);
    static void serverThreadProc(Event& stopEvent, Server* args);
    void createServer(Event& stopEvent);
    bool stopServers(DWORD waitMilliseconds = 3000);

private:
    static constexpr const wchar_t* PIPE_NAME = L"NewYearGarlandService";
    Logger& m_Logger;
    std::vector<std::shared_ptr<Client>> m_Clients;
    std::vector<std::shared_ptr<Server>> m_Servers;
    std::stack<size_t> m_UnusedClientsStack;
};