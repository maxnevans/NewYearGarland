#include "pch.h"

#include "ServiceControlManager.h"
#include "Win32Exception.h"

ServiceControlManager::ServiceControlManager(const wchar_t* machineName)
{
    m_Handle = OpenSCManager(machineName, NULL, SC_MANAGER_ALL_ACCESS);

    if (m_Handle == NULL)
        throw Win32Exception(L"OpenSCManager", L"failed to open service control manager handler");
}

ServiceControlManager::~ServiceControlManager()
{
    if (m_Handle != NULL)
        CloseServiceHandle(m_Handle);
}

SC_HANDLE ServiceControlManager::getHandle() const
{
    return m_Handle;
}