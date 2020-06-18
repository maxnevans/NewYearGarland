#pragma once
#include "SystemObject.h"

namespace std
{
    template<> struct less<SystemObject>
    {
        bool operator()(const SystemObject& lhs, const SystemObject& rhs) const
        {
            return lhs.getHandle() < rhs.getHandle();
        }
    };
}