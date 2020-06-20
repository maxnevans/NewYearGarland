#pragma once

#include "pch.h"
#include "SystemObject.h"

class Event
    :
    public SystemObject
{
public:
    Event(bool isManuallyReset = true, bool isSignaled = false, const wchar_t* name = nullptr);
    virtual ~Event();
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;
    Event(Event&& other) noexcept;
    Event& operator=(Event&& other) noexcept;
    void reset();
    bool check();
    bool wait(DWORD milliseconds = INFINITE);
    void emmit();
};