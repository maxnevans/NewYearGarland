#pragma once
#include "system/Event.h"
#include "Logger.h"

class GarlandApp
{
public:
    GarlandApp(Logger& logger);
    GarlandApp(const GarlandApp&) = delete;
    GarlandApp& operator=(const GarlandApp&) = delete;
    void main(Event& stopEvent);

private:
    Logger& const m_Logger;
};