#pragma once

#include "Mutex.h"

class MutexGuard
{
public:
    MutexGuard(Mutex& mutex);
    virtual ~MutexGuard();
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
private:
    Mutex& m_Mutex;
};