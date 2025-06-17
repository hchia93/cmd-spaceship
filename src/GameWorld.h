#pragma once

// Standard Library
#include <thread>
#include <memory>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <atomic>

// Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Project Headers
#include "Inputs.h"
#include "Network.h"
#include "BulletPoolService.h"
#include "Spaceship.h"

class GameWorld
{
public:
    static constexpr std::chrono::milliseconds FRAME_TIME{16};  // ~60 FPS
    
    GameWorld();
    ~GameWorld();

    void CreateSpaceShips();
    void Finalize();
  
    void Update();
    void Draw();
    void DrawRecvData();
    void DrawScoreboard();
    void Exit();
    void Reset();

    bool IsExiting() { return bPendingExit; }

private:
    void HandleLocalInput();
    void HandleRemoteInput();
    void InitializeNetwork();
    void FinalizeNetwork();
    void RequestReset();
    void HandleResetSync();
    void InitializeConsoleInput();

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
    std::atomic<bool> bPendingExit{false};
    
    // Frame timing
    std::chrono::steady_clock::time_point m_LastFrameTime;

    // Scoreboard
    int m_LocalScore = 0;
    int m_RemoteScore = 0;

    // Reset synchronization
    std::atomic<bool> bLocalReadyToReset{false};
    std::atomic<bool> bRemoteReadyToReset{false};

    // Console input handle
    HANDLE m_ConsoleInput;
};

