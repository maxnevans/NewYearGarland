#include "pch.h"
#include "NamedPipe.h"
#include "Win32Exception.h"

NamedPipe::NamedPipe(const std::wstring& name)
{
    std::wstring fullName = L"\\\\.\\pipe\\" + name;

    m_Handle = CreateNamedPipe(
        fullName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        INPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE, 0, NULL
    );

    if (m_Handle == INVALID_HANDLE_VALUE)
        throw Win32Exception(L"CreateNamedPipe");
}

NamedPipe::~NamedPipe()
{
    expect(m_Handle != NULL);

    if (m_Handle != NULL)
        CloseHandle(m_Handle);
}

void NamedPipe::listen()
{
    expect(m_Handle != NULL);

    if (ConnectNamedPipe(m_Handle, NULL) == 0)
        throw Win32Exception(L"ConnectNamedPipe");
}

NamedPipe NamedPipe::connect(const std::wstring& name)
{
    std::wstring fullName = L"\\\\.\\pipe\\" + name;

    const int checksNumber = 5;

    for (int i = 0; i < checksNumber; i++)
        if (WaitNamedPipe(fullName.c_str(), NMPWAIT_USE_DEFAULT_WAIT))
            break;
        else if (i == checksNumber - 1)
            throw Win32Exception(L"WaitNamedPipe", L"connect timeout, failed to connect to pipe");
    
    HANDLE hPipe = CreateFile(
        fullName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE)
        throw Win32Exception(L"CreateFile");

    return NamedPipe(hPipe);
}

NamedPipe::NamedPipe(HANDLE handle)
{
    expect(handle != NULL);
    m_Handle = handle;
}
