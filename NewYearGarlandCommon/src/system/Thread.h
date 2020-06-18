#pragma once
#include "pch.h"
#include "SystemObject.h"
#include "../Exception.h"
#include "Win32Exception.h"
#include "Event.h"

template<typename T>
class Thread
    :
    public SystemObject
{
public:
    using ThreadProc = void (*)(Event& stopEvent, T arg);
    using OnExceptionCallback = std::function<void(const Exception&)>;
public:
    Thread(ThreadProc proc, T arg)
        :
        m_Proc(proc),
        m_ProcArgument(arg)
    {
        m_Handle = CreateThread(NULL, STACK_SIZE, threadProc,
            reinterpret_cast<LPVOID>(this), 
            CREATE_SUSPENDED, &m_ThreadId);

        if (m_Handle == NULL)
            throw Win32Exception(L"CreateThread");
    }
    virtual ~Thread()
    {
        expect(m_Handle != NULL);

        if (m_Handle != NULL)
            CloseHandle(m_Handle);
    }
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    void start()
    {
        expect(m_Handle != NULL);

        auto countResumes = ResumeThread(m_Handle);

        if (countResumes == -1)
            throw Win32Exception(L"ResumeThread");
    }
    bool stop(int waitMilliseconds = 1000)
    {
        CancelSynchronousIo(m_Handle);
        m_StopEvent.emmit();
        return m_StopEvent.wait(waitMilliseconds);
    }
    void terminate()
    {
        if (!TerminateThread(m_Handle, 1))
            throw Win32Exception(L"TerminateThread");
    }
    void onException(OnExceptionCallback cb)
    {
        m_ExceptionCallback = cb;
    }
    DWORD getId() const
    {
        return m_ThreadId;
    }

protected:
    static DWORD WINAPI threadProc(LPVOID arg)
    {
        auto thread = reinterpret_cast<Thread<T>*>(arg);
        try
        {
            thread->m_Proc(thread->m_StopEvent, thread->m_ProcArgument);
            return THREAD_CODE_OK;
        }
        catch (Exception& e)
        {
            thread->m_Exception = e;
            if (thread->m_ExceptionCallback)
                thread->m_ExceptionCallback(e);
        }
        return THREAD_CODE_EXCEPTION;
    }

private:
    static constexpr const SIZE_T STACK_SIZE                = 4096;
    static constexpr const DWORD THREAD_CODE_TERMINATE      = 2;
    static constexpr const DWORD THREAD_CODE_EXCEPTION      = -1;
    static constexpr const DWORD THREAD_CODE_OK             = 0;

    T m_ProcArgument                                        = {};
    ThreadProc m_Proc                                       = nullptr;
    DWORD m_ThreadId                                        = NULL;
    Event m_StopEvent                                       = Event(true, false);
    Exception m_Exception                                   = {};
    OnExceptionCallback m_ExceptionCallback                 = nullptr;
};