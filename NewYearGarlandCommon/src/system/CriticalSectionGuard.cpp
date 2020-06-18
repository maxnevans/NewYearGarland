#include "pch.h"
#include "CriticalSectionGuard.h"

CriticalSectionGuard::CriticalSectionGuard(CriticalSection& cs)
    :
    m_Cs(cs)
{
    m_Cs.enter();
}

CriticalSectionGuard::~CriticalSectionGuard()
{
    m_Cs.leave();
}
