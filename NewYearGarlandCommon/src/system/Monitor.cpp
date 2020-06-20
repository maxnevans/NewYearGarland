#include "pch.h"
#include "Monitor.h"
#include "Win32Exception.h"

Monitor::Monitor()
{
    InitializeCriticalSectionAndSpinCount(&m_CriticalSection, SPIN_COUNT);
}

Monitor::~Monitor()
{
    DeleteCriticalSection(&m_CriticalSection);
}

void Monitor::enter()
{
    EnterCriticalSection(&m_CriticalSection);
}

void Monitor::leave()
{
    LeaveCriticalSection(&m_CriticalSection);
}

Monitor::ConditionalVariableType Monitor::createVariable()
{
    CONDITION_VARIABLE cs;
    InitializeConditionVariable(&cs);
    m_ConditionalVariables.emplace_back(std::move(cs));
    return m_ConditionalVariables.size() - 1;
}

bool Monitor::sleep(ConditionalVariableType variable, DWORD milliseconds)
{
    expect(variable >= 0 && variable < m_ConditionalVariables.size());

    auto& cs = m_ConditionalVariables[variable];
    if (SleepConditionVariableCS(&cs, &m_CriticalSection, milliseconds) == FALSE)
    {
        auto err = GetLastError();

        if (err == ERROR_TIMEOUT)
            return false;
        else
            throw Win32Exception(L"SleepConditionVariableCS", err);
    }

    return true;
}

void Monitor::wake(ConditionalVariableType variable, bool wakeAll)
{
    expect(variable >= 0 && variable < m_ConditionalVariables.size());

    auto& cs = m_ConditionalVariables[variable];
    if (wakeAll)
        WakeAllConditionVariable(&cs);
    else
        WakeConditionVariable(&cs);
}
