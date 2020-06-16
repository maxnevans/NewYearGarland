#include "pch.h"
#include "GarlandApp.h"

GarlandApp::GarlandApp(Logger& logger)
    :
    m_Logger(logger)
{
}

void GarlandApp::main(Event& stopEvent)
{
    try
    {
        bool shouldStop = false;
        while (!shouldStop)
        {
            if (stopEvent.check())
                shouldStop = true;

            NamedPipe pipe(PIPE_NAME);

            m_Logger.info(L"Waiting for client to be connected.");

            pipe.listen();

            m_Logger.info(L"Client connected.");
            m_Logger.info(L"Waiting for client to send message.");

            ClientMessage msg = pipe.read<ClientMessage>();

            ServerMessage answer;
            answer.type = ServerMessageType::CONNECT;
            answer.connect.color = { 10, 20, 30 };

            m_Logger.info(L"Message retrieved.");
            m_Logger.info(L"Sending message to client.");

            pipe.write(answer);

            m_Logger.info(L"Connection closed.");
        }
    }
    catch (Win32Exception& ex)
    {
        m_Logger.error(L"(GarlandApp::main): " + ex.what());
    }
}
