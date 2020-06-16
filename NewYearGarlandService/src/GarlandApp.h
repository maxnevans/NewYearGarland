#pragma once

#include "Logger.h"

class GarlandApp
{
public:
    GarlandApp(Logger& logger);
    GarlandApp(const GarlandApp&) = delete;
    GarlandApp& operator=(const GarlandApp&) = delete;
    void main(Event& stopEvent);

private:
    static constexpr const wchar_t* PIPE_NAME = L"NewYearGarlandService";
    Logger& m_Logger;
};