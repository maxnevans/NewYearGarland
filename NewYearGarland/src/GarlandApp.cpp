#include "pch.h"
#include "GarlandApp.h"
#include "GarlandWindowMessage.h"

GarlandApp::GarlandApp()
    :
    m_WorkerArgs(std::make_shared<Worker>()),
    m_WorkerThread(workerThreadProc, &m_WorkerArgs)
{
}

void GarlandApp::start(PostMessageBinded postMessage)
{
    m_WorkerArgs->postMessage = postMessage;
    m_WorkerThread.onException([](const Exception& ex) {
        MessageBox(NULL, ex.what().c_str(), L"GarlandApp::start exception catched", MB_OK);
    });

    m_WorkerThread.start();
}

bool GarlandApp::stop(DWORD waitMilliseconds)
{
    CancelSynchronousIo(m_WorkerThread.getHandle());
    return m_WorkerThread.stop(waitMilliseconds);
}

bool GarlandApp::didStop() const
{
    return m_WorkerThread.didStop();
}

void GarlandApp::draw(Gdiplus::Graphics& gfx, const AdditionalDrawData& data)
{
    // Copping data to local variables
    Gdiplus::Color bulbColor = {};
    bool isPowered = false;

    {
        CriticalSectionGuard m(m_WorkerArgs->accessCs);
        isPowered = m_WorkerArgs->isPowered;
        bulbColor = { m_WorkerArgs->color.r, m_WorkerArgs->color.g, m_WorkerArgs->color.b };
    }

    // Drawing
    
    // Draw extra area to be sure that there is no dark stripes
    gfx.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(180, 180, 180)), 
        Gdiplus::Rect(0, 0, data.dims.width, data.dims.height));

    // Calculate padding on based on window size: 5% of average of height and width
    const int lightbulbPadding = static_cast<int>(std::floor((data.dims.width * 1.f + data.dims.height * 1.f) / 2.0 * 0.05));
    auto lightbulbDrawRect = Gdiplus::Rect(lightbulbPadding, lightbulbPadding, 
        data.dims.width - 2 * lightbulbPadding, data.dims.height - 2 * lightbulbPadding);

    if (isPowered)
        gfx.FillEllipse(&Gdiplus::SolidBrush(bulbColor), lightbulbDrawRect);

    gfx.DrawEllipse(&Gdiplus::Pen(Gdiplus::Color::Black, 2), lightbulbDrawRect);
}

void GarlandApp::workerThreadProc(Event& stopEvent, std::shared_ptr<Worker>* pWorker)
{
    // Copy shared pointer to increment `use_count` and prevent from deletion
    auto worker = *pWorker;

    try
    {
        auto pipe = NamedPipe::connect(L"NewYearGarlandService", GENERIC_READ);

        while (!stopEvent.check())
        {
            ServerMessage connectMsg = pipe.read<ServerMessage>();

            {
                auto& color = connectMsg.color;
                CriticalSectionGuard cs(worker->accessCs);
                worker->color = { color.r, color.g, color.b };
            }

            ServerMessage lightOnMsg = pipe.read<ServerMessage>();

            {
                CriticalSectionGuard cs(worker->accessCs);
                worker->isPowered = true;
            }

            if (worker->postMessage(GarlandWindowMessage::WM_GARLAND, 
                GarlandWindowMessage::WPARAM_STATE_CHAGED, NULL) == FALSE)
                throw Win32Exception(L"PostMessage");

            ServerMessage lightOffMsg = pipe.read<ServerMessage>();

            {
                CriticalSectionGuard cs(worker->accessCs);
                worker->isPowered = false;
            }

            if (worker->postMessage(GarlandWindowMessage::WM_GARLAND,
                GarlandWindowMessage::WPARAM_STATE_CHAGED, NULL) == FALSE)
                throw Win32Exception(L"PostMessage");
        }

        worker->postMessage(GarlandWindowMessage::WM_GARLAND,
            GarlandWindowMessage::WPARAM_THREAD_STOPPED, NULL);
        return;

    }
    catch (const Win32Exception& ex)
    {
        if (ex.getCode() == ERROR_BROKEN_PIPE)
        {
            MessageBox(NULL, L"Service has disconnected.", L"New Year Garland: Application stopped", MB_OK | MB_ICONINFORMATION);
            worker->postMessage(GarlandWindowMessage::WM_GARLAND,
                GarlandWindowMessage::WPARAM_THREAD_STOPPED, NULL);
            return;
        }

        if (ex.getCode() == ERROR_FILE_NOT_FOUND)
        {
            MessageBox(NULL, L"Service did not found.", L"New Year Garland: Application stopped", MB_OK | MB_ICONINFORMATION);
            worker->postMessage(GarlandWindowMessage::WM_GARLAND,
                GarlandWindowMessage::WPARAM_THREAD_STOPPED, NULL);
            return;
        }

        
        if (ex.getCode() == ERROR_OPERATION_ABORTED)
        {
            worker->postMessage(GarlandWindowMessage::WM_GARLAND,
                GarlandWindowMessage::WPARAM_THREAD_STOPPED, NULL);
            return;
        }

        throw;
    }
}
