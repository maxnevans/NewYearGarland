#include "pch.h"
#include "Barrier.h"

Barrier::Barrier(long totalThreads)
{
    InitializeSynchronizationBarrier(&m_Barrier, totalThreads, SPIN_COUNT);
}

Barrier::~Barrier()
{
    DeleteSynchronizationBarrier(&m_Barrier);
}

void Barrier::enter()
{
    EnterSynchronizationBarrier(&m_Barrier, NULL);
}
