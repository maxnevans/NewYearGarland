#include "pch.h"
#include "Monitor.h"
#include "Win32Exception.h"

Monitor::Monitor()
{
    InitializeCriticalSectionAndSpinCount(&m_CriticalSection, SPIN_COUNT);
    InitializeConditionVariable(&m_ConditionalVariable);
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

int Monitor::createVariable()
{
    CONDITION_VARIABLE cs;
    InitializeConditionVariable(&cs);
    m_ConditionalVariables.push_back(cs);
    return m_ConditionalVariables.size() - 1;
}

bool Monitor::sleep(int variable, DWORD milliseconds)
{
    auto cs = m_ConditionalVariables.at(variable);
    if (SleepConditionVariableCS(&cs, &m_CriticalSection, milliseconds) == FALSE)
    {
        auto err = GetLastError();

        if (err == ERROR_TIMEOUT)
            return false;
        else
            throw Win32Exception(L"SleepConditionVariableCS", err);
    }
}

void Monitor::wake(int variable, bool wakeAll)
{
    auto cs = m_ConditionalVariables.at(variable);
    if (wakeAll)
        WakeAllConditionVariable(&cs);
    else
        WakeConditionVariable(&cs)
}
