#pragma once

#include "pch.h"

class Barrier
{
public:
    Barrier(long totalThreads);
    virtual ~Barrier();
    Barrier(const Barrier&) = delete;
    Barrier& operator=(const Barrier&) = delete;
    void enter();

private:
    static constexpr const DWORD SPIN_COUNT = 4096;
    SYNCHRONIZATION_BARRIER m_Barrier = {};
};