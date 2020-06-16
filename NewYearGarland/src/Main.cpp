#include "pch.h"

#include "UnclassifiedException.h"

#define WIDTH 1280
#define HEIGHT 720

void draw(Gdiplus::Graphics& gfx)
{
    Gdiplus::Color bulbColor(96, 191, 183);
    gfx.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(180, 180, 180)), Gdiplus::Rect(0, 0, WIDTH, HEIGHT));

    gfx.FillEllipse(&Gdiplus::SolidBrush(bulbColor), Gdiplus::Rect(50, 50, WIDTH - 50, HEIGHT - 50));
}

LRESULT wmPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    HDC memDc = CreateCompatibleDC(hdc);
    HBITMAP hBm = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    SelectObject(memDc, hBm);

    Gdiplus::Graphics gfx(memDc);

    draw(gfx);

    BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memDc, 0, 0, SRCCOPY);

    DeleteObject(hBm);
    DeleteObject(memDc);

    return 0;
}

void wmCreate()
{
    try
    {
        auto pipe = NamedPipe::connect(L"NewYearGarlandService");

        ClientMessage msg;
        msg.type = ClientMessageType::CONNECT;
        msg.pid = GetProcessId(GetModuleHandle(NULL));

        pipe.write(msg);

        ServerMessage answer = pipe.read<ServerMessage>();

        std::wstringstream ss;

        ss << L"Color: {";
        ss << answer.connect.color.r << L", ";
        ss << answer.connect.color.g << L", ";
        ss << answer.connect.color.b << L"}";

        MessageBox(NULL, ss.str().c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
    }
    catch (Win32Exception& ex)
    {
        MessageBox(NULL, ex.what().c_str(), L"Win32Exception Error", MB_OK |MB_ICONWARNING);
    }
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            wmCreate();
            return 0;
        case WM_PAINT:
            return wmPaint(hWnd, uMsg, wParam, lParam);
        case WM_ERASEBKGND:
            return TRUE;
        case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPWSTR lpCmdLine,  
    _In_ int nShowCmd)
{
    /* Arguments parsing */
    std::vector<std::wstring> args;
    std::wstring_view rawArgs(lpCmdLine);

    auto prevToken = rawArgs.begin();
    auto token = std::find(prevToken, rawArgs.end(), L' ');
    args.emplace_back(prevToken, token);

    while (token != rawArgs.end())
    {
        prevToken = token + 1;
        token = std::find(prevToken, rawArgs.end(), L' ');
        args.emplace_back(prevToken, token);
    }

    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr);

    try
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = wndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"MainWindow";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        RegisterClassEx(&wc);

        HWND hWnd = CreateWindow(L"MainWindow", L"New Year Garland", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            400, 400, WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);

        if (hWnd == NULL)
        {
            throw UnclassifiedException(L"Failed to create new window!");
        }

        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            DispatchMessage(&msg);
        }
    }
    catch (UnclassifiedException& exception)
    {
        MessageBox(NULL, exception.what().c_str(), L"Critical Error", MB_OK | MB_ICONERROR);
    }
    catch(...)
    {
        MessageBox(NULL, L"Unknown exception happend!", L"Critical Error", MB_OK | MB_ICONERROR);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);

    return 0;
}