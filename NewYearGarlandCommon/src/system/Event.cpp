#include "pch.h"
#include "Event.h"
#include "Win32Exception.h"

Event::Event(bool isManuallyReset, bool isSignaled, const wchar_t* name)
{
    m_Handle = CreateEvent(
        NULL,               // sec attrs: default
        isManuallyReset,    // is manually reset
        isSignaled,         // is signaled
        name                // name
    );

    if (m_Handle == NULL)
        throw Win32Exception(L"CreateEvent");

}

Event::~Event()
{
    if (m_Handle != NULL)
        CloseHandle(m_Handle);
}

Event::Event(Event&& other) noexcept
{
    m_Handle = other.m_Handle;
    other.m_Handle = NULL;
}

Event& Event::operator=(Event&& other) noexcept
{
    if (m_Handle != NULL)
        CloseHandle(m_Handle);

    m_Handle = other.m_Handle;
    other.m_Handle = NULL;

    return *this;
}

void Event::reset()
{
    expect(m_Handle != NULL);

    if (!ResetEvent(m_Handle))
        throw Win32Exception(L"ResetEvent");
}

bool Event::check()
{
    expect(m_Handle != NULL);

    switch (WaitForSingleObject(m_Handle, 0))
    {
        /* Stop signaled */
        case WAIT_OBJECT_0:
            return true;

        case WAIT_FAILED:
            throw Win32Exception(L"Event::check", L"WaitForSingleObject on event failed");
    }

    return false;
}

bool Event::wait(DWORD milliseconds)
{
    expect(m_Handle != NULL);

    switch (WaitForSingleObject(m_Handle, milliseconds))
    {
        /* Stop signaled */
    case WAIT_OBJECT_0:
        return true;

    case WAIT_FAILED:
        throw Win32Exception(L"Event::check", L"WaitForSingleObject on event failed");
    }

    return false;
}

void Event::emmit()
{
    expect(m_Handle != NULL);

    if (!SetEvent(m_Handle))
        throw Win32Exception(L"SetEvent");
}
