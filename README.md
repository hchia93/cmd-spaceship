# CmdSpaceship
(WIP) CmdSpaceship was a course I taken in Parallel Processing course with threads in Linux, now fully rewrite using C++ thread library and winsock2. Originally only support nthreads (Input thread and Logic thread), now input are communicating via winsock2 layer.

This project should be in a working state locally (Please do expecting crashes)

How To Run
---
1. Open server first.
2. Open client later.
3. AD to move, W to shoot.
4. Shoot bullet and dodge enemy's bullet.

Limitation
---
1. It has been only run via localhost, as due to lack of machines. 
2. Less desirable class architecture. Would be better with event dispatcher. (I was unable to find more on std::function and std::bind with templated varadic arguments)
3. Local Authoritative hit detection. Simulated bullet might not sync with the actual bullet position. 
4. Unoptimized for loops, as they deteriorate when the pool increases. However, profiler didn't pick this up.
5. No post-game reset game loop. 
6. Threads seems to lagging sometimes. (Profiler picks up on TaskSend())

Discovery/Mystery
---
1. "kbhit()" and "getch()" is somehow cannot be run on std::thread. Initially it freezed almost completely, take more than 2 seconds to render one page of ternimal characters. Moving that to main thread stop the data stalls. 

2. std::mutex doesn't work if it is interact with something not in std::thread. Multiple crashes and fixed couple of them

3. Chrono doesn't seems very realible. Works with unoptimization off, doesn't work in optimization on. Replaced with Sleep(20) instead and position syncing is more realible locally. (100% not on remote machine)

Why I made this
---
1. To familiar with const char* manipulation, usually seen in Network codes or Game codes.
2. Threading, network code learning, as well as C++11,14,17 syntax.
3. Learn to profiling and adjust codes.

Reference
---
The winsock2 part I take references from :	

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

