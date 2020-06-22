# New Year Garland - C++ Win32 client-service application

## Build instructions: 

- Download and install Windows SDK 10.0.18362.0 from Visual Studio Installer.
- Launch NewYearGarland.sln with Visual Studio 2019.
- Choose "Release" and x86 architecture.
- Press `Ctrl + Shift + B` to build solution.


## Launch instructions:

- Launch `cmd.exe` with admin rights.
- Naviate to `${SolutionDir}bin` folder.
- Run `NewYearGarlandService.exe install`.
- Press `Ctrl + R` and type `services.msc` to open SCM.
- Press `N` to quickly navigate to `New Year Garland` service.
- Right click on it and choose `start`.
- Open `${SolutionDir}bin` folder in file explorer. 
- Run `NewYearGarland.exe` as many instances as you want.
- Relax.