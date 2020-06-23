#pragma once

#include "pch.h"

#include "Geometry.h"

class GarlandApp
{
public:
    using PostMessageBinded = std::function<BOOL(UINT, WPARAM, LPARAM)>;

    struct AdditionalDrawData
    {
        IDimensions dims;
    };
private:
    struct Worker
    {
        using byte = unsigned char;
        struct
        {
            byte r = 0;
            byte g = 0;
            byte b = 0;
        } color = {};

        bool isPowered = false;
        CriticalSection accessCs = {};
        PostMessageBinded postMessage = nullptr;
    };
public:
    GarlandApp();
    void start(PostMessageBinded postMessage);
    bool stop(DWORD waitMilliseconds = INFINITE);
    bool didStop() const;
    void draw(Gdiplus::Graphics& gfx, const AdditionalDrawData& data);
    static void workerThreadProc(Event& stopEvent, std::shared_ptr<Worker>* pWorker);

private:
    PostMessageBinded m_PostMessageBinded;
    std::shared_ptr<Worker> m_WorkerArgs;
    Thread< std::shared_ptr<Worker>*> m_WorkerThread;
};