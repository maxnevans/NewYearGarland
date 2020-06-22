#pragma once
#include "pch.h"

#include "Logger.h"
#include "Garland.h"

class GarlandApp
{
public:
    using ReportStoppingFunc = void (*)(DWORD);

    struct Client;
    using ClientThread = Thread<Client*>;

    struct Client
    {
        Client(ClientThread::ThreadProc proc, const std::wstring& pipeName, 
            Logger& logger, size_t index, std::stack<size_t>& unusedClientsQueue, 
            Mutex& unusedClientsStackMutex, Garland& garland)
            :
            pipe(pipeName, PIPE_ACCESS_OUTBOUND),
            logger(logger),
            thread(proc, this),
            index(index),
            unusedClientsStack(unusedClientsQueue),
            unusedClientsStackMutex(unusedClientsStackMutex),
            garland(garland)
        {
        }
        ClientThread thread;
        NamedPipe pipe;
        Logger& logger;
        size_t index;
        Mutex& unusedClientsStackMutex;
        std::stack<size_t>& unusedClientsStack;
        Garland& garland;
    };
    
    struct Server;
    using ServerThread = Thread<Server*>;

    struct Server
    {
        Server(ServerThread::ThreadProc proc, Logger& logger, 
            std::vector<std::shared_ptr<Client>>& clients, 
            std::stack<size_t>& unusedClientsStack, Garland& garland)
            :
            logger(logger), 
            clients(clients),
            thread(proc, this),
            unusedClientsStack(unusedClientsStack),
            garland(garland)
        {
        }

        ServerThread thread;
        Mutex mutex;
        Logger& logger;
        Mutex clientsMutex;
        std::vector<std::shared_ptr<Client>>& clients;
        Mutex unusedClientsStackMutex;
        std::stack<size_t>& unusedClientsStack;
        Garland& garland;
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
    /**
     * Stop entities. `TEntity` should have field `thread` of type `Thread`.
     * 
     * \param entities
     * \return false if timeout otherwise true
     * \throw Win32Exception
     */
    template<typename TEntity>
    bool p_StopEntityThreads(const std::vector<std::shared_ptr<TEntity>>& entities, DWORD waitMilliseconds)
    {
        if (entities.size() > 0)
        {
            // Create handles vector to threads
            std::vector<HANDLE> handles;
            std::transform(entities.begin(), entities.end(), std::back_inserter(handles),
                [](const std::shared_ptr<TEntity>& entity) {return entity->thread.getHandle(); });

            // Stopping threads
            std::for_each(entities.begin(), entities.end(), [](const std::shared_ptr<TEntity>& entity) {
                CancelSynchronousIo(entity->thread.getHandle());
                entity->thread.stop();
            });

            // Waiting for all to stop
            Waiter waiter(true, waitMilliseconds);
            return waiter.wait(handles);
        }

        return true;
    }

    void p_IndicateStopping();

private:
    static constexpr const wchar_t* PIPE_NAME = L"NewYearGarlandService";
    Logger& m_Logger;
    Garland m_Garland;
    std::vector<std::shared_ptr<Client>> m_Clients;
    std::vector<std::shared_ptr<Server>> m_Servers;
    std::stack<size_t> m_UnusedClientsStack;
    volatile unsigned long m_IsStopping = false;
};