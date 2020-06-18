#pragma once
#include "pch.h"
class Garland
{
public:
    using byte = unsigned char;

    struct Color
    {
        byte r;
        byte g;
        byte b;
    };
public:
    Color getColor(DWORD pid) const;
    void start(DWORD id);
    void wait();
    void stop();

private:
    static constexpr const DWORD WAIT_TIME                      = 1000;
    static constexpr const byte MIN_COLOR_BRIGHTNESS            = 25;

    Event m_CycleControlEvent                                   = {true, false};
    Event m_LightControlEvent                                   = {false, true};
    std::vector<DWORD> m_LightbulbsDidLight                     = {};
    Mutex m_LightbulbsDidLightMutex                             = {};
    volatile DWORD m_CountWaiting                               = 0;
};