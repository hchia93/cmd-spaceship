// CmdSpaceship.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include "Inputs.h"
#include "Network.h"

int main(int argc, char** argv)
{
	InputManager InputManager;
	std::thread InputThread(&InputManager::ListenUserInput, std::ref(InputManager));
	std::thread ChatThread(&InputManager::ListenUserInputLines, std::ref(InputManager));

	NetworkManager NetworkManager(argc, argv);
	NetworkManager.Init(&InputManager);
	if (!NetworkManager.IsInitialized())
		return RESULT_ERROR;

	std::thread NetworkReceiverThread(&NetworkManager::TaskReceive, std::ref(NetworkManager));
	std::thread NetworkSenderThread(&NetworkManager::TaskSend, std::ref(NetworkManager));

	InputThread.join();
	ChatThread.detach();
	NetworkReceiverThread.join();
	NetworkSenderThread.join();

	return RESULT_SUCCEED;
}