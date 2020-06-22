#pragma once

#include "pch.h"

class Monitor
{
public:
    using ConditionalVariableType = unsigned int;
public:
    Monitor();
    virtual ~Monitor();
    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
    void enter();
    void leave();
    ConditionalVariableType createVariable();
    bool sleep(ConditionalVariableType variable, DWORD milliseconds = INFINITE);
    void wake(ConditionalVariableType variable, bool wakeAll = false);

public:
    static constexpr const ConditionalVariableType INVALID_CONDITIONAL_VARIABLE = -1;

private:
    static constexpr const DWORD SPIN_COUNT = 4096;
    CRITICAL_SECTION m_CriticalSection = {};
    std::vector<CONDITION_VARIABLE> m_ConditionalVariables = {};
};