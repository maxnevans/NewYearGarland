#pragma once
#include "pch.h"

class Exception 
{
public:
    Exception(std::wstring msg = L"")
    {
        m_Message = msg;
    }

    virtual const std::wstring& what() const noexcept
    {
        return m_Message;
    }

private:
    std::wstring m_Message = L"";
};