#pragma once

#include "CriticalSection.h"

class CriticalSectionGuard
{
public:
    CriticalSectionGuard(CriticalSection& cs);
    virtual ~CriticalSectionGuard();
    CriticalSectionGuard(const CriticalSectionGuard&) = delete;
    CriticalSectionGuard& operator=(const CriticalSectionGuard&) = delete;
private:
    CriticalSection& m_Cs;
};