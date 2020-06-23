#pragma once
#include "pch.h"

// Defining user message type to recieve notification
// from application logic started on the other thread
class GarlandWindowMessage
{
public:
    static constexpr const UINT WM_GARLAND = WM_USER + 0x001;
    static constexpr const WPARAM WPARAM_STATE_CHAGED = 0x1;
    static constexpr const WPARAM WPARAM_THREAD_STOPPED = 0x2;
};