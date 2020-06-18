#pragma once
#include "pch.h"

class SystemObject
{
public:
    virtual HANDLE getHandle() const;
    virtual void setHandle(HANDLE handle);

protected:
    HANDLE m_Handle = NULL;
};