#pragma once

#include "pch.h"

class Monitor
{
public:
    Monitor();
    virtual ~Monitor();
    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
    void enter();
    void leave();
    int createVariable();
    bool sleep(int variable, DWORD milliseconds = INFINITE);
    void wake(int variable, bool wakeAll = false);

private:
    static constexpr const DWORD SPIN_COUNT = 4096;
    CRITICAL_SECTION m_CriticalSection = {};
    std::vector<CONDITION_VARIABLE> m_ConditionalVariables = {};
};