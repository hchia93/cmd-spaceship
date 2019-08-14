# CmdSpaceship
(WIP) CmdSpaceship was a course I taken in Parallel Processing course with threads in Linux, now fully rewrite using C++ thread library and winsock2. Originally only support nthreads (Input thread and Logic thread), now input are communicating via winsock2 layer.

This project should be in a working state locally (Please do expecting crashes)

How To Run
---
1. Open server first.
2. Open client with 'localhost' as argument. (Ideally it should swapped out for other ip, but I haven reach that part yet.)
3. AD to move, W to shoot.

Limitation
---
1. It has been only run via localhost, as due to lack of machines. 
2. Crappy architecture, due to lack of event dispatcher, and result in dependencies. (I was unable to find more on std::function and std::bind with templated varadic arguments)
3. Hit determination are completely local side, and depending the machine processing speed. Bullet might been seen collide with each othe in one cmd and not in another one. 
4. Unoptimized for loops, as they deteriorate when the pool increases. (No proper hit detection designs)
5. No post-game reset game loop. 
6. Some background freezes possibly caused by threads or network surges, as initial movement aren't smooth at all. 

Discovery/Mystery
---
1. "kbhit()" and "getch()" is somehow cannot be run on std::thread. Initially it freezed almost completely, take more than 2 seconds to render one page of ternimal characters. Moving that to main thread stop the data stalls. 

2. std::mutex doesn't work if it is interact with something not in std::thread. Multiple crashes and fixed couple of them

Why I made this
---
1. To familiar with const char* manipulation, usually seen in Network codes or Game codes.
2. Learn threading for the first time. 
3. Learn std::chrono for steady timers update to simulate game ticks.
4. Learn some Network codes.

Reference
---
The winsock2 part I take references from :	

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code

https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code


The rest is just class encapsulation.
