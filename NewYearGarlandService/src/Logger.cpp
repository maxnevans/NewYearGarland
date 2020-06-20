#include "pch.h"
#include "Logger.h"

Event Logger::m_StopEvent = Event();

Logger::Logger(HANDLE handle, bool shouldThrow, bool enableFlusher)
    :
    m_LogFileHandle(handle),
    m_ShouldThrow(shouldThrow),
    m_ShouldCloseFile(false),
    m_EnableFlusher(enableFlusher)
{
}

Logger::Logger(const std::wstring& pathToLogs, bool shouldThrow, bool enableFlusher)
    :
    m_ShouldThrow(shouldThrow),
    m_LogDir(pathToLogs),
    m_EnableFlusher(enableFlusher)
{
}

Logger::~Logger()
{
    expect(m_FlusherThread != NULL);

    if (m_FlusherThread != NULL)
    {
        TerminateThread(m_FlusherThread, NULL);
        CloseHandle(m_FlusherThread);
    }

    if (m_ShouldCloseFile && m_LogFileHandle != NULL)
        CloseHandle(m_LogFileHandle);
}

void Logger::start()
{
    try
    {
        if (m_LogFileHandle == NULL)
            createLogFile();

        if (m_EnableFlusher)
            startFlusher();
            
    }
    catch (...)
    {
        m_StartFailed = true;
        if (m_ShouldThrow)
            throw;
        else
            m_IsFailed = true;
    }
}

void Logger::stop(int maxWait)
{
    if (m_EnableFlusher)
    {
        expect(m_FlusherThread != NULL);

        if (m_FlusherThread == NULL)
            return;

        try
        {
            m_StopEvent.emmit();
            switch (WaitForSingleObject(m_FlusherThread, maxWait))
            {
            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                throw Win32Exception(L"Logger::stop", L"WaitForSingleObject on event failed");
            }
        }
        catch (...)
        {
            if (m_ShouldThrow)
                throw;
            else
                m_IsFailed = true;
        }
    }
}

bool Logger::isFailed() const
{
    return m_IsFailed;
}

void Logger::error(const std::wstring& message)
{
    if (!m_StartFailed)
        writeMessage(formatMessage(MARK_ERROR, message));
}

void Logger::warn(const std::wstring& message)
{
    if (!m_StartFailed)
        writeMessage(formatMessage(MARK_WARN, message));
}

void Logger::debug(const std::wstring& message)
{
    if (!m_StartFailed)
        writeMessage(formatMessage(MARK_DEBUG, message));
}

void Logger::info(const std::wstring& message)
{
    if (!m_StartFailed)
        writeMessage(formatMessage(MARK_INFO, message));
}

std::wstring Logger::formatMessage(const std::wstring& mark, const std::wstring& message) const
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wstring timestamp = L"[" + formatTime(st) + L"]";

    return mark + L" " + timestamp + L": " + message + L"\r\n";
}

std::wstring Logger::formatTime(const SYSTEMTIME& st) const
{
    std::wstringstream ss;
    ss << padNumber(st.wHour) << ':' << padNumber(st.wMinute) << ':' << padNumber(st.wSecond) << '.' << padNumber(st.wMilliseconds, 3);
    return ss.str();
}

std::wstring Logger::formatDate(const SYSTEMTIME& st) const
{
    const wchar_t* const DAYS[] = { L"SUN", L"MON", L"TUE", L"WED", L"THU", L"FRI", L"SAT" };
    std::wstringstream ss;
    ss << st.wYear << '-' << padNumber(st.wMonth) << '-' << padNumber(st.wDay) << '-' << DAYS[st.wDayOfWeek];
    return ss.str();
}

void Logger::writeMessage(const std::wstring& message)
{
    expect(m_LogFileHandle != NULL);

    try
    {        
        auto requiredSize = WideCharToMultiByte(CP_UTF8, NULL, message.c_str(), message.size(), NULL, 0, NULL, NULL);
        if (requiredSize == NULL)
            throw Win32Exception(L"WideCharToMultiByte");

        std::string convertedStr(requiredSize, 0);
        if (WideCharToMultiByte(CP_UTF8, NULL, message.c_str(), message.size(), convertedStr.data(), convertedStr.size(), NULL, NULL) == NULL)
            throw Win32Exception(L"WideCharToMultiByte");

        CriticalSectionGuard cs(m_Cs);
        DWORD written;
        if (!WriteFile(m_LogFileHandle, convertedStr.c_str(), convertedStr.size(), &written, NULL) || written != convertedStr.size())
            throw Win32Exception(L"WriteFile");
    }
    catch (...)
    {
        if (m_ShouldThrow)
            throw;
        else
            m_IsFailed = true;
    }
}

bool Logger::directoryExists(const std::wstring& path) const
{
    expect(!path.empty());

    DWORD dwAttrib = GetFileAttributes(path.c_str());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void Logger::makeDirectory(const std::wstring& path) const
{
    expect(!path.empty());

    if (!CreateDirectory(path.c_str(), NULL))
        throw Win32Exception(L"CreateDirectory");
}

std::wstring Logger::padNumber(int number, int pad) const
{
    expect(number < 1000 && number >= 0);

    if (number < 10)
        return std::wstring(pad - 1, L'0') + std::to_wstring(number);

    if (number < 100)
        return std::wstring(pad - 2, L'0') + std::to_wstring(number);

    return std::to_wstring(number);
}

std::wstring Logger::createLogFileName() const
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    return formatDate(st) + L".txt";
}

std::wstring Logger::joinPath(const std::wstring& dir, const std::wstring& filename) const
{
    wchar_t path[MAX_PATH];
    if (PathCombine(path, dir.c_str(), filename.c_str()) == NULL)
        throw Win32Exception(L"PathCombine");

    return path;
}

DWORD WINAPI Logger::flusherThreadProc(LPVOID params)
{
    auto logger = reinterpret_cast<Logger*>(params);

    while (true)
    {
        {
            CriticalSectionGuard cs(logger->m_Cs);

            if (!FlushFileBuffers(logger->m_LogFileHandle))
                throw Win32Exception(L"FlushFileBuffers");
        }

        if (m_StopEvent.wait(FLUSH_INTERVAL))
            break;
    }

    return 0;
}

void Logger::startFlusher()
{
    m_FlusherThread = CreateThread(NULL, NULL, flusherThreadProc, this, NULL, NULL);

    if (m_FlusherThread == NULL)
        throw Win32Exception(L"CreateThread");
}

void Logger::createLogFile()
{
    if (!directoryExists(m_LogDir))
        makeDirectory(m_LogDir);

    auto logFilePath = joinPath(m_LogDir, createLogFileName());

    m_LogFileHandle = CreateFile(
        logFilePath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_LogFileHandle == INVALID_HANDLE_VALUE)
    {
        auto errCode = GetLastError();

        if (errCode != ERROR_FILE_EXISTS)
            throw Win32Exception(L"CreateFile", L"failed to create non existing file", errCode);

        // Trying to open existing file

        m_LogFileHandle = CreateFile(
            logFilePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (m_LogFileHandle == INVALID_HANDLE_VALUE)
            throw Win32Exception(L"CreateFile", L"failed to open existing file");
    }

    if (SetFilePointer(m_LogFileHandle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        throw Win32Exception(L"SetFilePointer");
}
