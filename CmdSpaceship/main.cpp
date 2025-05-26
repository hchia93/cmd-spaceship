// CmdSpaceship.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <thread>
#include <iostream>
#include <string>
#include "Game.h"
#include "Player.h"
#include "Inputs.h"
#include "Network.h"

// Global ProfileTag
#if SERVER_BUILD
std::string BuildTag = "[Server]";
#elif CLIENT_BUILD
std::string BuildTag = "[Client]";
#endif

class Game;

int main(int argc, char** argv)
{
    Player LocalPlayer;;

    InputManager InputManager;

    NetworkManager NetworkManager(argc, argv);
    NetworkManager.Init(InputManager);
    if (!NetworkManager.IsInitialized())
    {
        return RESULT_ERROR;
    }

    std::thread NetworkReceiverThread(&NetworkManager::TaskReceive, std::ref(NetworkManager));
    std::thread NetworkSenderThread(&NetworkManager::TaskSend, std::ref(NetworkManager));

    Game SpaceshipGame;
    SpaceshipGame.Init(InputManager, NetworkManager);

    while (!SpaceshipGame.bExit)
    {
        SpaceshipGame.Update();
    }

    NetworkReceiverThread.join();
    NetworkSenderThread.join();

    return RESULT_SUCCEED;
}
