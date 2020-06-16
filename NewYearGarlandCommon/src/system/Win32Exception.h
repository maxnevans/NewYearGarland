#pragma once
#include "../pch.h"

#include "../Exception.h"

class Win32Exception
    :
    public Exception
{
public:
    Win32Exception(const std::wstring& functionName)
        :
        Exception()
    {
        m_FunctionName = functionName;
        m_ErrorCode = GetLastError();
    }

    Win32Exception(const std::wstring& functionName, DWORD code)
        :
        Exception()
    {
        m_FunctionName = functionName;
        m_ErrorCode = code;
    }

    Win32Exception(const std::wstring& functionName, const std::wstring& message)
        :
        Exception(message)
    {
        m_FunctionName = functionName;
        m_ErrorCode = GetLastError();
    }

    Win32Exception(const std::wstring& functionName, const std::wstring& message, DWORD code)
        :
        Exception(message)
    {
        m_FunctionName = functionName;
        m_ErrorCode = code;
    }

    virtual std::wstring what() const noexcept {
        std::wstringstream ss;
        ss << L"Error in " << m_FunctionName << L" ";
        ss << L"[System Code: " << m_ErrorCode << "]";

        if (!getMessage().empty())
            ss << ": " << getMessage();

        if (!ss)
            return L"Fatal error: failed to generate exception message!";
       
        return ss.str().c_str();
    }

    DWORD getCode() const
    {
        return m_ErrorCode;
    }

protected:
    const DWORD& getErrorCode() const
    {
        return m_ErrorCode;
    }

private:
    std::wstring m_FunctionName = L"";
    DWORD m_ErrorCode = ERROR_SUCCESS;
};