#include "pch.h"
#include "SystemObject.h"

HANDLE SystemObject::getHandle() const
{
    return m_Handle;
}

void SystemObject::setHandle(HANDLE handle)
{
    // You should never use set handle
    expect(false);
    m_Handle = handle;
}