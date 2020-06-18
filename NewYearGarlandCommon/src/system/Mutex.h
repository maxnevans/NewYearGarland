#pragma once
#include "SystemObject.h"

class Mutex
    :
    public SystemObject
{
public:
    Mutex();
    virtual ~Mutex();
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    bool lock(DWORD waitMilliseconds = INFINITE);
    void release();
    bool isLocked() const;

private:
    HANDLE m_Handle = NULL;
    bool m_IsLocked = false;
};