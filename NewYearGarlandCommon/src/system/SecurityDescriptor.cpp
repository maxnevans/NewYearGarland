#include "pch.h"
#include "SecurityDescriptor.h"
#include "Win32Exception.h"

SecurityDescriptor::SecurityDescriptor()
{
    SID_IDENTIFIER_AUTHORITY sidAuthEveryone = SECURITY_WORLD_SID_AUTHORITY;
    PSID everyoneSid = NULL;

    if (AllocateAndInitializeSid(&sidAuthEveryone, 1, SECURITY_WORLD_RID,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, &everyoneSid) == FALSE)
        throw Win32Exception(L"AllocateAndInitializeSid");

    // Memory for 2 access entities: everyone and admins
    EXPLICIT_ACCESS ea[2];
    ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

    // Initializing everyone rights: read only
    ea[0].grfAccessPermissions = KEY_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = reinterpret_cast<LPTSTR>(everyoneSid);

    SID_IDENTIFIER_AUTHORITY sidAuthAdmin = SECURITY_NT_AUTHORITY;
    PSID adminSid = NULL;

    if (AllocateAndInitializeSid(&sidAuthAdmin, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, NULL, NULL, NULL, NULL, NULL, NULL, &adminSid) == FALSE)
        throw Win32Exception(L"AllocateAndInitializeSid");

    // Initializing admin rights: all access
    ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName = reinterpret_cast<LPTSTR>(adminSid);

    PACL acl = nullptr;
    if (auto code = SetEntriesInAcl(2, ea, NULL, &acl); code != ERROR_SUCCESS)
        throw Win32Exception(L"SetEntriesInAcl", code);

    if (InitializeSecurityDescriptor(&m_Sd, SECURITY_DESCRIPTOR_REVISION) == FALSE)
        throw Win32Exception(L"InitializeSecurityDescriptor");

    if (SetSecurityDescriptorDacl(&m_Sd, TRUE, acl, FALSE) == FALSE)
        throw Win32Exception(L"SetSecurityDescriptorDacl");
}

const SECURITY_DESCRIPTOR& SecurityDescriptor::get() const
{
    return m_Sd;
}

SECURITY_DESCRIPTOR& SecurityDescriptor::getModifiable()
{
    return m_Sd;
}
