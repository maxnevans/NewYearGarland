#include "pch.h"
#include "SecurityAttributes.h"
#include "SecurityDescriptor.h"

SecurityAttributes::SecurityAttributes()
    :
    m_Sa({ sizeof(SECURITY_ATTRIBUTES), NULL, FALSE })
{
    SecurityDescriptor sd = {};
    m_Sa.lpSecurityDescriptor = &sd.getModifiable();
}

const SECURITY_ATTRIBUTES& SecurityAttributes::get() const
{
    return m_Sa;
}

SECURITY_ATTRIBUTES& SecurityAttributes::getModifiable()
{
    return m_Sa;
}
