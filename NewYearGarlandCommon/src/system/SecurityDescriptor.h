#pragma once

class SecurityDescriptor
{   
public:
    SecurityDescriptor();
    const SECURITY_DESCRIPTOR& get() const;
    SECURITY_DESCRIPTOR& getModifiable();

private:
    SECURITY_DESCRIPTOR m_Sd = {};
};