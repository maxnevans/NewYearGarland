#pragma once
#include "pch.h"

class Exception 
{
public:
    Exception() {}

    Exception(const std::wstring& msg)
    {
        m_Message = msg;
    }

    virtual std::wstring what() const noexcept
    {
        return m_Message;
    }

protected:
    const std::wstring& getMessage() const
    {
        return m_Message;
    }

private:
    std::wstring m_Message = L"";
};