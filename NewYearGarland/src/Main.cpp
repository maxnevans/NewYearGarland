#include "pch.h"
#include "GarlandApp.h"
#include "GarlandWindow.h"

std::vector<std::wstring> parseArguments(LPWSTR lpCmdLine)
{
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

    return args;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPWSTR lpCmdLine,  
    _In_ int nShowCmd)
{
    auto args = parseArguments(lpCmdLine);

    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr);

    try
    {
        const int startWidth = 300, startHeight = 100;
        // Padding from screen corners to prevent appearing under taskbar
        const int screenPadding = 50;

        // Random start position to show different windows in different start positions
        std::random_device rd;
        std::uniform_int_distribution<int> startXDist(screenPadding, GetSystemMetrics(SM_CXSCREEN) - startWidth - screenPadding);
        std::uniform_int_distribution<int> startYDist(screenPadding, GetSystemMetrics(SM_CYSCREEN) - startHeight - screenPadding);
        std::mt19937 gen(rd());

        GarlandWindow window(startXDist(gen), startYDist(gen), startWidth, startHeight);

        window.start();

        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            DispatchMessage(&msg);
        }

        Gdiplus::GdiplusShutdown(gdiplusToken);
        return static_cast<int>(msg.wParam);
    }
    catch (const Win32Exception& ex)
    {
        MessageBox(NULL, ex.what().c_str(), L"Critical Error", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return -1;
    }
    catch(...)
    {
        MessageBox(NULL, L"Unknown exception happend!", L"Critical Error", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return -1;
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}