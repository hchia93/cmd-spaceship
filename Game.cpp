
#include <cstdlib>
#include <iostream>
#include <conio.h> // kbhit, MS-DOS specific now.

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
	LocalPlayer = std::make_unique<Spaceship>();
	RemotePlayer = std::make_unique<Spaceship>();

	LocalPlayer->SetLocation(FLocation2D(SCREEN_X_MAX / 2, SCREEN_Y_MAX - 2));
	RemotePlayer->SetLocation(FLocation2D(SCREEN_X_MAX / 2, 1));
}

Game::~Game()
{
	Spaceship::KillPool();
	LocalPlayer->~Spaceship();
	RemotePlayer->~Spaceship();
}

void Game::Init(InputManager* InputManager, NetworkManager* NetworkManager)
{
	pInputs = InputManager;
	pNetwork = NetworkManager;
}

void Game::Update()
{
	if (!LocalPlayer || !RemotePlayer)
		return;

	//  _kbhit and _getch is performance hit if it is on its seperate thread.
	// Locals
	if (_kbhit())
	{
		if (pInputs)
		{
			pInputs->ReceiveLocalGameInput(_getch());
		}
	}

	if (pInputs && pInputs->GetLocalPendingInput())
	{
		char key = pInputs->GetLocalPendingInput().value(); // deref std::optional
		if (key == 'a')
		{
			LocalPlayer->SetLocation(LocalPlayer->GetLocation() + FLocation2D(-1, 0));

		}
		else if (key == 'w')
		{
			// Local Shoot
			Bullet* pBullet = LocalPlayer->Shoot();
			if (pBullet)
			{
				pBullet->SetForwardDirection(EDR_Up);
				pBullet->Activate(LocalPlayer->GetLocation() + FLocation2D(0, 1), LocalPlayer.get());
	
			}

			// Notify a remote Shoot
			if (pNetwork)
			{
				const char* Location = LocalPlayer->GetLocation().ToString();
				pNetwork->Send(Location, ENetChannel::ENET_BULLET_CHANNEL);
			}

		}
		else if (key == 'd')
		{
			LocalPlayer->SetLocation(LocalPlayer->GetLocation() + FLocation2D(+1, 0));
		}
		pInputs->UpdateLocalInputQueue();
	}
	// Remote
	if (pInputs && pInputs->GetRemotePendingInput())
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
				pBullet->SetForwardDirection(EDR_Down);
				pBullet->Activate(RemotePlayer->GetLocation() + FLocation2D(0, -1), RemotePlayer.get());

			}
		}
		else if (key == 'd')
		{
			RemotePlayer->SetLocation(RemotePlayer->GetLocation() + FLocation2D(-1, 0));
		}
		pInputs->UpdateRemoteInputQueue();
	}

	Spaceship::UpdatePool();
		
	Draw();
	DrawRecvData();
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
				SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED);
			else if (bBodyRemote || bHeadRemote || bLeftWingRemote || bRightWingRemote || bBulletRemote)
				SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
			else
				SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);


			if (bBodyLocal || bBodyRemote)
				printf("H");
			else if (bLeftWingLocal || bRightWingLocal)
				printf("-");
			else if (bLeftWingRemote || bRightWingRemote)
				printf("+");
			else if (bHeadLocal)
				printf("A");
			else if (bHeadRemote)
				printf("V");
			else if (bWarZone)
				printf(" ");
			else if (bBulletLocal)
				printf("o");
			else if (bBulletRemote)
				printf("x");
			else
				printf("#");
		}
		putchar('\n');
	}
}

void Game::DrawRecvData()
{
	SetCursorPostion(0, SCREEN_Y_MAX + BufferLineCounter);
	printf("%20s", " ");
	if (pInputs && pInputs->GetCoordBuffer())
	{
		SetCursorPostion(0, SCREEN_Y_MAX + BufferLineCounter);
		printf("%s", pInputs->GetCoordBuffer().value());
		BufferLineCounter++;
		pInputs->UpdateCoordBufferQueue();

		if (BufferLineCounter > 10)
			BufferLineCounter = 0;
	}
}
