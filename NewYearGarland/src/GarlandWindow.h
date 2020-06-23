#pragma once

#include "pch.h"
#include "GarlandApp.h"
#include "Geometry.h"

class GarlandWindow
{
private:
    class GarlandWindowRegisterer
    {
    public:
        GarlandWindowRegisterer(HINSTANCE hInst, const wchar_t* className);
        ~GarlandWindowRegisterer();
        DWORD getErrorCode() const;
        bool didSuccess() const;
        HINSTANCE getHInstance() const;

    private:
        std::wstring m_ClassName = L"";
        HINSTANCE m_HInstance = NULL;
        bool m_DidSuccess = false;
        DWORD m_ErrorCode = ERROR_SUCCESS;
    };
public:
    GarlandWindow(int x, int y, int width, int height);
    GarlandWindow(const GarlandWindow&) = delete;
    GarlandWindow& operator=(const GarlandWindow&) = delete;
    void start();
    void close();

private:
    // Window procedures
    static LRESULT CALLBACK p_WndInstallProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK p_WndProxyProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT p_WndProc(UINT, WPARAM, LPARAM);

    // Window message handlers
    LRESULT p_WmCreate(UINT, WPARAM, LPARAM);
    LRESULT p_WmPaint(UINT, WPARAM, LPARAM);
    LRESULT p_WmSizing(UINT, WPARAM, LPARAM);
    LRESULT p_WmClose(UINT, WPARAM, LPARAM);
    LRESULT p_WmDestroy(UINT, WPARAM, LPARAM);
    LRESULT p_WmEraseBackground(UINT, WPARAM, LPARAM);
    LRESULT p_WmGarland(UINT, WPARAM, LPARAM);

private:
    static constexpr const wchar_t* WINDOW_NAME = L"New Year Garland Lightbulb";
    static constexpr const wchar_t* WINDOW_CLASS = L"NewYearGarlandWindow";
    static constexpr const wchar_t* MSG_SERVICE_DISCONNECTED = L"Connection with service lost.";
    static constexpr const DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    static GarlandWindowRegisterer m_WndRegisterer;

    HWND m_HWnd = NULL;
    GarlandApp m_App;

    // Client rect anchor point
    IPoint m_AnchorPoint;

    // Client rect dimentions
    IDimensions m_Dimensions;

    bool m_IsClosingWindow = false;
};