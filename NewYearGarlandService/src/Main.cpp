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



int wmain(int argc, wchar_t* argv[])
{
    try
    {
        if (lstrcmpi(argv[1], L"install") == 0)
        {
            std::wcout << L"Installing service...\n";
            ServiceControlManager scm;
            std::wcout << L"SCM successfully opened.\n";
            Service::install(scm, GarlandService::SERVICE_NAME,
                GarlandService::SERVICE_DISPLAY_NAME, getPathToCurrentExecutable());
            std::wcout << L"Service successfully installed!\n";
            return 0;
        }

        if (lstrcmpi(argv[1], L"delete") == 0)
        {
            std::wcout << L"Deleting service...\n";
            ServiceControlManager scm;
            std::wcout << L"SCM successfully opened.\n";
            Service service(scm, GarlandService::SERVICE_NAME);
            std::wcout << L"Service found.\n";
            service.uninstall();
            std::wcout << L"Service successfully deleted!\n";
            return 0;
        }

        if (lstrcmpi(argv[1], L"start") == 0)
        {
            std::wcout << L"Starting service...\n";
            ServiceControlManager scm;
            std::wcout << L"SCM successfully opened.\n";
            Service service(scm, GarlandService::SERVICE_NAME);
            std::wcout << L"Service found.\n";
            service.start();
            std::wcout << L"Service successfully started!\n";
            return 0;
        }

        if (lstrcmpi(argv[1], L"--mode") == 0 && lstrcmpi(argv[2], L"console") == 0)
        {
            if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
                printf("\nERROR: Could not set control handler");
                return 1;
            }

            std::wcout << L"Starting service from console...\n";

            wchar_t szPath[MAX_PATH];
            if (!GetModuleFileName(NULL, szPath, MAX_PATH))
            {
                lstrcpy(szPath, L"C:\\logs");
            }
            PathRemoveFileSpec(szPath);
            PathAppend(szPath, L"logs");


            Logger logger(szPath, true);
            logger.start();
            std::wcout << L"Logger successfully initialized.\n";
            GarlandApp app(logger);
            std::wcout << L"Application successfully initialized.\n";

            std::wcout << L"Starting app.main.\n";
            app.main(stopEvent, fakeReportingStopFunction);
            std::wcout << L"app.main stopped.\n";

            logger.stop();
            std::wcout << L"logger stopped.\n";

            return 0;
        }

        GarlandService::start();
    }
    catch (Win32Exception& ex)
    {
        std::wcout << ex.what() << L"\n";
    }
    catch (...)
    {
        std::wcout << L"Failed to execute operation! Unknown exception happend!\n";
    }

    return 0;
}