// CmdSpaceship.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include <iostream>
#include <string>
#include "Game.h"
#include "PlayerProfile.h"
#include "Inputs.h"
#include "Network.h"

// Global ProfileTag
#if SERVER
std::string BuildTag = "[Server]";
#elif CLIENT
std::string BuildTag = "[Client]";
#endif

class Game;

int main(int argc, char** argv)
{

	PlayerProfile LocalPlayerProfile;

#if 0 // For customization name only.
	//std::string NameInput;
	//std::cout << BuildTag <<"Whats your name? Input->";
	//std::getline(std::cin, NameInput);

	//LocalPlayerProfile.SetPlayerName(NameInput);
	//std::cout << "My Name is " << LocalPlayerProfile.GetLocalPlayerName();
#endif

	InputManager InputManager;

	NetworkManager NetworkManager(argc, argv);
	NetworkManager.Init(&InputManager);
	if (!NetworkManager.IsInitialized())
		return RESULT_ERROR;

	std::thread NetworkReceiverThread(&NetworkManager::TaskReceive, std::ref(NetworkManager));
	std::thread NetworkSenderThread(&NetworkManager::TaskSend, std::ref(NetworkManager));


	Game SpaceshipGame;
	SpaceshipGame.Init(&InputManager, &NetworkManager);
	while (!SpaceshipGame.bExit)
		SpaceshipGame.Update();

	NetworkReceiverThread.join();
	NetworkSenderThread.join();

	return RESULT_SUCCEED;
}
