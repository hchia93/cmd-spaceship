#pragma once

// Standard Library
#include <thread>
#include <memory>
#include <unordered_map>
#include <functional>

// Project Headers
#include "Inputs.h"
#include "Network.h"
#include "BulletPoolService.h"
#include "Spaceship.h"

class GameWorld
{
public:
    GameWorld();
    ~GameWorld() = default;

    void CreateSpaceShips();
    void Finalize();
  
    void Update();
    void Draw();
    void DrawRecvData();
    void Exit();

    bool IsExiting() { return bPendingExit; }

private:
    void HandleLocalInput();
    void HandleRemoteInput();

    void InitializeNetwork();
    void FinalizeNetwork();

    bool HasBulletHitRemotePlayer();
    bool IsABullet(const int row, const int col, const ENetRole netRole);
    char GetDisplayCharAt(bool isBodyLocal, bool isBodyRemote,
                         bool isWingLocal, bool isWingRemote,
                         bool isHeadLocal, bool isHeadRemote,
                         bool isBulletLocal, bool isBulletRemote,
                         bool isWarZone);
  
    InputManager m_InputManager;
    NetworkManager m_NetworkManager;
    std::thread m_NetworkReceiverThread;
    std::thread m_NetworkSenderThread;

    std::unique_ptr<Spaceship> m_LocalSpaceShip;
    std::unique_ptr<Spaceship> m_RemoteSpaceShip;
    BulletPoolService m_BulletPoolService;

    int m_BufferLineCounter = 0;
    bool bPendingExit = false;
};

