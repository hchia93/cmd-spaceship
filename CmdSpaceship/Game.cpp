
#include <cstdlib>
#include <iostream>
#include <conio.h> // kbhit, MS-DOS specific now.
#include <cassert>
#include "Game.h"
#include "Network.h"
#include "Inputs.h"

#include "Spaceship.h"
#include "Bullet.h"
#include "Utils.h"

// system("cls") causes flicker due to io, use windows api here.
// https://stackoverflow.com/questions/34842526/update-console-without-flickering-c
// https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#_win32_character_attributes

void SetCursorPostion(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

void SetConsoleColor(unsigned short color)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    SetConsoleTextAttribute(hOut, color);
}

void SetCursorInfo()
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    // Disable cursor blinks in cmd.
    CONSOLE_CURSOR_INFO Info;
    Info.bVisible = false;
    Info.dwSize = 0;
    SetConsoleCursorInfo(hOut, &Info);
}

void SetScreenBufferSize(unsigned int X, unsigned int Y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = X;
    coord.Y = Y;
    SetConsoleScreenBufferSize(hOut, coord);
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

Game::Game()
{
    CreatePlayer();
}

Game::~Game()
{
    Spaceship::KillPool();
}

void Game::CreatePlayer()
{
    LocalPlayer = std::make_unique<Spaceship>();
    RemotePlayer = std::make_unique<Spaceship>();

    LocalPlayer->SetLocation(FLocation2D(SCREEN_X_MAX / 2, SCREEN_Y_MAX - 2));
    RemotePlayer->SetLocation(FLocation2D(SCREEN_X_MAX / 2, 1));

    SetCursorInfo();
    SetScreenBufferSize(CONSOLE_SCREEN_BUFFER_X, CONSOLE_SCREEN_BUFFER_Y);
}

void Game::Init(InputManager& InputManager, NetworkManager& NetworkManager)
{
    pInputs = &InputManager;
    pNetwork = &NetworkManager;

    assert(pNetwork != nullptr && pInputs != nullptr);
}

void Game::Update()
{
    if (!LocalPlayer || !RemotePlayer)
        return;

    HandleLocalInput();
    HandleRemoteInput();

    // Stop rechecks if a hit to player has been made. (This flag is from local or remote.
    if (!pInputs->bHasWinner)
    {
        Spaceship::UpdatePool(); // Update all bullet positions.
        FHitResult HitResult = Spaceship::CheckHit(RemotePlayer.get());

        if (HitResult.bHitRemotePlayer)
        {
            pInputs->bHasWinner = true; // local. remote will get set later
            pNetwork->Send("", ENET_WINNER_CHANNEL);
        }
    }

    Draw();
    //DrawRecvData();

    if (pInputs->bHasWinner)
    {
        static const std::string LoseMessage = "Enemy Wins. You Lose.";
        static const std::string WinMessage = "You hit the enemy. You won.";
        const std::string* Message = pInputs->bLoseFlag ? &LoseMessage : &WinMessage;

        SetCursorPostion(SCREEN_X_MAX / 2 - ((int)Message->length() / 2), SCREEN_Y_MAX / 2);
        printf("%s", Message->c_str());
    }

    Sleep(20);
}

void Game::Draw()
{
    if (!LocalPlayer || !RemotePlayer)
        return;

    bool bBodyLocal, bBodyRemote;
    bool bLeftWingLocal, bRightWingLocal; // LeftWingLocal = RightWingRemote check
    bool bLeftWingRemote, bRightWingRemote; // RightWingLocal = LeftWingRemote check.
    bool bHeadLocal;
    bool bHeadRemote;
    bool bWarZone;
    bool bBulletLocal, bBulletRemote;

    FLocation2D lp = LocalPlayer->GetLocation();
    FLocation2D rp = RemotePlayer->GetLocation();

    for (int col = 0; col < SCREEN_Y_MAX; ++col)
    {
        for (int row = 0; row < SCREEN_X_MAX; ++row)
        {
            SetCursorPostion(row, col);
            bBodyLocal = FLocation2D::IsMatch(row, col, lp);
            bHeadLocal = FLocation2D::IsMatch(row, col, lp + FLocation2D(0, -1));
            bLeftWingLocal = FLocation2D::IsMatch(row, col, lp + FLocation2D(-1, 0));
            bRightWingLocal = FLocation2D::IsMatch(row, col, lp + FLocation2D(+1, 0));

            bBulletLocal = Spaceship::FindLocalBullet(row, col);

            bBodyRemote = FLocation2D::IsMatch(row, col, rp);
            bHeadRemote = FLocation2D::IsMatch(row, col, rp + FLocation2D(0, +1));
            bRightWingRemote = FLocation2D::IsMatch(row, col, rp + FLocation2D(1, 0));
            bLeftWingRemote = FLocation2D::IsMatch(row, col, rp + FLocation2D(-1, 0));

            bBulletRemote = Spaceship::FindRemoteBullet(row, col);

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
            else if (bLeftWingLocal || bRightWingLocal)
            {
                printf("-");
            }
            else if (bLeftWingRemote || bRightWingRemote)
            {
                printf("+");
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

void Game::DrawRecvData()
{
    SetCursorPostion(0, SCREEN_Y_MAX + BufferLineCounter);
    printf("%20s", " ");
    if (pInputs->GetCoordBuffer())
    {
        SetCursorPostion(0, SCREEN_Y_MAX + BufferLineCounter);
        printf("%s", pInputs->GetCoordBuffer().value());
        BufferLineCounter++;
        pInputs->UpdateCoordBufferQueue();

        if (BufferLineCounter >= CONSOLE_MAX_MESSAGE_LINES)
            BufferLineCounter = 0;
    }
}

void Game::Exit()
{
    bExit = true;
}

void Game::HandleLocalInput()
{
    if (_kbhit())
    {
        pInputs->ReceiveLocalGameInput(_getch());
    }

    if (pInputs->GetLocalPendingInput())
    {
        char key = pInputs->GetLocalPendingInput().value(); // deref std::optional

        if (key == 'a')
        {
            LocalPlayer->SetLocation(LocalPlayer->GetLocation() + FLocation2D(-1, 0));
        }
        else if (key == 'w')
        {
            // Local Shoot
            if (!pInputs->bHasWinner)
            {
                Bullet* pBullet = LocalPlayer->Shoot();
                if (pBullet)
                {
                    pBullet->SetForwardDirection(Up);
                    pBullet->Activate(LocalPlayer->GetLocation() + FLocation2D(0, -2), LocalPlayer.get());
                }

                // Notify a remote Shoot
                const char* Location = LocalPlayer->GetLocation().ToString();
                pNetwork->Send(Location, ENetChannel::ENET_BULLET_CHANNEL);
            }
        }
        else if (key == 'd')
        {
            LocalPlayer->SetLocation(LocalPlayer->GetLocation() + FLocation2D(+1, 0));
        }
        else if (key == 'q')
        {
            Exit();
        }
        pInputs->UpdateLocalInputQueue();
    }
}

void Game::HandleRemoteInput()
{
    if (pInputs->GetRemotePendingInput())
    {
        char key = pInputs->GetRemotePendingInput().value(); // deref std::optional
        if (key == 'a')
        {
            RemotePlayer->SetLocation(RemotePlayer->GetLocation() + FLocation2D(+1, 0));
        }
        else if (key == 'w')
        {
            // Simulated shoot.
            Bullet* pBullet = RemotePlayer->Shoot();
            if (pBullet)
            {
                pBullet->SetForwardDirection(Down);
                pBullet->Activate(RemotePlayer->GetLocation() + FLocation2D(0, +2), RemotePlayer.get());
            }
        }
        else if (key == 'd')
        {
            RemotePlayer->SetLocation(RemotePlayer->GetLocation() + FLocation2D(-1, 0));
        }
        pInputs->UpdateRemoteInputQueue();
    }
}
