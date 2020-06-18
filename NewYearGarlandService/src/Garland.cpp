#include "pch.h"
#include "Garland.h"

Garland::Color Garland::getColor(DWORD pid) const
{
    std::random_device rd;
    // MIN_COLOR_BRIGHTNESS will guarantee that color will be at lease red of MIN_COLOR_BRIGHTNESS
    std::uniform_int_distribution<DWORD> dist(MIN_COLOR_BRIGHTNESS, std::numeric_limits<int>::max());
    std::mt19937 mt(rd());

    const auto rand = dist(mt);

    Color color = {};
    color.r = static_cast<byte>(rand);
    color.g = static_cast<byte>(rand >> 8);
    color.b = static_cast<byte>(rand >> 16);

    return color;
}

void Garland::start(DWORD id)
{
    m_LightbulbsDidLightMutex.lock();
    if (std::find(m_LightbulbsDidLight.begin(), m_LightbulbsDidLight.end(), id) != m_LightbulbsDidLight.end())
    {
        m_LightbulbsDidLightMutex.release();
        m_CycleControlEvent.wait();
    }

    InterlockedIncrement(&m_CountWaiting);

    m_LightControlEvent.wait();

    InterlockedDecrement(&m_CountWaiting);

    if (m_LightbulbsDidLight.empty())
    {
        m_CycleControlEvent.reset();
    }

    {
        MutexGuard m(m_LightbulbsDidLightMutex);
        m_LightbulbsDidLight.push_back(id);
    }
}

void Garland::wait()
{
    Sleep(WAIT_TIME);
}

void Garland::stop()
{
    if (m_CountWaiting == 0)
    {
        {
            MutexGuard m(m_LightbulbsDidLightMutex);
            m_LightbulbsDidLight.clear();
        }

        m_CycleControlEvent.emmit();
    }

    m_LightControlEvent.emmit();
}