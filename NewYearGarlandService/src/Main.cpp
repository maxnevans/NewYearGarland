#include "pch.h"

#include "GarlandService.h"
#include "GarlandApp.h"
#include "Logger.h"

std::wstring getPathToCurrentExecutable()
{
    wchar_t path[MAX_PATH];
    if (!GetModuleFileName(NULL, path, MAX_PATH))
        throw Win32Exception(L"GetModuleFileName");

    return path;
}

Event stopEvent = {};
bool shouldPauseConsole = false;

BOOL WINAPI consoleHandler(DWORD signal) {

    if (signal == CTRL_C_EVENT)
    {
        stopEvent.emmit();
        std::wcout << L"Stopping service..." << L"\n";
    }

    return TRUE;
}

void fakeReportingStopFunction(DWORD milliseconds)
{
    std::wcout << L"Service reported that it will stop in " << milliseconds << L"\n";
}

void installService()
{
    std::wcout << L"Installing service...\n";
    ServiceControlManager scm;
    std::wcout << L"SCM successfully opened.\n";
    try
    {
        Service::install(scm, GarlandService::SERVICE_NAME,
            GarlandService::SERVICE_DISPLAY_NAME, getPathToCurrentExecutable());
    }
    catch (const Win32Exception& ex)
    {
        if (ex.getCode() == ERROR_SERVICE_EXISTS)
        {
            std::wcout << L"Service already installed.\n";
            return;
        }
        throw;
    }
    std::wcout << L"Service successfully installed!\n";
}

void deleteService()
{
    std::wcout << L"Deleting service...\n";
    ServiceControlManager scm;
    std::wcout << L"SCM successfully opened.\n";
    Service service(scm, GarlandService::SERVICE_NAME);
    std::wcout << L"Service found.\n";
    service.uninstall();
    std::wcout << L"Service successfully deleted!\n";
}

void startService(int argc, wchar_t* argv[])
{
    std::wcout << L"Starting service...\n";
    ServiceControlManager scm;
    std::wcout << L"SCM successfully opened.\n";
    Service service(scm, GarlandService::SERVICE_NAME);
    std::wcout << L"Service found.\n";
    service.start(argc, const_cast<const wchar_t**>(argv));
    std::wcout << L"Service successfully started!\n";
}

void startInConsoleMode()
{
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE))
        throw Win32Exception(L"SetConsoleCtrlHandler");

    std::wcout << L"Starting service from console...\n";

    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE)
        throw Win32Exception(L"GetStdHandle");

    Logger logger(hStd, true);
    logger.start();
    std::wcout << L"Logger successfully initialized.\n";

    GarlandApp app(logger);
    std::wcout << L"Application successfully initialized.\n";

    std::wcout << L"Starting app.main.\n";
    app.main(stopEvent, fakeReportingStopFunction);
    std::wcout << L"app.main stopped.\n";

    logger.stop();
    std::wcout << L"logger stopped.\n";
}


int wmain(int argc, wchar_t* argv[])
{
    try
    {
        if (lstrcmpi(argv[1], L"install") == 0)
        {
            installService();
            return 0;
        }

        if (lstrcmpi(argv[1], L"delete") == 0)
        {
            deleteService();
            return 0;
        }

        if (lstrcmpi(argv[1], L"start") == 0)
        {
            startService(argc, argv);
            return 0;
        }

        if (lstrcmpi(argv[1], L"--mode") == 0 && lstrcmpi(argv[2], L"console") == 0)
        {
            startInConsoleMode();
            return 0;
        }

        auto logger = Logger(GarlandService::getLogsPath(), true);
        logger.start();

        if (!GarlandService::start(logger))
        {
            shouldPauseConsole = true;

            // If started from console install and start service
            if (!IsUserAnAdmin())
            {
                std::wcout << L"Please run this app as administrator!\n";
            } 
            else
            {
                installService();
                startService(argc, argv);
                std::wcout << L"You can now close console safely.\n";
            }
        }

        logger.stop();
    }
    catch (const Win32Exception& ex)
    {
        std::wcout << ex.what() << L"\n";
    }
    catch (...)
    {
        std::wcout << L"Failed to execute operation! Unknown exception happend!\n";
    }

    if (shouldPauseConsole)
        system("pause");

    return 0;
}