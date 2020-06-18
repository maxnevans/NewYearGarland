#include "pch.h"
#include "Mutex.h"
#include "Win32Exception.h"

Mutex::Mutex()
{
    m_Handle = CreateMutex(NULL, FALSE, NULL);

    if (m_Handle == INVALID_HANDLE_VALUE)
        throw Win32Exception(L"CreateMutex");
}

Mutex::~Mutex()
{
    expect(m_Handle != NULL);

    if (m_Handle != NULL)
        CloseHandle(m_Handle);
}

bool Mutex::lock(DWORD waitMilliseconds)
{
    switch (WaitForSingleObject(m_Handle, waitMilliseconds))
    {
        case WAIT_ABANDONED:
            expect(false); // Mutex was abandoned
        case WAIT_OBJECT_0:
            m_IsLocked = true;
            return true;
        case WAIT_FAILED:
            throw Win32Exception(L"WaitForSingleObject", L"failed to wait for mutex");
        case WAIT_TIMEOUT:
            return false;
    }

    throw Win32Exception(L"WaitForSingleObject", L"unknown control path");
}

void Mutex::release()
{
    if (!ReleaseMutex(m_Handle))
        throw Win32Exception(L"ReleaseMutex");
}

bool Mutex::isLocked() const
{
    return m_IsLocked;
}
