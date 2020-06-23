#include "pch.h"
#include "GarlandWindow.h"
#include "GarlandWindowMessage.h"

GarlandWindow::GarlandWindowRegisterer GarlandWindow::m_WndRegisterer = {GetModuleHandle(NULL), GarlandWindow::WINDOW_CLASS};

GarlandWindow::GarlandWindowRegisterer::GarlandWindowRegisterer(HINSTANCE hInst, const wchar_t* className)
    :
    m_HInstance(hInst),
    m_ClassName(className)
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = GarlandWindow::p_WndInstallProc;
    wc.hInstance = hInst;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if (RegisterClassEx(&wc) == FALSE)
    {
        m_ErrorCode = GetLastError();
        m_DidSuccess = false;
    }
    else
        m_DidSuccess = true;
}

GarlandWindow::GarlandWindowRegisterer::~GarlandWindowRegisterer()
{
    UnregisterClass(m_ClassName.c_str(), m_HInstance);
}

DWORD GarlandWindow::GarlandWindowRegisterer::getErrorCode() const
{
    return m_ErrorCode;
}

bool GarlandWindow::GarlandWindowRegisterer::didSuccess() const
{
    return m_DidSuccess;
}

HINSTANCE GarlandWindow::GarlandWindowRegisterer::getHInstance() const
{
    return m_HInstance;
}

GarlandWindow::GarlandWindow(int x, int y, int width, int height)
    :
    m_AnchorPoint({ x, y }),
    m_Dimensions({ width, height }),
    m_App()
{
}


void GarlandWindow::start()
{
    // Adjust window rect to desired client rect

    RECT windowClientRect = {
        m_AnchorPoint.x,
        m_AnchorPoint.y, 
        m_AnchorPoint.x + m_Dimensions.width,
        m_AnchorPoint.y + m_Dimensions.height,
    };

    if (AdjustWindowRect(&windowClientRect, WINDOW_STYLE, NULL) == NULL)
        throw Win32Exception(L"AdjustWindowRect");

    HWND hWnd = CreateWindow(WINDOW_CLASS, WINDOW_NAME, WINDOW_STYLE,
        windowClientRect.left, windowClientRect.top, 
        windowClientRect.right - windowClientRect.left, 
        windowClientRect.bottom - windowClientRect.top,
        NULL, NULL, m_WndRegisterer.getHInstance(), this);

    if (hWnd == NULL)
        throw Win32Exception(L"CreateWindow");

    // Do not set m_HWnd member here. It is being set inside 
    // p_WndInstallProc more earlier that it could be set here.
}

void GarlandWindow::close()
{
    expect(m_HWnd != NULL);

    if (DestroyWindow(m_HWnd) == FALSE)
        throw Win32Exception(L"DestroyWindow");
}

LRESULT GarlandWindow::p_WndInstallProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE)
        throw Exception(L"Failed to setup p_WndProxyProc! WM_NCCREATE has not been called!");

    if (uMsg != WM_NCCREATE)
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
    GarlandWindow* const window = reinterpret_cast<GarlandWindow* const>(createStruct->lpCreateParams);

    SetLastError(ERROR_SUCCESS);

    window->m_HWnd = hWnd;

    if (SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window)) == FALSE && GetLastError() != ERROR_SUCCESS)
        throw Exception(L"Failed to set GWLP_USERDATA when handling p_WndInstallProc");

    if (SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GarlandWindow::p_WndProxyProc)) == FALSE && GetLastError() != ERROR_SUCCESS)
        throw Exception(L"Failed to set GWLP_WNDPROC when handling p_WndInstallProc with p_WndProxyProc");

    return window->p_WndProc(uMsg, wParam, lParam);
}

LRESULT GarlandWindow::p_WndProxyProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    GarlandWindow* const wnd = reinterpret_cast<GarlandWindow* const>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    return wnd->p_WndProc(uMsg, wParam, lParam);
}

LRESULT GarlandWindow::p_WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:                             return p_WmCreate(uMsg, wParam, lParam);
        case WM_SIZING:                             return p_WmSizing(uMsg, wParam, lParam);
        case WM_PAINT:                              return p_WmPaint(uMsg, wParam, lParam);
        case WM_ERASEBKGND:                         return p_WmEraseBackground(uMsg, wParam, lParam);
        case WM_CLOSE:                              return p_WmClose(uMsg, wParam, lParam);
        case WM_DESTROY:                            return p_WmDestroy(uMsg, wParam, lParam);
        case GarlandWindowMessage::WM_GARLAND:      return p_WmGarland(uMsg, wParam, lParam);
    }

    return DefWindowProc(m_HWnd, uMsg, wParam, lParam);
}

LRESULT GarlandWindow::p_WmCreate(UINT, WPARAM, LPARAM)
{
    expect(m_HWnd != NULL);

    m_App.start([this](UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return PostMessage(m_HWnd, uMsg, wParam, lParam);
    });

    return 0;
}

LRESULT GarlandWindow::p_WmPaint(UINT, WPARAM, LPARAM)
{
    expect(m_HWnd != NULL);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_HWnd, &ps);
    HDC memDc = CreateCompatibleDC(hdc);
    HBITMAP hBm = CreateCompatibleBitmap(hdc, m_Dimensions.width, m_Dimensions.height);
    SelectObject(memDc, hBm);

    Gdiplus::Graphics gfx(memDc);

    m_App.draw(gfx, {m_Dimensions});

    BitBlt(hdc, 0, 0, m_Dimensions.width, m_Dimensions.height, memDc, 0, 0, SRCCOPY);

    DeleteObject(hBm);
    DeleteObject(memDc);

    return 0;
}

LRESULT GarlandWindow::p_WmSizing(UINT, WPARAM, LPARAM lParam)
{
    expect(m_HWnd != NULL);

    // Changing saved m_Dimensions and m_AnchorPoint

    RECT clientRect;
    if (GetClientRect(m_HWnd, &clientRect) == FALSE)
        throw Win32Exception(L"GetClientRect");

    m_Dimensions.width = clientRect.right;
    m_Dimensions.height = clientRect.bottom;

    RECT* windowRect = reinterpret_cast<RECT*>(lParam);

    m_AnchorPoint = {
        windowRect->left,
        windowRect->top,
    };

    // Redraw

    InvalidateRect(m_HWnd, NULL, FALSE);

    return TRUE;
}

LRESULT GarlandWindow::p_WmClose(UINT, WPARAM, LPARAM)
{
    expect(m_HWnd != NULL);

    if (!m_IsClosingWindow && !m_App.didStop())
    {
        // Ignore all close messages if we did not stop app yet

        if (!m_IsClosingWindow)
        {
            m_App.stop(0);
            m_IsClosingWindow = true;

            // Give user notification that application is closing right now
        }
        
        return 0;
    }

    DestroyWindow(m_HWnd);
    return 0;
}

LRESULT GarlandWindow::p_WmDestroy(UINT, WPARAM, LPARAM)
{
    PostQuitMessage(0);
    m_HWnd = NULL;
    return 0;
}

LRESULT GarlandWindow::p_WmEraseBackground(UINT, WPARAM, LPARAM)
{
    return TRUE;
}

LRESULT GarlandWindow::p_WmGarland(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    expect(m_HWnd != NULL);

    switch (wParam)
    {
        case GarlandWindowMessage::WPARAM_STATE_CHAGED:
        {
            InvalidateRect(m_HWnd, NULL, FALSE);
            break;
        }
        case GarlandWindowMessage::WPARAM_THREAD_STOPPED:
        {
            m_IsClosingWindow = true;
            DestroyWindow(m_HWnd);
            break;
        }
    }

    return 0;
}
