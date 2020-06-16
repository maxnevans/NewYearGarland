#include "pch.h"
#include "GarlandApp.h"

GarlandApp::GarlandApp(Logger& logger)
    :
    m_Logger(logger)
{
}

void GarlandApp::main(Event& stopEvent)
{
    bool shouldStop = false;
    while (!shouldStop)
    {
        if (stopEvent.check())
            shouldStop = true;

        HANDLE hPipe = CreateNamedPipe(
            L"\\\\.\\pipe\\NewYearGarlandService",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0, 0, 0, NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE)
            return m_Logger.error(Win32Exception(L"CreateNamedPipe").what());

        m_Logger.info(L"Waiting for client to be connected.");

        if (ConnectNamedPipe(hPipe, NULL) == 0)
            return m_Logger.error(Win32Exception(L"ConnectNamedPipe").what());

        m_Logger.info(L"Client connected.");
        m_Logger.info(L"Waiting for client to send message.");

        ClientMessage msg;
        
        DWORD numberOfBytesRead;
        if (ReadFile(hPipe, &msg, sizeof(msg), &numberOfBytesRead, NULL) == FALSE || numberOfBytesRead != sizeof(msg))
        {
            return m_Logger.error(Win32Exception(L"ReadFile").what());
        }

        ServerMessage answer;
        answer.type = ServerMessageType::CONNECT;
        answer.connect.color = {10, 20, 30};

        m_Logger.info(L"Message retrieved.");
        m_Logger.info(L"Sending message to client.");

        DWORD numberOfBytesWritten;
        if (WriteFile(hPipe, &answer, sizeof(answer), &numberOfBytesWritten, NULL) == FALSE
            || numberOfBytesWritten != sizeof(answer))
            return m_Logger.error(Win32Exception(L"WriteFile").what());

        CloseHandle(hPipe);

        m_Logger.info(L"Connection closed.");
    }
}
