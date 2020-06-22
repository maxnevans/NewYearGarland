#include "pch.h"
#include "Waiter.h"
#include "Win32Exception.h"

Waiter::Waiter(bool waitAll, DWORD timeout)
    :
    m_ShouldWaitAll(waitAll),
    m_Timeout(timeout)
{
}

bool Waiter::wait(const std::vector<HANDLE>& handles, DWORD timeout) const
{
    auto waitMs = timeout != INFINITE ? timeout : m_Timeout;

    auto ret = WaitForMultipleObjects(handles.size(), handles.data(),
        m_ShouldWaitAll, waitMs);

    switch (ret)
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_FAILED:
        throw Win32Exception(L"WaitForMultipleObjects");
    }

    if (ret >= WAIT_OBJECT_0 && ret < WAIT_ABANDONED_0 + handles.size())
        return true;

    if (ret >= WAIT_ABANDONED_0 && ret < WAIT_ABANDONED_0 + handles.size())
        throw Win32Exception(L"WaitForMultipleObjects", L"mutex has been recieved in "
            "abandoned state [mutex index: " + std::to_wstring(handles.size()) + L"]");

    return false;
}

bool Waiter::wait(const WaitableSystemObject& waitableObject, DWORD timeout) const
{
    auto waitMs = timeout != INFINITE ? timeout : m_Timeout;

    switch (WaitForSingleObject(waitableObject.getHandle(), waitMs))
    {
        case WAIT_TIMEOUT:
            return false;
        case WAIT_FAILED:
            throw Win32Exception(L"WaitForMultipleObjects");
        case WAIT_OBJECT_0:
            return true;
        case WAIT_ABANDONED_0:
            throw Win32Exception(L"WaitForMultipleObjects", L"mutex has been recieved in abandoned state");
    }

    throw Win32Exception(L"WaitForSingleObject", L"unknown ret value");
}
