#pragma once
#include "pch.h"

class CriticalSection
{
public:
    CriticalSection(DWORD spinCount = DEFAULT_SPIN_COUNT);
    virtual ~CriticalSection();
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
    void enter();
    void leave();

public:
    static constexpr const DWORD DEFAULT_SPIN_COUNT = 4096;
private:
    CRITICAL_SECTION m_CriticalSection = {};
};