#pragma once

#define WIN32_LEAN_AND_MEAN  // Prevents windows.h from including winsock.h
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <string_view>
#include <chrono>
#include <atomic>
#include <functional>

#include "Macro.h"
#include "Utils.h"

#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#if SERVER_BUILD
//#pragma comment (lib, "Mswsock.lib")
#elif CLIENT_BUILD // Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

#if DEBUG_LOG_FILE
#include <fstream>
#endif //DEBUG_LOG_FILE

/// Resources	:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
///				:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

class InputManager;
class NetworkCommon;

enum ENetChannel
{
    INPUT_CHANNEL,    // Sending WASD Input, Single key only. {0}x\0 = 5 char
    BULLET_CHANNEL,   // Sending Bullet Coord, {1} ... = 8 max char
    WINNER_CHANNEL,
    RESET_CHANNEL,    // Synchronize game reset between players
};

class NetworkManager
{
public:
    static constexpr std::chrono::milliseconds NETWORK_UPDATE_INTERVAL{5}; // 200Hz network update rate
    
    NetworkManager();
    ~NetworkManager();

    void Initialize();
    void SetInputManager(InputManager* manager);
    void SetOnWinMessageReceived(std::function<void()> callback) { m_OnWinMessageReceived = callback; }

    void TaskSend(); //Run by threads
    void TaskReceive(); // Run by threads.
    void Send(std::string_view Context, ENetChannel ID);

    NetworkCommon* GetNetwork() { return m_Network.get(); }
    const bool IsInitialized() { return bInitialized; }
    void RequestShutdown() { bShouldShutdown = true; }

private:
    static std::string GetNetStringToken(ENetChannel NetChannel);

    std::unique_ptr<NetworkCommon> m_Network;
    InputManager* m_InputManager;
    std::function<void()> m_OnWinMessageReceived;

    std::atomic<bool> bInitialized{false};
    std::atomic<bool> bShouldShutdown{false};

    std::chrono::steady_clock::time_point m_LastSendTime;
    std::chrono::steady_clock::time_point m_LastReceiveTime;

#if DEBUG_LOG_FILE
    std::ofstream netSendLog;
    std::ofstream netRecvLog;
#endif //DEBUG_LOG_FILE
};

class NetworkCommon
{
public:
    virtual int Initialize();
    virtual int Setup() { return RESULT_NOT_SUPPORTED; };
    virtual int Send(std::string_view context) { return RESULT_NOT_SUPPORTED; };
    virtual int Receive(std::string& context) { return RESULT_NOT_SUPPORTED; };
    virtual int Shutdown() { return RESULT_NOT_SUPPORTED; };

protected:
    struct addrinfo* m_AddressResult;
};

class NetworkServer : public NetworkCommon
{
public:
    virtual int Initialize() override;
    virtual int Setup() override;
    virtual int Send(std::string_view context) override;
    virtual int Receive(std::string& context) override;
    virtual int Shutdown() override;

private:
    int CreateListenSocketAndAcceptClient();

    SOCKET m_ListenSocket = INVALID_SOCKET;
    SOCKET m_ClientSocket = INVALID_SOCKET;
};

class NetworkClient : public NetworkCommon
{
public:
    virtual int Initialize() override;
    virtual int Setup() override;
    virtual int Send(std::string_view context) override;
    virtual int Receive(std::string& context) override;
    virtual int Shutdown() override;

    void SetTarget(std::string newTarget) { m_Target = std::move(newTarget); }

private:
    int CreateSocketAndConnect();

    std::string m_Target;
    SOCKET m_ConnectSocket = INVALID_SOCKET;
};