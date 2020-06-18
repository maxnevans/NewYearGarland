#include "pch.h"
#include "CriticalSection.h"

CriticalSection::CriticalSection(DWORD spinCount)
{
    InitializeCriticalSectionAndSpinCount(&m_CriticalSection, spinCount);
}

CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&m_CriticalSection);
}

void CriticalSection::enter()
{
    EnterCriticalSection(&m_CriticalSection);
}

void CriticalSection::leave()
{
    LeaveCriticalSection(&m_CriticalSection);
}
