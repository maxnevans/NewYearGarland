#include "pch.h"

#include "UnclassifiedException.h"

#define SERVICE_NAME L"New Year Garland"
#define SERVICE_DISPLAY_NAME L"New Year Garland"

#define PRINT_ERROR(functionName) std::wcout << L"Failed "\
    << (functionName) << L": " << GetLastError() << L'\n'

HANDLE hLogFile = INVALID_HANDLE_VALUE;

void log(HANDLE hLogFile, std::string message)
{
    DWORD nobw;
    WriteFile(hLogFile, message.c_str(), message.size(), &nobw, NULL);
    FlushFileBuffers(hLogFile);
}

SERVICE_STATUS_HANDLE g_HServiceStatus = 0;
HANDLE g_HServiceStopEvent = NULL;
SERVICE_STATUS g_ServiceStatus = {};

void reportServiceStatus(DWORD dwCurrentState, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    g_ServiceStatus.dwCurrentState = dwCurrentState;
    g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    g_ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        g_ServiceStatus.dwControlsAccepted = 0;
    else g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        g_ServiceStatus.dwCheckPoint = 0;
    else g_ServiceStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    auto ret = SetServiceStatus(g_HServiceStatus, &g_ServiceStatus);

    log(hLogFile, "In reportServiceStatus SetServiceStatus result: " + std::to_string(ret) + '\n');
}

void serviceInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    log(hLogFile, "serviceInit start...\n");

    g_HServiceStopEvent = CreateEvent(
        NULL,       // sec attrs: default
        TRUE,       // is manually reset
        FALSE,      // is signaled
        NULL        // name: anonymous
    );

    if (g_HServiceStopEvent == NULL)
    {
        reportServiceStatus(SERVICE_STOPPED, 0);
        return;
    }

    reportServiceStatus(SERVICE_RUNNING, 0);

    bool shouldStop = false;
    while (!shouldStop)
    {
        // TODO: Logic here

        auto waitRet = WaitForSingleObject(g_HServiceStopEvent, 3000);
        
        switch (waitRet)
        {
            /* Stop signaled */
            case WAIT_OBJECT_0:
            {
                reportServiceStatus(SERVICE_STOPPED, 0);
                shouldStop = true;
                break;
            }

            case WAIT_FAILED:
            {
                reportServiceStatus(SERVICE_STOPPED, 0);
                shouldStop = true;
                break;
            }
        }
    }
}

void WINAPI serviceControlHandler(DWORD dwCtrl)
{
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        reportServiceStatus(SERVICE_STOP_PENDING, 0);
        SetEvent(g_HServiceStopEvent);
        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

void WINAPI serviceMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    log(hLogFile, "ServiceMain start...");
    g_HServiceStatus = RegisterServiceCtrlHandler(
        SERVICE_NAME, serviceControlHandler);

    log(hLogFile, "After RegisterServiceCtrlHandler\n");

    if (g_HServiceStatus == 0)
    {
        using namespace std::string_literals;
        //reportServiceStatus(SERVICE_STOPPED, 0);
        log(hLogFile, "Error RegisterServiceCtrlHandler: "s + std::to_string(GetLastError()) + '\n');
        return;
    }

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;

    log(hLogFile, "Before reportServiceStatus\n");
    
    reportServiceStatus(SERVICE_START_PENDING, 3000);

    log(hLogFile, "After reportServiceStatus\n");
    log(hLogFile, "Before serviceInit\n");

    serviceInit(dwArgc, lpszArgv);
}

void serviceInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    wchar_t szPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        PRINT_ERROR(L"GetModuleFileName");
        return;
    }

    std::wcout << L"Path to executable service file: " << szPath << L'\n';

    schSCManager = OpenSCManager(
        NULL,                   // local computer
        NULL,                   // default scm database
        SC_MANAGER_ALL_ACCESS   // access rights
    );

    if (NULL == schSCManager)
    {
        PRINT_ERROR(L"OpenSCManager");
        return;
    }

    schService = CreateService(
        schSCManager,              // SCM database 
        SERVICE_NAME,              // name of service 
        SERVICE_DISPLAY_NAME,      // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL                       // no password 
    );

    if (schService == NULL)
    {
        PRINT_ERROR(L"CreateService");
        CloseServiceHandle(schSCManager);
        return;
    }
    else
    {
        std::wcout << L"Service successfully installed!\n";
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

void serviceDelete()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS    // full access rights 
    );

    if (NULL == schSCManager)
    {
        PRINT_ERROR(L"OpenSCManager");
        return;
    }

    schService = OpenService(
        schSCManager,       // SCM database 
        SERVICE_NAME,       // name of service 
        DELETE              // need delete access 
    );            

    if (schService == NULL)
    {
        PRINT_ERROR(L"OpenService");
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!DeleteService(schService))
    {
        PRINT_ERROR(L"DeleteService");
    }
    else
    {
        std::wcout << L"Service successfully deleted!\n";
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

void waitForStatusChange(SC_HANDLE schService, SERVICE_STATUS_PROCESS& ssStatus, 
    DWORD dwStatusToBeChanged)
{
    DWORD dwStartTickCount = GetTickCount();
    DWORD dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (ssStatus.dwCurrentState == dwStatusToBeChanged)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 

        DWORD dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending. 

        DWORD dwBytesNeeded;
        if (!QueryServiceStatusEx(
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE)&ssStatus,              // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))                // size needed if buffer is too small
        {
            using namespace std::string_literals;
            throw UnclassifiedException(L"Failed QueryServiceStatusEx: "s + std::to_wstring(GetLastError()));
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
            {
                throw UnclassifiedException(L"Timeout waiting for service to change status!");
            }
        }
    }
}

void serviceStart()
{
    SC_HANDLE schSCManager = OpenSCManager(
        NULL,                                   // local computer
        NULL,                                   // servicesActive database 
        SC_MANAGER_ALL_ACCESS                   // full access rights 
    );

    if (NULL == schSCManager)
    {
        PRINT_ERROR(L"OpenSCManager");
        return;
    }

    SC_HANDLE schService = OpenService(
        schSCManager,               // SCM database 
        SERVICE_NAME,               // name of service 
        SERVICE_ALL_ACCESS          // full access 
    );

    if (schService == NULL)
    {
        PRINT_ERROR(L"OpenService");
        CloseServiceHandle(schSCManager);
        return;
    }

    // Query status to check if the service is already running.
    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwBytesNeeded;
    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // information level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // size needed if buffer is too small
    {
        PRINT_ERROR(L"QueryServiceStatusEx");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check if the service is already running. It would be possible 
    // to stop the service here, but for simplicity this example just returns. 

    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        std::wcout << L"Failed to start service because it is already running!\n";
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Wait while service is stoppping.

    std::wcout << L"Waiting for the service to stop...\n";
    try
    {
        waitForStatusChange(schService, ssStatus, SERVICE_STOP_PENDING);
    }
    catch (UnclassifiedException& ex)
    {
        std::wcout << ex.what().c_str() << L'\n';
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    }

    // Attempt to start the service.

    if (!StartService(
        schService,     // handle to service 
        0,              // number of arguments 
        NULL            // no arguments 
    ))
    {
        PRINT_ERROR(L"StartService");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }
    else
    {
        std::wcout << L"Service start pending...\n";
    }

    // Check the status until the service is no longer start pending. 

    if (!QueryServiceStatusEx(
        schService,                             // handle to service 
        SC_STATUS_PROCESS_INFO,                 // info level
        reinterpret_cast<LPBYTE>(&ssStatus),    // address of structure
        sizeof(SERVICE_STATUS_PROCESS),         // size of structure
        &dwBytesNeeded                          // if buffer too small
    ))
    {
        PRINT_ERROR(L"QueryServiceStatusEx");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    std::wcout << L"Waiting for the service to start...\n";
    try
    {
        waitForStatusChange(schService, ssStatus, SERVICE_START_PENDING);
    }
    catch (UnclassifiedException& ex)
    {
        std::wcout << ex.what().c_str() << L'\n';
    }

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
    {
        std::wcout << L"Service is successfully running!\n";
    }
    else
    {
        std::wcout << L"Service not started.\n"
            << L"   Current state: " << ssStatus.dwCurrentState << L'\n'
            << L"   Exit Code: " << ssStatus.dwWin32ExitCode << L'\n'
            << L"   Check Point: " << ssStatus.dwCheckPoint << L'\n'
            << L"   Wait Hint: " << ssStatus.dwWaitHint << L'\n';
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

int wmain(int argc, wchar_t* argv[])
{
    if (lstrcmpi(argv[1], L"install") == 0)
    {
        serviceInstall();
        return 0;
    }

    if (lstrcmpi(argv[1], L"delete") == 0)
    {
        serviceDelete();
        return 0;
    }

    if (lstrcmpi(argv[1], L"start") == 0)
    {
        serviceStart();
        return 0;
    }

    if (lstrcmpi(argv[1], L"stop") == 0)
    {
        //serviceStop();
        std::wcout << L"Stop is not implemented yet.";
        return 0;
    }

    // Initializing log file

    wchar_t szPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        reportServiceStatus(SERVICE_STOPPED, 0);
        return -1;
    }

    PathRemoveFileSpec(szPath);
    wchar_t szLogFilePath[MAX_PATH];
    PathCombine(szLogFilePath, szPath, L"logs.txt");

    hLogFile = CreateFile(
        szLogFilePath,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );


    wchar_t serviceName[] = SERVICE_NAME;

    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { serviceName, static_cast<LPSERVICE_MAIN_FUNCTION>(serviceMain) },
        { NULL, NULL }
    };

    log(hLogFile, "Before StartServiceCtrlDispatcher\n");

    if (StartServiceCtrlDispatcher(DispatchTable) == 0)
    {
        using namespace std::string_literals;
        log(hLogFile, "Error StartServiceCtrlDispatcher: "s + std::to_string(GetLastError()) + '\n');
        reportServiceStatus(SERVICE_STOPPED, 0);
        return -1;
    }

    log(hLogFile, "After StartServiceCtrlDispatcher\n");

    return 0;
}