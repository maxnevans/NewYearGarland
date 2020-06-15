#pragma once

#include "ServiceControlManager.h"

class Service
{
public:
    Service(const ServiceControlManager& scManager, const std::wstring& serviceName);
    virtual ~Service();
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    SC_HANDLE getHandle() const;
    void start();
    bool isUninstalled();
    void uninstall();
    static Service install(const ServiceControlManager& scManager, const std::wstring& serviceName,
        const std::wstring& serviceDisplayName, const std::wstring& pathToServiceBinary);

protected:
    Service(SC_HANDLE handle);
    void waitForStatusChange(DWORD statusToBeChanged);
    SERVICE_STATUS_PROCESS queryStatus();

private:
    SC_HANDLE m_Handle = NULL;
    SERVICE_STATUS_PROCESS m_Status = {};
};