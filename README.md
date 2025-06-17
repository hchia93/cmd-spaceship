# CmdSpaceship
CmdSpaceship is a simple shooter game in command prompt settings. 
It was firstly written with `<pthread.h>` in Linux, and now fully rewritten with modern C++ as well as Winsock2 on Windows.

The code is evolved from a course material from Parallel Processing course in [Multimedia University](https://www.mmu.edu.my/) in 2015.

![Watch the preview](cmd-spaceship-3.gif)

## Instruction
### Running the Application
1. Build both executable.
2. Run server application first, then client.
> Alternatively, run `Server` in VisualStudio, detach the process, then runs `Client`.

### Gameplay
- `A` and `D` to move horizontally, `W` to shoot.
- Press `R` to continue after one of the spaceship is defeated.

## Reference
- [winsock2(server)](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code)
- [winsock2(client)](https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code)
