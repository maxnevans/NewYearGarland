#pragma once
#include "../pch.h"

class ServiceControlManager
{
public:
    ServiceControlManager(const wchar_t* machineName = nullptr);
    virtual ~ServiceControlManager();
    ServiceControlManager(const ServiceControlManager&) = delete;
    ServiceControlManager& operator=(const ServiceControlManager&) = delete;
    SC_HANDLE getHandle() const;

private:
    SC_HANDLE m_Handle = NULL;
};
