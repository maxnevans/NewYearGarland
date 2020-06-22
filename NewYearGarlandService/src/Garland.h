#pragma once
#include "pch.h"
#include "Logger.h"

class Garland
{
public:
    using byte = unsigned char;

    struct Color
    {
        byte r;
        byte g;
        byte b;
    };
    using OnLightOnFunc = std::function<void()>;
    using OnLightOffFunc = std::function<void()>;
    using OnColorFunc = std::function<void(const Color&)>;

    using LightbulbHandler = unsigned int;

private:
    struct Lightbulb
    {
        Lightbulb(Event& stopEvent, OnLightOnFunc onLightOn, OnLightOffFunc onLightOff, OnColorFunc onColor)
            :
            onLightOn(onLightOn),
            onLightOff(onLightOff),
            onColor(onColor),
            ex({}),
            stopEvent(stopEvent)
        {}
        OnLightOnFunc onLightOn = nullptr;
        OnLightOffFunc onLightOff = nullptr;
        OnColorFunc onColor = nullptr;
        std::optional<Exception> ex = std::nullopt;
        bool shouldStop = false;
        Event& stopEvent;
    };

public:
    Garland(Logger& logger);
    Garland(const Garland&) = delete;
    Garland& operator=(const Garland&) = delete;
    virtual ~Garland();
    void start(Event& stopEvent);
    void stop();

    /**
     * Used by lightbulbs to register it's callback functions.
     * 
     * \param stopEvent[in] event which should signalize to garland logic when to stop
     *      lighbulb
     * \param onColor[in] called on color set by thread that started Garland
     * \param onLightOn[in] called on light on by thread that started Garland
     * \param onLightOff[in] called on light off by thread that started Garland
     * \return handler for other operations on registered lightbulb
     */
    LightbulbHandler registerLightbulb(Event& stopEvent, OnColorFunc onColor,
        OnLightOnFunc onLightOn, OnLightOffFunc onLightOff);

    /**
     * Used by lightbulbs to delegate to stop thread and guarantee that thread 
     *      will not use it's internall variables
     * 
     * \param handler[in] handler to already registered lighbulb
     * \throw Exception If exception happend inside `onLightOn`, `onLightOff` or `onColor` callbacks
     *      rethrows it outside.
     */
    void delegate(LightbulbHandler handler);

private:
    void p_Unregister(LightbulbHandler handler);

    /**
     * Checks whether lightbulb should stop. Called inside monitor in `Lighbulb` thread context.
     *
     * \return true if lightbulb should stop now.
     */
    bool p_CheckShouldStop(const std::shared_ptr<Lightbulb>& lightbulb) const noexcept;
    Color p_GetColor() const noexcept;

    /**
     * Executes single lightbulb logic.Modifies `Lightbulb`.
     * 
     * \param lb[in] lightbulb to perform logic on
     * \return if exception in lighbulb user registered logic happend
     */
    bool p_PerformLightbulbLogic(Event& stopEvent, std::shared_ptr<Lightbulb>& lb) const noexcept;

    /**
     * Checks whether all lightbulbs unregistered and stopped. Called inside monitor 
     * in `Garland` thread context.
     *
     * \return true if all lightbulbs are stopped
     */
    bool p_CheckAllStopped() const noexcept;

private:
    static constexpr const byte MIN_COLOR_BRIGHTNESS = 20;

    Logger& m_Logger;
    std::map<LightbulbHandler, std::shared_ptr<Lightbulb>> m_Lightbulbs = {};
    Monitor m_Monitor = {};
    Monitor::ConditionalVariableType m_LightningControlCv = Monitor::INVALID_CONDITIONAL_VARIABLE;
    Monitor::ConditionalVariableType m_StoppingControlCv = Monitor::INVALID_CONDITIONAL_VARIABLE;
    Event m_ConnectionEvent = {false, false};
    bool m_ShouldStopAll = false;
};