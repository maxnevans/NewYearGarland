#include "pch.h"
#include "GarlandApp.h"

void GarlandApp::main(Event& stopEvent)
{
    bool shouldStop = false;
    while (!shouldStop)
    {
        if (stopEvent.check())
            shouldStop = true;

        Sleep(1000);
    }
}
