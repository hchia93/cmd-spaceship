#pragma once

#if SERVER_BUILD
#undef UNICODE
#endif //SERVER_BUILD

#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFLEN 16
#define DEFAULT_PORT "27015"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "Utils.h"

#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#if SERVER 
//#pragma comment (lib, "Mswsock.lib")
#elif CLIENT // Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif

#define RESULT_SUCCEED 0 
#define RESULT_ERROR 1
#define RESULT_NOT_SUPPORTED 2

#define DEBUG_NET_LOG 0
#define DEBUG_LOG_FILE 1

#if DEBUG_NET_LOG
#define NET_LOG(...) printf(__VA_ARGS__);
#else 
#define NET_LOG(...) 
#endif //DEBUG_NET_LOG

#if DEBUG_LOG_FILE
#include <fstream>
#endif //DEBUG_LOG_FILE

/// Resources	:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
///				:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code


class NetworkCommon;
class InputManager;

enum ENetChannel
{
    ENET_INPUT_CHANNEL, // Sending WASD Input, Single key only. {0}x\0 = 5 char
    ENET_BULLET_CHANNEL, // Sending Bullet Coord, {1} ... = 8 max char
    ENET_WINNER_CHANNEL,
};

class NetworkManager
{
public:
    NetworkManager(int argc, char** argv);
    ~NetworkManager();

    void Init(InputManager& InputManager);
    void TaskSend(); //Run by threads
    void TaskReceive(); // Run by threads.
    void Send(const char* Context, ENetChannel ID);

    NetworkCommon* GetNetwork() { return pNetwork.get(); }
    bool IsInitialized() { return bInitialized; }

private:

    static void GetNetStringToken(char* Destination, ENetChannel NetChannel);

    std::unique_ptr<NetworkCommon> pNetwork;
    InputManager* pInputs; // Cache only
    bool bInitialized = false;
#if DEBUG_LOG_FILE
    std::ofstream netSendLog;
    std::ofstream netRecvLog;
#endif
};

class NetworkCommon
{
public:
    virtual int Initialize();
    virtual int Setup() { return RESULT_NOT_SUPPORTED; };
    virtual int Send(const char* Context) { return RESULT_NOT_SUPPORTED; };
    virtual int Receive(char* Context) { return RESULT_NOT_SUPPORTED; };
    virtual int Shutdown() { return RESULT_NOT_SUPPORTED; };

protected:
    struct addrinfo* AddressResult;
};

class NetworkServer : public NetworkCommon
{
public:
    virtual int Initialize() override;
    virtual int Setup() override;
    virtual int Send(const char* Context) override;
    virtual int Receive(char* Context) override;
    virtual int Shutdown() override;

private:
    int CreateListenSocketAndAcceptClient();

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
};

class NetworkClient : public NetworkCommon
{
public:
    virtual int Initialize() override;
    virtual int Setup() override;
    virtual int Send(const char* Context) override;
    virtual int Receive(char* Context) override;
    virtual int Shutdown() override;

    void SetTarget(char* NewTarget) { Target = NewTarget; }		//	set ip as argument 1.

private:
    int CreateSocketAndConnect();

    char* Target;
    SOCKET ConnectSocket = INVALID_SOCKET;
};