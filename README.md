# CmdSpaceship
CmdSpaceship is a simple shooter game in command prompt settings. 
It was firstly written with `<pthread.h>` in Linux, and now fully rewritten with modern C++ as well as Winsock2 on Windows.

The code is evolved from a course material from Parallel Processing course in [Multimedia University](https://www.mmu.edu.my/) in 2015.

![Watch the preview](cmd-spaceship-3.gif)

## Building Executables
- Build for `ServerBuild`
- Build for `ClientBuild`
- You can locate both build under ..\src\CmdSpaceship\x64

## Running the Application
- Run the server executable, then run the client executable.

## Gameplay Instruction
- `A` and `D` to move horizontally, `W` to shoot.
- Press `R` to continue after one of the spaceship is defeated.

## Reference
- [winsock2(server)](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code)
- [winsock2(client)](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code)
