#define WIN32_LEAN_AND_MEAN
// Windows specific
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h> // kbhit, MS-DOS specific

// Standard Library
#include <iostream>
#include <string>
#include <array>
#include <chrono>
#include <thread>

// Project Headers
#include "GameWorld.h"
#include "Bullet.h"
#include "Utils.h"

// system("cls") causes flicker due to io, use windows api here.
// https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
// https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#_win32_character_attributes

// Add this after the includes and before the console functions
class ConsoleHandle 
{
public:
    static ConsoleHandle& GetInstance() 
    {
        static ConsoleHandle m_Instance;
        return m_Instance;
    }
    
    HANDLE Get() const { return m_Handle; }
    
private:
    ConsoleHandle() : m_Handle(GetStdHandle(STD_OUTPUT_HANDLE)) {}
    ~ConsoleHandle() = default;
    ConsoleHandle(const ConsoleHandle&) = delete;
    ConsoleHandle& operator=(const ConsoleHandle&) = delete;
    
    HANDLE m_Handle;
};

void SetCursorPostion(int x, int y)
{
    std::cout.flush();
    COORD coordinate{ static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(ConsoleHandle::GetInstance().Get(), coordinate);
}

void SetConsoleColor(unsigned short color)
{
    std::cout.flush();
    SetConsoleTextAttribute(ConsoleHandle::GetInstance().Get(), color);
}

void SetCursorInfo(bool visible = false)
{
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 1;    // The percentage of the character cell that is filled by the cursor
    info.bVisible = visible;  // Visibility of the cursor
    SetConsoleCursorInfo(ConsoleHandle::GetInstance().Get(), &info);
}

void SetScreenBufferSize(unsigned int X, unsigned int Y)
{
    COORD coordinate{static_cast<SHORT>(X), static_cast<SHORT>(Y)};
    SetConsoleScreenBufferSize(ConsoleHandle::GetInstance().Get(), coordinate);
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

// Add after the ConsoleHandle class and before GameWorld methods
struct SpaceshipDisplay {
    static constexpr char Body = 'H';
    static constexpr char Wing = '-';
    static constexpr char LocalHead = 'A';
    static constexpr char RemoteHead = 'V';
    static constexpr char LocalBullet = 'o';
    static constexpr char RemoteBullet = 'x';
    static constexpr char Empty = ' ';
    static constexpr char Border = '#';
};

// Add this helper function before Draw()
char GameWorld::GetDisplayCharAt(bool isBodyLocal, bool isBodyRemote, 
                               bool isWingLocal, bool isWingRemote,
                               bool isHeadLocal, bool isHeadRemote,
                               bool isBulletLocal, bool isBulletRemote,
                               bool isWarZone)
{
    if (isBodyLocal || isBodyRemote) return SpaceshipDisplay::Body;
    if (isWingLocal || isWingRemote) return SpaceshipDisplay::Wing;
    if (isHeadLocal) return SpaceshipDisplay::LocalHead;
    if (isHeadRemote) return SpaceshipDisplay::RemoteHead;
    if (isBulletLocal) return SpaceshipDisplay::LocalBullet;
    if (isBulletRemote) return SpaceshipDisplay::RemoteBullet;
    if (isWarZone) return SpaceshipDisplay::Empty;
    return SpaceshipDisplay::Border;
}

GameWorld::GameWorld()
{
    m_LastFrameTime = std::chrono::steady_clock::now();
    InitializeConsoleInput();
    CreateSpaceShips();
    InitializeNetwork();
}

GameWorld::~GameWorld()
{
    Exit();
    Finalize();
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
        // Set up callback for when we receive a win message
        m_NetworkManager.SetOnWinMessageReceived([this]() 
        {
            m_RemoteScore++; // Increment remote score when they hit us
        });

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
        m_NetworkManager.RequestShutdown();
        m_NetworkReceiverThread.join();
        m_NetworkSenderThread.join();
    }
}

void GameWorld::RequestReset()
{
    bLocalReadyToReset = true;
    m_NetworkManager.Send("", RESET_CHANNEL);
}

void GameWorld::HandleResetSync()
{
    if (bLocalReadyToReset && m_InputManager.IsRemoteReadyToReset())
    {
        // Both players are ready, perform the actual reset
        bLocalReadyToReset = false;
        m_InputManager.ClearResetFlags();
        
        // Reset game state but keep scores
        m_InputManager.bHasWinner = false;
        m_InputManager.bLoseFlag = false;

        // Clear any pending inputs
        m_InputManager.ClearAllInputs();

        // Reset spaceship positions
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

        // Deactivate all bullets
        m_BulletPoolService.DeactivateAll();
    }
}

void GameWorld::DrawScoreboard()
{
    // Draw scoreboard outside game bounds
    SetCursorInfo(false);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    
    // Draw at the bottom of the screen, below the game area
    const int scoreboardY = SCREEN_Y_MAX + CONSOLE_MAX_MESSAGE_LINES + 1;
    SetCursorPostion(0, scoreboardY);
    printf("Scoreboard: You: %d  Enemy: %d", m_LocalScore, m_RemoteScore);
}

void GameWorld::Update()
{
    if (!m_LocalSpaceShip || !m_RemoteSpaceShip)
    {
        return;
    }

    // Ensure cursor is hidden at the start of each frame
    SetCursorInfo(false);

    // Calculate frame timing
    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_LastFrameTime);
    
    // If we're running faster than our target frame rate, wait
    if (deltaTime < FRAME_TIME)
    {
        auto sleepTime = FRAME_TIME - deltaTime;
        std::this_thread::sleep_for(sleepTime);
        currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_LastFrameTime);
    }
    
    m_LastFrameTime = currentTime;

    if (!m_InputManager.bHasWinner)
    {
        HandleLocalInput();
        HandleRemoteInput();

        m_BulletPoolService.TickAll();
        if (HasBulletHitRemotePlayer())
        {
            m_InputManager.bHasWinner = true;
            m_NetworkManager.Send("", WINNER_CHANNEL);
            m_LocalScore++; // Increment local score when hitting enemy
        }
    }
    else
    {
        // Check for reset command when game is over
        if (_kbhit())
        {
            char key = _getch();
            if (key == 'r' || key == 'R')
            {
                RequestReset();
            }
        }

        // Check if both players are ready to reset
        HandleResetSync();
    }

    Draw();
    DrawScoreboard();

    if (m_InputManager.bHasWinner)
    {
        static constexpr std::string_view LoseMessage = "Enemy Wins. You Lose.";
        static constexpr std::string_view WinMessage = "You hit the enemy. You won.";
        const auto& message = m_InputManager.bLoseFlag ? LoseMessage : WinMessage;

        SetCursorPostion(SCREEN_X_MAX / 2 - (static_cast<int>(message.length()) / 2), SCREEN_Y_MAX / 2);
        std::cout << message;
        
        if (bLocalReadyToReset)
        {
            // Show waiting message
            static constexpr std::string_view WaitingMessage = "Waiting for other player...";
            SetCursorPostion(SCREEN_X_MAX / 2 - (static_cast<int>(WaitingMessage.length()) / 2), SCREEN_Y_MAX / 2 + 1);
            std::cout << WaitingMessage;
        }
        else
        {
            // Show reset instruction
            static constexpr std::string_view ResetMessage = "Press 'R' to play again";
            SetCursorPostion(SCREEN_X_MAX / 2 - (static_cast<int>(ResetMessage.length()) / 2), SCREEN_Y_MAX / 2 + 1);
            std::cout << ResetMessage;
        }
    }
}

void GameWorld::Draw()
{
    if (!m_LocalSpaceShip || !m_RemoteSpaceShip)
    {
        return;
    }

    // Ensure cursor is hidden before drawing
    SetCursorInfo(false);

    const FLocation2D localPosition = m_LocalSpaceShip->GetLocation();
    const FLocation2D remotePosition = m_RemoteSpaceShip->GetLocation();

    for (int col = 0; col < SCREEN_Y_MAX; ++col)
    {
        for (int row = 0; row < SCREEN_X_MAX; ++row)
        {
            const bool bBodyLocal = FLocation2D::IsMatch(row, col, localPosition);
            const bool bHeadLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(0, -1));
            const bool bLeftWingLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(-1, 0));
            const bool bRightWingLocal = FLocation2D::IsMatch(row, col, localPosition + FLocation2D(+1, 0));
            const bool bBulletLocal = IsABullet(row, col, ENetRole::LOCAL);

            const bool bBodyRemote = FLocation2D::IsMatch(row, col, remotePosition);
            const bool bHeadRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(0, +1));
            const bool bLeftWingRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(-1, 0));
            const bool bRightWingRemote = FLocation2D::IsMatch(row, col, remotePosition + FLocation2D(+1, 0));
            const bool bBulletRemote = IsABullet(row, col, ENetRole::REMOTE);

            const bool bWarZone = (row > 0) && (row < SCREEN_X_MAX - 1) && (col > 0) && (col < SCREEN_Y_MAX - 1);

            SetCursorPostion(row, col);

            // Set color based on what we're drawing
            const bool isLocalPart = bBodyLocal || bHeadLocal || bLeftWingLocal || bRightWingLocal || bBulletLocal;
            const bool isRemotePart = bBodyRemote || bHeadRemote || bLeftWingRemote || bRightWingRemote || bBulletRemote;
            
            if (isLocalPart)
            {
                SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED);
            }
            else if (isRemotePart)
            {
                SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            else
            {
                SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            }

            // Get and print the character
            const char displayChar = GetDisplayCharAt(bBodyLocal, bBodyRemote,
                                                    bLeftWingLocal || bRightWingLocal,
                                                    bLeftWingRemote || bRightWingRemote,
                                                    bHeadLocal, bHeadRemote,
                                                    bBulletLocal, bBulletRemote,
                                                    bWarZone);
            putchar(displayChar);
        }
        putchar('\n');
    }
}

void GameWorld::DrawRecvData()
{
    // Ensure cursor is hidden before drawing
    SetCursorInfo(false);

    SetCursorPostion(0, SCREEN_Y_MAX + m_BufferLineCounter);
    printf("%20s", " ");
    if (auto coordBuffer = m_InputManager.GetCoordBuffer())
    {
        SetCursorPostion(0, SCREEN_Y_MAX + m_BufferLineCounter);
        std::string coordStr{coordBuffer.value()};
        printf("%s", coordStr.c_str());
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

    const FLocation2D& remotePosition = m_RemoteSpaceShip->GetLocation();
    
    // Define all positions to check
    const std::array<FLocation2D, 4> checkPositions = {
        remotePosition,                          // body
        remotePosition + FLocation2D(0, +1),     // head
        remotePosition + FLocation2D(-1, 0),     // left wing
        remotePosition + FLocation2D(+1, 0)      // right wing
    };
    
    // Check all positions
    for (const auto& pos : checkPositions)
    {
        if (auto* pBullet = m_BulletPoolService.GetActiveBulletAt(pos))
        {
            if (pBullet->GetOwner() == m_LocalSpaceShip.get())
            {
                pBullet->Deactivate();
                return true;
            }
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
    if (m_NetworkManager.IsInitialized())
    {
        m_NetworkManager.RequestShutdown();
    }
}

void GameWorld::InitializeConsoleInput()
{
    // Get the console input handle
    m_ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (m_ConsoleInput == INVALID_HANDLE_VALUE)
    {
        // Handle error
        return;
    }

    // Get the current console mode
    DWORD consoleMode;
    if (!GetConsoleMode(m_ConsoleInput, &consoleMode))
    {
        // Handle error
        return;
    }

    // Disable window input (prevents focus events from generating input)
    // Enable only the input modes we want
    consoleMode &= ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
    consoleMode |= ENABLE_PROCESSED_INPUT;

    if (!SetConsoleMode(m_ConsoleInput, consoleMode))
    {
        // Handle error
        return;
    }
}

void GameWorld::HandleLocalInput()
{
    INPUT_RECORD inputRecord;
    DWORD numEventsRead;

    // Check if there are any input events
    if (!GetNumberOfConsoleInputEvents(m_ConsoleInput, &numEventsRead) || numEventsRead == 0)
    {
        return;
    }

    // Read console input
    if (ReadConsoleInput(m_ConsoleInput, &inputRecord, 1, &numEventsRead) && numEventsRead > 0)
    {
        // Only process key events
        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown)
        {
            char key = static_cast<char>(inputRecord.Event.KeyEvent.uChar.AsciiChar);
            
            // Only process valid game inputs
            switch (key)
            {
                case 'a':
                case 'd':
                case 'w':
                case 'q':
                case 'r':
                case 'R':
                    m_InputManager.ReceiveLocalGameInput(key);
                    break;
                default:
                    break;
            }
        }
    }

    if (auto input = m_InputManager.GetLocalPendingInput())
    {
        const char key = input.value();
        switch (key)
        {
            case 'a':
                m_LocalSpaceShip->MoveLeft();
                break;
            case 'd':
                m_LocalSpaceShip->MoveRight();
                break;
            case 'w':
                if (auto* pBullet = m_LocalSpaceShip->Shoot())
                {
                    m_NetworkManager.Send(pBullet->GetLocation().ToString(), ENetChannel::BULLET_CHANNEL);
                }
                break;
            case 'q':
                Exit();
                break;
        }
        m_InputManager.UpdateLocalInputQueue();
    }
}

void GameWorld::HandleRemoteInput()
{
    if (auto input = m_InputManager.GetRemotePendingInput())
    {
        const char key = input.value();
        switch (key)
        {
            case 'a':
                m_RemoteSpaceShip->MoveLeft();
                break;
            case 'd':
                m_RemoteSpaceShip->MoveRight();
                break;
            case 'w':
                m_RemoteSpaceShip->Shoot();
                break;
        }
        m_InputManager.UpdateRemoteInputQueue();
    }
}
