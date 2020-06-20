#pragma once
#include "pch.h"
#include "Win32Exception.h"
#include "SystemObject.h"

class NamedPipe
    :
    public SystemObject
{
public:
    NamedPipe(const std::wstring& name, DWORD mode = PIPE_ACCESS_DUPLEX);
    virtual ~NamedPipe();
    NamedPipe(const NamedPipe&) = delete;
    NamedPipe& operator=(const NamedPipe&) = delete;
    void listen();
    static NamedPipe connect(const std::wstring& name);

    template<typename T>
    void write(T data)
    {
        expect(m_Handle != NULL);
        expect(m_ErrorCode == ERROR_SUCCESS);

        DWORD numberOfBytesWritten;
        auto status = WriteFile(m_Handle, &data, sizeof(T), &numberOfBytesWritten, NULL);

        if (status == FALSE || numberOfBytesWritten != sizeof(T))
            throw Win32Exception(L"WriteFile");
    }

    template<typename T>
    T read(int waitMilliseconds = 1000)
    {
        expect(m_Handle != NULL);
        expect(m_ErrorCode == ERROR_SUCCESS);

        T msg;
        DWORD numberOfBytesRead;
        auto status = ReadFile(m_Handle, &msg, sizeof(T), &numberOfBytesRead, NULL);
        if (status == FALSE || numberOfBytesRead != sizeof(msg))
            throw Win32Exception(L"ReadFile");

        return msg;
    }

    template<typename T>
    T peek()
    {
        expect(m_Handle != NULL);
        expect(m_ErrorCode == ERROR_SUCCESS);

        T msg;
        DWORD numberOfBytesRead;
        auto status = PeekNamedPipe(m_Handle, &msg, sizeof(T), &numberOfBytesRead, NULL, NULL);
        if (status == FALSE || numberOfBytesRead != sizeof(msg))
            throw Win32Exception(L"PeekNamedPipe");

        return msg;
    }

    bool operator!()
    {
        if (PeekNamedPipe(m_Handle, NULL, NULL, NULL, NULL, NULL) == FALSE)
        {
            m_ErrorCode = GetLastError();
            return true;
        }

        return false;
    }

    bool doesClientDisconnected()
    {
        if (!*this && m_ErrorCode == ERROR_BROKEN_PIPE)
        {
            m_ErrorCode = ERROR_SUCCESS;
            return true;
        }
        return false;
    }

    DWORD getErrorCode() const
    {
        return m_ErrorCode;
    }

protected:
    NamedPipe(HANDLE handle);

private:
    static constexpr const DWORD INPUT_BUFFER_SIZE      = 4096;
    static constexpr const DWORD OUTPUT_BUFFER_SIZE     = 4096;

    DWORD m_ErrorCode = ERROR_SUCCESS;
};