#pragma once

#define WIN32_LEAN_AND_MEAN  // Prevents windows.h from including winsock.h
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

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
    ENET_INPUT_CHANNEL, // Sending WASD Input, Single key only. {0}x\0 = 5 char
    ENET_BULLET_CHANNEL, // Sending Bullet Coord, {1} ... = 8 max char
    ENET_WINNER_CHANNEL,
};

class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    void Initialize();
    void SetInputManager(InputManager* manager);

    void TaskSend(); //Run by threads
    void TaskReceive(); // Run by threads.
    void Send(const char* Context, ENetChannel ID);

    NetworkCommon* GetNetwork() { return m_Network.get(); }
    const bool IsInitialized() { return bInitialized; }

private:

    static void GetNetStringToken(char* Destination, ENetChannel NetChannel);

    std::unique_ptr<NetworkCommon> m_Network;
    InputManager* m_InputManager;

    bool bInitialized = false;

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
    virtual int Send(const char* context) { return RESULT_NOT_SUPPORTED; };
    virtual int Receive(char* context) { return RESULT_NOT_SUPPORTED; };
    virtual int Shutdown() { return RESULT_NOT_SUPPORTED; };

protected:
    struct addrinfo* m_AddressResult;
};

class NetworkServer : public NetworkCommon
{
public:
    virtual int Initialize() override;
    virtual int Setup() override;
    virtual int Send(const char* context) override;
    virtual int Receive(char* context) override;
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
    virtual int Send(const char* context) override;
    virtual int Receive(char* context) override;
    virtual int Shutdown() override;

    void SetTarget(char* newTarget) { m_Target = newTarget; } //	set ip as argument 1.

private:
    int CreateSocketAndConnect();

    char* m_Target;
    SOCKET m_ConnectSocket = INVALID_SOCKET;
};