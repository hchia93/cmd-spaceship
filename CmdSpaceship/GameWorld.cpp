#define WIN32_LEAN_AND_MEAN
// Windows specific
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h> // kbhit, MS-DOS specific

// Standard Library
#include <iostream>
#include <string>

// Project Headers
#include "GameWorld.h"
#include "Bullet.h"
#include "Utils.h"

// system("cls") causes flicker due to io, use windows api here.
// https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
// https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#_win32_character_attributes

void SetCursorPostion(int x, int y)
{
    static const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coordinate = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(stdHandle, coordinate);
}

void SetConsoleColor(unsigned short color)
{
    static const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    SetConsoleTextAttribute(stdHandle, color);
}

void SetCursorInfo()
{
    static const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    // Disable cursor blinks in cmd.
    CONSOLE_CURSOR_INFO info;
    info.bVisible = false;
    info.dwSize = 0;
    SetConsoleCursorInfo(stdHandle, &info);
}

void SetScreenBufferSize(unsigned int X, unsigned int Y)
{
    static const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordinate;
    coordinate.X = X;
    coordinate.Y = Y;
    SetConsoleScreenBufferSize(stdHandle, coordinate);
}

//FOREGROUND_BLUE 	Text color contains blue.
//FOREGROUND_GREEN 	Text color contains green.
//FOREGROUND_RED 	Text color contains red.
//FOREGROUND_INTENSITY 	Text color is intensified.
//BACKGROUND_BLUE 	Background color contains blue.
//BACKGROUND_GREEN 	Background color contains green.
//BACKGROUND_RED 	Background color contains red.
//BACKGROUND_INTENSITY 	Background color is intensified.

////////////////////////////////////////////////////////////////////////////////////////////////////////////

GameWorld::GameWorld()
{
    CreateSpaceShips();
    InitializeNetwork();
}

GameWorld::~GameWorld()
{
}

void GameWorld::CreateSpaceShips()
{
    SpaceShipSpawnParam localSpawnParam;
    localSpawnParam.netRole = ENetRole::LOCAL;
    localSpawnParam.spawnLocation = FLocation2D(SCREEN_X_MAX / 2, SCREEN_Y_MAX - 2);
    localSpawnParam.spawnBulletFunction = std::bind(&BulletPoolService::Request, &m_BulletPoolService);

    SpaceShipSpawnParam remoteSpawnParam;
    remoteSpawnParam.netRole = ENetRole::REMOTE;
    remoteSpawnParam.spawnLocation = FLocation2D(SCREEN_X_MAX / 2, 1);
    remoteSpawnParam.spawnBulletFunction = std::bind(&BulletPoolService::Request, &m_BulletPoolService);

    m_LocalSpaceShip = std::make_unique<Spaceship>(localSpawnParam);
    m_RemoteSpaceShip = std::make_unique<Spaceship>(remoteSpawnParam);

    SetCursorInfo();
    SetScreenBufferSize(CONSOLE_SCREEN_BUFFER_X, CONSOLE_SCREEN_BUFFER_Y);
}

void GameWorld::Finalize()
{
    FinalizeNetwork();
}

void GameWorld::InitializeNetwork()
{
    m_NetworkManager.SetInputManager(&m_InputManager);
    m_NetworkManager.Initialize();
    if (m_NetworkManager.IsInitialized())
    {
        m_NetworkReceiverThread = std::thread(&NetworkManager::TaskReceive, std::ref(m_NetworkManager));
        m_NetworkSenderThread = std::thread(&NetworkManager::TaskSend, std::ref(m_NetworkManager));
    }
    else
    {
        Exit();
    }
}

void GameWorld::FinalizeNetwork()
{
    if (m_NetworkManager.IsInitialized())
    {
        m_NetworkReceiverThread.join();
        m_NetworkSenderThread.join();
    }
}

void GameWorld::Update()
{
    if (!m_LocalSpaceShip || !m_RemoteSpaceShip)
    {
        return;
    }

    if (!m_InputManager.bHasWinner)
    {
        HandleLocalInput();
        HandleRemoteInput();

        m_BulletPoolService.TickAll(); // Update all bullet positions.
        if (HasBulletHitRemotePlayer())
        {
            m_InputManager.bHasWinner = true; // local. remote will get set later
            m_NetworkManager.Send("", ENET_WINNER_CHANNEL);
        }
    }

    Draw();
    //DrawRecvData();

    if (m_InputManager.bHasWinner)
    {
        static const std::string LoseMessage = "Enemy Wins. You Lose.";
        static const std::string WinMessage = "You hit the enemy. You won.";
        const std::string* Message = m_InputManager.bLoseFlag ? &LoseMessage : &WinMessage;

        SetCursorPostion(SCREEN_X_MAX / 2 - ((int)Message->length() / 2), SCREEN_Y_MAX / 2);
        printf("%s", Message->c_str());
    }

    Sleep(20);
}

void GameWorld::Draw()
{
    if (!m_LocalSpaceShip || !m_RemoteSpaceShip)
    {
        return;
    }

    bool bBodyLocal, bBodyRemote;
    bool bLeftWingLocal, bRightWingLocal; // LeftWingLocal = RightWingRemote check
    bool bLeftWingRemote, bRightWingRemote; // RightWingLocal = LeftWingRemote check.
    bool bHeadLocal;
    bool bHeadRemote;
    bool bWarZone;
    bool bBulletLocal, bBulletRemote;

    FLocation2D localPosition = m_LocalSpaceShip->GetLocation();
    FLocation2D remotePosition = m_RemoteSpaceShip->GetLocation();

    for (int col = 0; col < SCREEN_Y_MAX; ++col)
    {
        for (int row = 0; row < SCREEN_X_MAX; ++row)
        {
            SetCursorPostion(row, col);
            bBodyLocal = FLocation2D::IsMatch(row, col, localPosition);
            bHeadLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(0, -1));
            bLeftWingLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(-1, 0));
            bRightWingLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(+1, 0));

            bBulletLocal = IsABullet(row, col, ENetRole::LOCAL);

            bBodyRemote = FLocation2D::IsMatch(row, col, remotePosition);
            bHeadRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(0, +1));
            bRightWingRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(1, 0));
            bLeftWingRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(-1, 0));

            bBulletRemote = IsABullet(row, col, ENetRole::REMOTE);

            bWarZone = (row > 0) && (row < SCREEN_X_MAX - 1) && (col > 0) && (col < SCREEN_Y_MAX - 1);

            if (bBodyLocal || bHeadLocal || bLeftWingLocal || bRightWingLocal || bBulletLocal)
            {
                SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED);
            }
            else if (bBodyRemote || bHeadRemote || bLeftWingRemote || bRightWingRemote || bBulletRemote)
            {
                SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            else
            {
                SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            }

            if (bBodyLocal || bBodyRemote)
            {
                printf("H");
            }
            else if (bLeftWingLocal || bRightWingLocal || bLeftWingRemote || bRightWingRemote)
            {
                printf("-");
            }
            else if (bHeadLocal)
            {
                printf("A");
            }
            else if (bHeadRemote)
            {
                printf("V");
            }
            else if (bBulletLocal)
            {
                printf("o");
            }
            else if (bBulletRemote)
            {
                printf("x");
            }
            else if (bWarZone)
            {
                printf(" ");
            }
            else
            {
                printf("#");
            }
        }
        putchar('\n');
    }
}

void GameWorld::DrawRecvData()
{
    SetCursorPostion(0, SCREEN_Y_MAX + m_BufferLineCounter);
    printf("%20s", " ");
    if (m_InputManager.GetCoordBuffer())
    {
        SetCursorPostion(0, SCREEN_Y_MAX + m_BufferLineCounter);
        printf("%s", m_InputManager.GetCoordBuffer().value());
        m_BufferLineCounter++;
        m_InputManager.UpdateCoordBufferQueue();

        if (m_BufferLineCounter >= CONSOLE_MAX_MESSAGE_LINES)
        {
            m_BufferLineCounter = 0;
        }
    }
}

bool GameWorld::HasBulletHitRemotePlayer()
{
    if (!m_RemoteSpaceShip)
    {
        return false;
    }

    // Get remote ship's position
    const FLocation2D& remotePosition = m_RemoteSpaceShip->GetLocation();

    // Check ship's body position
    if (Bullet* pBullet = m_BulletPoolService.GetActiveBulletAt(remotePosition))
    {
        if (pBullet->GetOwner() == m_LocalSpaceShip.get())
        {
            pBullet->Deactivate();
            return true;
        }
    }

    // Check ship's head position (one unit above body)
    if (Bullet* pBullet = m_BulletPoolService.GetActiveBulletAt(remotePosition + FLocation2D(0, +1)))
    {
        if (pBullet->GetOwner() == m_LocalSpaceShip.get())
        {
            pBullet->Deactivate();
            return true;
        }
    }

    // Check ship's wings positions
    if (Bullet* pBullet = m_BulletPoolService.GetActiveBulletAt(remotePosition + FLocation2D(-1, 0)))
    {
        if (pBullet->GetOwner() == m_LocalSpaceShip.get())
        {
            pBullet->Deactivate();
            return true;
        }
    }

    if (Bullet* pBullet = m_BulletPoolService.GetActiveBulletAt(remotePosition + FLocation2D(+1, 0)))
    {
        if (pBullet->GetOwner() == m_LocalSpaceShip.get())
        {
            pBullet->Deactivate();
            return true;
        }
    }

    return false;
}

bool GameWorld::IsABullet(const int row, const int col, const ENetRole netRole)
{
    if (Bullet* pBullet = m_BulletPoolService.GetActiveBulletAt(FLocation2D(row, col)))
    {
        if (pBullet->GetOwner()->GetNetRole() == netRole)
        {
            return true;
        }
    }
    return false;
}

void GameWorld::Exit()
{
    bPendingExit = true;
}

void GameWorld::HandleLocalInput()
{
    if (_kbhit())
    {
        m_InputManager.ReceiveLocalGameInput(_getch());
    }

    if (m_InputManager.GetLocalPendingInput())
    {
        char key = m_InputManager.GetLocalPendingInput().value(); // deref std::optional

        if (key == 'a')
        {
            m_LocalSpaceShip->MoveLeft();
        }
        else if (key == 'd')
        {
            m_LocalSpaceShip->MoveRight();
        }
        else if (key == 'w')
        {
            if (Bullet* pBullet = m_LocalSpaceShip->Shoot())
            {
                // Notify a remote Shoot
                const char* bulletLocationBuffer = pBullet->GetLocation().ToString();
                m_NetworkManager.Send(bulletLocationBuffer, ENetChannel::ENET_BULLET_CHANNEL);
            }
        }
        else if (key == 'q')
        {
            Exit();
        }
        m_InputManager.UpdateLocalInputQueue();
    }
}

void GameWorld::HandleRemoteInput()
{
    if (m_InputManager.GetRemotePendingInput())
    {
        char key = m_InputManager.GetRemotePendingInput().value(); // deref std::optional
        if (key == 'a')
        {
            m_RemoteSpaceShip->MoveLeft();
        }
        else if (key == 'd')
        {
            m_RemoteSpaceShip->MoveRight();
        }
        else if (key == 'w')
        {
            m_RemoteSpaceShip->Shoot();
        }

        m_InputManager.UpdateRemoteInputQueue();
    }
}
