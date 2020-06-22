#pragma once
#include "pch.h"

#include "Logger.h"

/**
* All members are static to allow to use logger inside static WINAPI functions.
*/

class GarlandService
{
public:
    static void start(Logger& logger);
    static void reportFail();
    static std::wstring getLogsPath();

protected:
    static void CALLBACK serviceMain(DWORD dwArgc, LPWSTR* lpszArgv);
    static void CALLBACK serviceControlHandler(DWORD dwCtrl);
    static void reportStatus(DWORD dwCurrentState, DWORD dwWaitHint);
    static void serviceInit(DWORD dwArgc, LPWSTR* lpszArgv);
    static void reportStopping(DWORD waitHint);
    
public:
    static constexpr const wchar_t* const SERVICE_NAME = L"NewYearGarland";
    static constexpr const wchar_t* const SERVICE_DISPLAY_NAME = L"New Year Garland";

private:
    static SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
    static SERVICE_STATUS m_ServiceStatus;
    static DWORD m_CheckPoint;
    static std::optional<Event> m_StopEvent;
    static Logger* m_Logger;
};