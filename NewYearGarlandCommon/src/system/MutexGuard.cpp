#include "pch.h"
#include "MutexGuard.h"

MutexGuard::MutexGuard(Mutex& mutex)
    :
    m_Mutex(mutex)
{
    mutex.lock(INFINITE);
}

MutexGuard::~MutexGuard()
{
    m_Mutex.release();
}
