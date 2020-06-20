#include "pch.h"
#include "GarlandService.h"
#include "GarlandApp.h"

SERVICE_STATUS_HANDLE GarlandService::m_ServiceStatusHandle = NULL;
SERVICE_STATUS GarlandService::m_ServiceStatus = {};
DWORD GarlandService::m_CheckPoint = 1;
std::optional<Event> GarlandService::m_StopEvent = std::nullopt;
Logger GarlandService::m_Logger = Logger(getLogsPath(), true);

void GarlandService::start()
{
    try
    {
        m_Logger.start();

        m_Logger.info(L"Service is starting...");

        std::wstring serviceName = GarlandService::SERVICE_NAME;
        SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            { serviceName.data(), static_cast<LPSERVICE_MAIN_FUNCTION>(serviceMain) },
            { NULL, NULL }
        };

        m_Logger.info(L"Starting service control dispatcher.");

        if (!StartServiceCtrlDispatcher(DispatchTable))
            throw Win32Exception(L"StartServiceCtrlDispatcher");
    }
    catch (Win32Exception& ex)
    {
        reportStatus(SERVICE_STOPPED, 0);
        m_Logger.error(L"(GarlandService::start): " + ex.what());
        m_Logger.info(L"Service stopped!");
    }
    catch (...)
    {
        reportStatus(SERVICE_STOPPED, 0);
        m_Logger.error(L"(GarlandService::start): unknown exception happend!");
        m_Logger.info(L"Service stopped!");
    }

    m_Logger.stop();
}

void GarlandService::reportFail()
{
    m_Logger.info(L"Service stopped!");
    reportStatus(SERVICE_STOPPED, 0);
}

void CALLBACK GarlandService::serviceMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    try
    {
        m_ServiceStatusHandle = RegisterServiceCtrlHandler(
            SERVICE_NAME, serviceControlHandler);

        if (m_ServiceStatusHandle == 0)
            throw Win32Exception(L"RegisterServiceCtrlHandler");

        m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        reportStatus(SERVICE_START_PENDING, 3000);

        m_Logger.info(L"Service successfully reported start pending status.");

        GarlandService::serviceInit(dwArgc, lpszArgv);
    }
    catch (Win32Exception& ex)
    {
        m_Logger.error(L"(GarlandService::serviceMain): " + ex.what());
        m_Logger.info(L"Service stopped!");
        reportStatus(SERVICE_STOPPED, 0);
    }
    catch (...)
    {
        m_Logger.error(L"(GarlandService::serviceMain): unknown exception happend!");
        m_Logger.info(L"Service stopped!");
        reportStatus(SERVICE_STOPPED, 0);
    }
}

void CALLBACK GarlandService::serviceControlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        reportStatus(SERVICE_STOP_PENDING, 3000);
        m_Logger.info(L"Service got control stop.");
        m_Logger.info(L"Stoping service. Saying that we will stop it in 3 seconds.");
        try 
        {
            m_StopEvent->emmit();
            m_Logger.info(L"Stop event emmited.");
        }
        catch (...)
        {
            m_Logger.error(L"(GarlandService::serviceControlHandler): failed to emmit stop event");
        }
        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

void GarlandService::reportStatus(DWORD dwCurrentState, DWORD dwWaitHint)
{
    m_ServiceStatus.dwCurrentState = dwCurrentState;
    m_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    m_ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        m_ServiceStatus.dwControlsAccepted = 0;
    else m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        m_ServiceStatus.dwCheckPoint = 0;
    else m_ServiceStatus.dwCheckPoint = m_CheckPoint++;

    // Report the status of the service to the SCM.
    if (!SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus))
        throw Win32Exception(L"SetServiceStatus");
}

void GarlandService::serviceInit(DWORD dwArgc, LPWSTR* lpszArgv)
{
    m_StopEvent.emplace();
    reportStatus(SERVICE_RUNNING, 0);
    m_Logger.info(L"Service successfully initialized.");

    m_Logger.info(L"Starting user main function.");
    auto app = GarlandApp(m_Logger);
    app.main(*m_StopEvent, reportStopping);
    m_Logger.info(L"User function returned.");

    m_Logger.info(L"Stopping service.");
    reportStatus(SERVICE_STOPPED, 0);
    m_Logger.info(L"Service stopped!");
}

std::wstring GarlandService::getLogsPath()
{
    wchar_t szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        return L"C:\\logs";
    }
    PathRemoveFileSpec(szPath);
    PathAppend(szPath, L"logs");

    return szPath;
}

void GarlandService::reportStopping(DWORD waitHint)
{
    reportStatus(SERVICE_STOP_PENDING, waitHint);
}
