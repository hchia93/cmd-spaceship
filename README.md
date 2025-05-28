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
`A` and `D` to move horizontally, W to shoot.

## Roadmap
### 2025 - Done
- Make proper solution for a old-school school project. 
- Encapsulate logic and responsibility to each of their class.
- Update coding standard to cpp17. 
- Updated ReadMe.md and added fancy preview.
- Fix code, and refactoring with vibe coding. (Still a bit manual coding before the codebase reaching vibe-able.)

## Reference
The winsock2 part I take references from :	

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

## Authors
[@hchia93](https://www.github.com/hchia93)
