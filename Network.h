
// Resources	:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
//				:	https://docs.microsoft.com/en-us/windows/win32/winsock/complete-client-code

#pragma once

#if SERVER
#undef UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
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

class NetworkCommon;
class InputManager;


enum ENetChannel
{
	ENET_INPUT_CHANNEL,	// Sending WASD Input, Single key only. {0}x\0 = 5 char
	ENET_BULLET_CHANNEL,	// Sending Bullet Coord, {1} ... = 128 max char
	ENET_NAME_CHANNEL,	// Request Player Name, {2}... = 16 max char
	ENET_EVENT_CHANNEL,	// Broadcast an event. {3}... = 16 max char
};

class NetworkManager
{
public:
	NetworkManager(int argc, char** argv);
	~NetworkManager();
	void Init(InputManager* InputManager);
	void TaskSend(); //Run by threads
	void TaskReceive(); // Run by threads.
	void Send(const char* Context, ENetChannel ID);


	class NetworkCommon*	GetNetwork()		{ return pNetwork; }
	bool					IsInitialized()		{ return bInitialized; }
private:

	static void				GetNetStringToken(char* Destination, ENetChannel NetChannel);

	NetworkCommon*			pNetwork;			// Directly owned.
	InputManager*			pInputs;			// Cache only
	bool					bInitialized = false;
	double					NetworkTime = 0;
};

class NetworkCommon
{
public:
	virtual int Initialize();
	virtual int Setup()							{ return RESULT_NOT_SUPPORTED; };
	virtual int Send(const char* Context)		{ return RESULT_NOT_SUPPORTED; };
	virtual int Receive(char* Context)			{ return RESULT_NOT_SUPPORTED; };
	virtual int Shutdown()						{ return RESULT_NOT_SUPPORTED; };

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

	void SetTarget(char* NewTarget)					{ Target = NewTarget; }		//	set ip as argument 1.
					

private:
	int CreateSocketAndConnect();

	char* Target;
	SOCKET ConnectSocket = INVALID_SOCKET;

};