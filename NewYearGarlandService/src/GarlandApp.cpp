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

        m_Logger.info(L"Test");

        Sleep(1000);
    }
}
