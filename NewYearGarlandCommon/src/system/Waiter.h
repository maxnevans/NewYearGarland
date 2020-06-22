#pragma once
#include "pch.h"

#include "WaitableSystemObject.h"

class Waiter
{
public:
    Waiter(bool waitAll = true, DWORD timeout = INFINITE);
    template<typename TWaitableSystemObject>
    bool wait(const std::vector<TWaitableSystemObject>& waitableObjects, DWORD timeout = INFINITE) const
    {
        expect(waitableObjects.size() != 0 && waitableObjects.size() <= MAXIMUM_WAIT_OBJECTS);

        if (waitableObjects.size() == 1)
            return wait(*waitableObjects[0]);

        std::vector<HANDLE> handles;
        std::transform(waitableObjects.begin(), waitableObjects.end(), std::back_inserter(handles),
            [](const TWaitableSystemObject& wo) {return wo->getHandle(); });

        return wait(handles, timeout);
    }
    bool wait(const std::vector<HANDLE>& waitableObjects, DWORD timeout = INFINITE) const;
    bool wait(const WaitableSystemObject& waitableObject, DWORD timeout = INFINITE) const;

private:
    bool m_ShouldWaitAll = true;
    DWORD m_Timeout = INFINITE;
};