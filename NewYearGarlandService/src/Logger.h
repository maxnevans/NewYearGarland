#pragma once
#include "pch.h"

#include "system/Event.h"

class Logger
{
public:
    Logger(const std::wstring& pathToLogs, bool shouldThrow = false);
    virtual ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    void start();
    void stop(int maxWait = 3000);
    bool isFailed() const;
    virtual void error(const std::wstring& message);
    virtual void warn(const std::wstring& message);
    virtual void debug(const std::wstring& message);
    virtual void info(const std::wstring& message);

protected:
    std::wstring formatMessage(const std::wstring& mark, const std::wstring& message) const;
    std::wstring formatTime(const SYSTEMTIME& st) const;
    std::wstring formatDate(const SYSTEMTIME& st) const;
    void writeMessage(const std::wstring& message);
    bool directoryExists(const std::wstring& path) const;
    void makeDirectory(const std::wstring& path) const;
    std::wstring padNumber(int number, int pad = 2) const;
    std::wstring createLogFileName() const;
    std::wstring joinPath(const std::wstring& dir, const std::wstring& filename) const;
    static DWORD WINAPI flusherThreadProc(_In_ LPVOID params);
    void startFlusher();

public:
    static constexpr const wchar_t* const MARK_ERROR = L"ERROR";
    static constexpr const wchar_t* const MARK_WARN = L"WARN";
    static constexpr const wchar_t* const MARK_DEBUG = L"DEBUG";
    static constexpr const wchar_t* const MARK_INFO = L"INFO";

private:
    static constexpr const int FLUSH_INTERVAL = 1000;

    HANDLE m_FlusherThread = NULL;
    static Event m_StopEvent;
    HANDLE m_LogFileHandle = NULL;
    std::wstring m_LogDir = L"";
    bool m_ShouldThrow = false;
    bool m_IsFailed = false;
    bool m_StartFailed = false;
};