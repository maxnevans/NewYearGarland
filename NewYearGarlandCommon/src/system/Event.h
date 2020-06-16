#pragma once

#include "../pch.h"

class Event
{
public:
    Event(bool isManuallyReset = true, bool isSignaled = false, const wchar_t* name = nullptr);
    virtual ~Event();
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;
    void reset();
    bool check();
    bool wait(int milliseconds = INFINITE);
    void emmit();
    
private:
    HANDLE m_Handle = NULL;
};