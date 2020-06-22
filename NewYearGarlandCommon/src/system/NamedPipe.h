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
    static NamedPipe connect(const std::wstring& name, DWORD mode = GENERIC_READ | GENERIC_WRITE);

    template<typename T>
    void write(T data)
    {
        expect(m_Handle != NULL);

        DWORD numberOfBytesWritten;
        auto status = WriteFile(m_Handle, &data, sizeof(T), &numberOfBytesWritten, NULL);

        if (status == FALSE || numberOfBytesWritten != sizeof(T))
            throw Win32Exception(L"WriteFile");
    }

    template<typename T>
    T read(int waitMilliseconds = 1000)
    {
        expect(m_Handle != NULL);

        T msg;
        DWORD numberOfBytesRead;
        auto status = ReadFile(m_Handle, &msg, sizeof(T), &numberOfBytesRead, NULL);
        if (status == FALSE || numberOfBytesRead != sizeof(msg))
            throw Win32Exception(L"ReadFile");

        return msg;
    }

protected:
    NamedPipe(HANDLE handle);

private:
    static constexpr const DWORD INPUT_BUFFER_SIZE      = 4096;
    static constexpr const DWORD OUTPUT_BUFFER_SIZE     = 4096;
};