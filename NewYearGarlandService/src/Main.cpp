#include "pch.h"

#include "GarlandService.h"

std::wstring getPathToCurrentExecutable()
{
    wchar_t path[MAX_PATH];
    if (!GetModuleFileName(NULL, path, MAX_PATH))
        throw Win32Exception(L"GetModuleFileName");

    return path;
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