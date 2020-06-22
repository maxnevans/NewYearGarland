#include "pch.h"

#include "Service.h"

#include "Win32Exception.h"

Service::Service(const ServiceControlManager& scManager, const std::wstring& serviceName)
{
    m_Handle = OpenService(scManager.getHandle(), serviceName.c_str(), SERVICE_ALL_ACCESS);

    if (m_Handle == NULL)
        throw Win32Exception(L"OpenService");
}


/* Protected constructor for inner intialization */
Service::Service(SC_HANDLE handle)
    :
    m_Handle(handle)
{
}

void Service::waitForStatusChange(DWORD statusToBeChanged)
{
    auto startTickCount = GetTickCount64();
    auto oldCheckPoint = m_Status.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.
    while (m_Status.dwCurrentState == statusToBeChanged)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 

        auto dwWaitTime = m_Status.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        std::wcout << L"Status " << m_Status.dwCurrentState;
        std::wcout << L"Hint " << m_Status.dwWaitHint << L"\n";
        std::wcout << L"Wait for " << dwWaitTime << L"\n";

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending. 

        m_Status = queryStatus();

        if (m_Status.dwCheckPoint > oldCheckPoint)
        {
            // Continue to wait and check.

            startTickCount = GetTickCount64();
            oldCheckPoint = m_Status.dwCheckPoint;
        }
        else
        {
            if (GetTickCount64() - startTickCount > m_Status.dwWaitHint)
            {
                throw Win32Exception(L"Service::waitForStatusChange", 
                    L"timeout waiting for service to change status!");
            }
        }
    }
}

SERVICE_STATUS_PROCESS Service::queryStatus()
{
    expect(m_Handle != NULL);

    SERVICE_STATUS_PROCESS status;
    DWORD dwBytesNeeded;
    if (!QueryServiceStatusEx(
        m_Handle,                                           // handle to service 
        SC_STATUS_PROCESS_INFO,                             // information level
        reinterpret_cast<LPBYTE>(&status),                  // address of structure
        sizeof(SERVICE_STATUS_PROCESS),                     // size of structure
        &dwBytesNeeded))                                    // size needed if buffer is too small
    {
        throw Win32Exception(L"QueryServiceStatusEx");
    }

    return status;
}

Service::~Service()
{
    if (m_Handle != NULL)
        CloseServiceHandle(m_Handle);
}

SC_HANDLE Service::getHandle() const
{
    return m_Handle;
}

void Service::start(DWORD argc, LPCWSTR* argsv)
{
    if (isUninstalled())
        return;

    m_Status = queryStatus();

    if (m_Status.dwCurrentState != SERVICE_STOPPED && m_Status.dwCurrentState != SERVICE_STOP_PENDING)
    {
        return;
    }

    waitForStatusChange(SERVICE_STOP_PENDING);

    if (!StartService(
        m_Handle,       // handle to service 
        argc,           // number of arguments 
        argsv           // arguments array
    ))
    {
        throw Win32Exception(L"StartService");
    }

    m_Status = queryStatus();

    waitForStatusChange(SERVICE_START_PENDING);

    if (m_Status.dwCurrentState != SERVICE_RUNNING)
    {
        throw Win32Exception(L"Service::start", L"service status changed but it is not running");
    }
}

bool Service::isUninstalled()
{
    return m_Handle == NULL;
}

void Service::uninstall()
{
    if (isUninstalled())
        return;

    if (!DeleteService(m_Handle))
        throw Win32Exception(L"DeleteService");

    CloseServiceHandle(m_Handle);
    m_Handle = NULL;
}

Service Service::install(const ServiceControlManager& scManager, const std::wstring& serviceName,
    const std::wstring& serviceDisplayName, const std::wstring& pathToServiceBinary)
{
    SC_HANDLE handle = CreateService(
        scManager.getHandle(),              // SCM database 
        serviceName.c_str(),                // name of service 
        serviceDisplayName.c_str(),         // service name to display 
        SERVICE_ALL_ACCESS,                 // desired access 
        SERVICE_WIN32_OWN_PROCESS,          // service type 
        SERVICE_DEMAND_START,               // start type 
        SERVICE_ERROR_NORMAL,               // error control type 
        pathToServiceBinary.c_str(),        // path to service's binary 
        NULL,                               // no load ordering group 
        NULL,                               // no tag identifier 
        NULL,                               // no dependencies 
        NULL,                               // LocalSystem account 
        NULL                                // no password 
    );

    if (handle == NULL)
        throw Win32Exception(L"CreateService");

    return Service(handle);
}
