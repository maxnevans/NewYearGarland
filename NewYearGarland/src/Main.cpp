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


enum class ClientMessageType
{
    CONNECT,
    DISCONNECT,
};

struct ClientMessage
{
    ClientMessageType type;
    DWORD pid;
};

enum class ServerMessageType
{
    CONNECT,
    DISCONNECT,
    LIGHT
};

struct ServerMessageConnect
{
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } color;
};

struct ServerMessageLight
{
    bool isPowered;
};

struct ServerMessageDisconnect
{
    // TODO: fill message with server reason for disconnect
};

struct ServerMessage {
    ServerMessageType type;
    union {
        ServerMessageConnect connect;
        ServerMessageLight light;
        ServerMessageDisconnect disconnect;
    };
};

void wmCreate()
{
    HANDLE hPipe = CreateFile(
        L"\\\\.\\pipe\\NewYearGarlandService",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Failed to create pipe connection!", L"Warning!", MB_OK | MB_ICONWARNING);
        return;
    }

    ClientMessage msg;
    msg.type = ClientMessageType::CONNECT;
    msg.pid = GetProcessId(GetModuleHandle(NULL));

    DWORD numberOfBytesWritten;
    if (WriteFile(hPipe, &msg, sizeof(msg), &numberOfBytesWritten, NULL) == FALSE || numberOfBytesWritten != sizeof(msg))
    {
        MessageBox(NULL, L"Failed to write to pipe!", L"Warning!", MB_OK | MB_ICONWARNING);
        return;
    }

    ServerMessage answer;
    DWORD numberOfBytesRead;
    if (ReadFile(hPipe, &answer, sizeof(answer), &numberOfBytesRead, NULL) == FALSE || numberOfBytesRead != sizeof(answer))
    {
        MessageBox(NULL, L"Failed to read from pipe!", L"Warning!", MB_OK | MB_ICONWARNING);
        return;
    }
    std::wstringstream ss;

    ss << L"Color: {";
    ss << answer.connect.color.r << L", ";
    ss << answer.connect.color.g << L", ";
    ss << answer.connect.color.b << L"}";

    MessageBox(NULL, ss.str().c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
    
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