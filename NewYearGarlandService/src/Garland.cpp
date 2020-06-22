#include "pch.h"
#include "Garland.h"

Garland::Garland(Logger& logger)
    :
    m_Logger(logger)
{
    m_LightningControlCv = m_Monitor.createVariable();
    m_StoppingControlCv = m_Monitor.createVariable();
}

Garland::~Garland()
{
    // Garland should never be destroyed 
    // if there is still working lightbulbs
    expect(m_ShouldStopAll == true);
    expect(m_Lightbulbs.size() == 0);
}

void Garland::start(Event& stopEvent)
{
    m_Logger.debug(L"Garland::start: begin.");
    bool shouldStop = false;
    while (!shouldStop)
    {
        m_Logger.info(L"Garland::start: waiting for any lightbulb to be connected.");

        m_Monitor.enter();
        size_t countLightbulbs = m_Lightbulbs.size();
        m_Monitor.leave();

        // Garland has no lighbulbs, waiting for any to connect
        while (!shouldStop && countLightbulbs == 0 && !m_ConnectionEvent.wait(200))
        {
            if (stopEvent.check())
                shouldStop = true;
        }

        m_Logger.info(L"Garland::start: got lighbulb or should stop or got a connection.");

        if (!shouldStop)
        {
            m_Logger.info(L"Garland::start: starting lightbulb loop.");
            m_Logger.info(L"Garland::start: this lightbulb loop contains " + std::to_wstring(m_Lightbulbs.size()));

            m_Monitor.enter();
            try
            {
                bool shouldNotifyAll = false;
                for (auto i = m_Lightbulbs.begin(); i != m_Lightbulbs.end(); i++)
                {
                    auto& lb = i->second;
                    m_Monitor.leave();

                    m_Logger.info(L"Garland::start: handling lighbulb.");

                    m_Logger.info(L"Garland::start: check if we should stop.");

                    // check if we should stop main loop
                    if (stopEvent.check())
                    {
                        m_Logger.info(L"Garland::start: should stop from lightbulb loop.");
                        m_Monitor.enter();
                        shouldStop = true;
                        shouldNotifyAll = false;
                        break;
                    }

                    m_Logger.debug(L"Garland::start: check if current lightbulb is stopping right now.");

                    // check if lightbulb is stopping
                    if (lb->shouldStop)
                    {
                        m_Logger.info(L"Garland::start: current lightbulb is stopping right now.");
                        m_Monitor.enter();
                        continue;
                    }

                    m_Logger.debug(L"Garland::start: check if current lightbulb should be stopped.");

                    // check if lightbulb should be stopped
                    if (lb->stopEvent.check())
                    {
                        m_Logger.info(L"Garland::start: current lightbulb should be stopped.");
                        lb->shouldStop = true;
                        shouldNotifyAll = true;
                        m_Monitor.enter();
                        continue;
                    }

                    m_Logger.debug(L"Garland::start: performing lightbulb logic.");

                    // if fails than lighbulb should be notified to check state: wake all lightbulbs and let them check
                    if (!p_PerformLightbulbLogic(stopEvent, lb))
                    {
                        shouldNotifyAll = true;
                        m_Logger.warn(L"Garland::start: lightbulb logic needs to notify all lightbulbs.");
                    }

                    m_Monitor.enter();
                }
                m_Monitor.leave();

                // true if any lighbulb thrown exception or should be stopped
                // on this garland loop
                if (shouldNotifyAll)
                {
                    m_Monitor.wake(m_LightningControlCv, true);
                    m_Logger.info(L"Garland::start: notifing all lightbulbs to check their states.");
                }
            }
            catch (const Exception& e)
            {
                m_Logger.error(L"Garland::start: " + e.what());
                throw;
            }
            catch (...)
            {
                m_Logger.error(L"Garland::start: unknown error");
                throw;
            }
        }
    }
    m_Logger.debug(L"Garland::start: end.");
}

bool Garland::stop(DWORD waitMilliseconds)
{
    auto savePoint = GetTickCount64();
    long long ticksLeft = waitMilliseconds;

    m_Monitor.enter();
    m_ShouldStopAll = true;
    m_Monitor.wake(m_LightningControlCv, true);

    // Wait until all lightbulbs will stop
    while (!p_CheckAllStopped() && ticksLeft > 0)
    {
        m_Monitor.sleep(m_StoppingControlCv, static_cast<DWORD>(ticksLeft));
        ticksLeft = waitMilliseconds - static_cast<DWORD>(GetTickCount64() - savePoint);
    }

    if (ticksLeft <= 0 && !p_CheckAllStopped())
    {
        m_Monitor.leave();
        return false;
    }

    m_Monitor.leave();

    return true;
}

Garland::LightbulbHandler Garland::registerLightbulb(Event& stopEvent, OnColorFunc onColor,
    OnLightOnFunc onLightOn, OnLightOffFunc onLightOff)
{
    bool garlandIsStopping = false;
    {
        m_Monitor.enter();
        garlandIsStopping = m_ShouldStopAll;
        m_Monitor.leave();
    }

    if (garlandIsStopping)
        throw Exception(L"Failed to register lightbulb in garland! Garland is stopping now.");

    auto lightbulb = std::make_shared<Lightbulb>(stopEvent, onLightOn, onLightOff, onColor);
    auto handler = reinterpret_cast<LightbulbHandler>(lightbulb.get());

    {
        m_Monitor.enter();
        m_Lightbulbs.emplace(handler, std::move(lightbulb));
        m_Monitor.leave();
    }

    return handler;
}

void Garland::delegate(LightbulbHandler handler)
{
    m_Logger.debug(L"Garland::delegate starts.");

    std::map<LightbulbHandler, std::shared_ptr<Lightbulb>>::iterator pair;
    {
        m_Monitor.enter();
        pair = m_Lightbulbs.find(handler);
        m_Monitor.leave();
    }
    if (pair != m_Lightbulbs.end())
    {
        auto lb = pair->second;

        m_Logger.debug(L"Garland::delegate: lightbulb found.");

        m_ConnectionEvent.emmit();
        m_Logger.debug(L"Garland::delegate: connection event emmited.");

        m_Monitor.enter();
        while (!p_CheckShouldStop(lb))
        {
            m_Logger.debug(L"Garland::delegate: lighbulb should not stop. Sleeping.");
            m_Monitor.sleep(m_LightningControlCv);
        }
        m_Monitor.leave();

        m_Logger.debug(L"Garland::delegate: lightbulb is stopping.");
        
        if (lb->ex != std::nullopt)
        {
            m_Logger.debug(L"Garland::delegate: lightbulb has an exception.");
            p_Unregister(pair->first);
            throw *lb->ex;
        }

        p_Unregister(pair->first);
        m_Logger.debug(L"Garland::delegate: lightbulb stopped.");
        m_Logger.debug(L"Garland::delegate ends.");
    }
}

void Garland::p_Unregister(LightbulbHandler handler)
{
    m_Monitor.enter();
    m_Lightbulbs.erase(handler);
    m_Monitor.wake(m_StoppingControlCv, true);
    m_Monitor.leave();
}

bool Garland::p_CheckShouldStop(const std::shared_ptr<Lightbulb>& lightbulb) const noexcept
{
    return lightbulb->shouldStop || m_ShouldStopAll;
}

Garland::Color Garland::p_GetColor() const noexcept
{
    std::random_device rd;
    std::uniform_int_distribution<unsigned int> dist(MIN_COLOR_BRIGHTNESS, std::numeric_limits<unsigned int>::max());
    std::mt19937 mt(rd());

    auto rand = dist(mt);
    
    Color c = {};
    c.r = static_cast<byte>(rand);
    c.g = static_cast<byte>(rand >> 8);
    c.b = static_cast<byte>(rand >> 16);

    return c;
}

bool Garland::p_PerformLightbulbLogic(Event& stopEvent, std::shared_ptr<Lightbulb>& lb) const noexcept
{
    try
    {
        if (!lb->onColor(p_GetColor()))
        {
            lb->shouldStop = true;
            return false;
        }

        if (!lb->onLightOn())
        {
            lb->shouldStop = true;
            return false;
        }

        Waiter w(true, 1000);
        if (w.wait(stopEvent))
            return true;

        if (!lb->onLightOff())
        {
            lb->shouldStop = true;
            return false;
        }

        return true;
    }
    catch (const Exception& e)
    {
        lb->ex = e;
        lb->shouldStop = true;
        return false;
    }

    return false;
}

bool Garland::p_CheckAllStopped() const noexcept
{
    return m_Lightbulbs.size() == 0;
}
