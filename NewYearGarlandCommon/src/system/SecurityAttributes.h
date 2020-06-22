#pragma once
#include "pch.h"

class SecurityAttributes
{
public:
    SecurityAttributes();
    const SECURITY_ATTRIBUTES& get() const;
    SECURITY_ATTRIBUTES& getModifiable();

private:
    SECURITY_ATTRIBUTES m_Sa = {};
};