
#include "Network.h"

#include <chrono>
#include <sstream>
#include <cassert>

#include "Inputs.h"

NetworkManager::NetworkManager(int argc, char** argv)
{
#if SERVER
	pNetwork = new NetworkServer();
#elif CLIENT
	pNetwork = new NetworkClient();

	/// Expecting Client to Run with additional parameter
	/// Default : localhost
	if (argc != 2) // Validate the parameters
		printf("usage: %s server-name\n", argv[0]);

	((NetworkClient*)pNetwork)->SetTarget(argv[1]);
#endif
}

NetworkManager::~NetworkManager()
{
	if(pNetwork)
		pNetwork->Shutdown();

	delete pNetwork;
}

void NetworkManager::Init(InputManager* InputManager)
{
	if (pNetwork)
	{
		int ret = pNetwork->Initialize();
		if (ret != 0)
		{
			bInitialized = false;
			return;
		}

		ret = pNetwork->Setup();
		bInitialized = (ret == RESULT_SUCCEED);
	}

	pInputs = InputManager;
}

void NetworkManager::TaskSend()
{
	char Data[DEFAULT_BUFLEN];

	while (true)
	{
		auto start = std::chrono::steady_clock::now();
		if (pNetwork && pInputs && NetworkTime >= 0.02f)
		{
			// Read from Pending GameInput
			if (std::optional<char> PendingChar = pInputs->GetPendingGameInputToSend())
			{
				char key[5];
				GetNetStringToken(key, ENET_INPUT_CHANNEL);
				key[3] = *PendingChar;
				key[4] = '\0';
				const char* source = key;

				strcpy_s(Data, sizeof(source), source);
				
				int ret = pNetwork->Send(Data);
				if (ret > 0)
				{
					pInputs->UpdatePendingSendGameInputQueue();
				}
			}

			NetworkTime = 0;
		}
		auto finish = std::chrono::steady_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		NetworkTime += elapsed.count();
	}
}

void NetworkManager::TaskReceive()
{
	char Data[DEFAULT_BUFLEN];

	while (true)
	{
		auto start = std::chrono::steady_clock::now();
		if (pNetwork && NetworkTime >= 0.00000002f)
		{
			int ret = pNetwork->Receive(Data);
			if (ret > 0)
			{
				// INPUT CHANNEL
				if (Data[1] == '0')
				{
					if (pInputs)
					{
						pInputs->ReceiveRemoteGameInput(Data[3]);
					}
				}
				else if (Data[1] == '1')
				{
					if (pInputs)
					{
						char BulletCoord[16]; // Copy to this one as the Data is mod
						const char* coordData = BulletCoord;
						strcpy_s(BulletCoord, 16, Data);
						pInputs->ReceiveRemoteCoordinate(BulletCoord);
					}
						
				}
				else if (Data[1] == '2')
				{
					if (pInputs)
					{
						pInputs->bHasWinner = true;
						pInputs->bLoseFlag = true;
					}
						
				}
			}

			NetworkTime = 0;
		}
		auto finish = std::chrono::steady_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		NetworkTime += elapsed.count();
	}
}

void NetworkManager::Send(const char * Context, ENetChannel ID)
{
	assert(ID != 0); // Input handled by Threading Tick already, sending explicitly is not allowed.
	
	/// FIXME : Inifite sending.
	if (pNetwork)
	{
		char Data[DEFAULT_BUFLEN];
		char Token[4];
		GetNetStringToken(Token ,ID);
		strcpy_s(Data, 4, Token); // Copy {token} first, then append context.

		const char* ptr = Data;
		strcpy_s(Data + 3, sizeof(Context), Context);

		pNetwork->Send(Data);
	}
		
}

void NetworkManager::GetNetStringToken(char* Destination, ENetChannel NetChannel)
{
	if (Destination == nullptr)
		return;

	std::stringstream ss;
	ss << (int)NetChannel;
	std::string token = "{" + ss.str() + "}";

	strcpy_s(Destination, 4 , token.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////

int NetworkCommon::Initialize()
{
	// Initialize Winsock
	WSADATA wsaData;
	
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) 
	{
		printf("WSAStartup failed with error: %d\n", ret);
		return RESULT_ERROR;
	}

	return RESULT_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////////////

int NetworkServer::Initialize()
{
	if (NetworkCommon::Initialize() != 0)
		return RESULT_ERROR;

	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int ret = getaddrinfo(NULL, DEFAULT_PORT, &hints, &AddressResult);

	if (ret != 0) 
	{
		printf("[Server] getaddrinfo failed with error: %d\n", ret);
		WSACleanup();
		return RESULT_ERROR;
	}

	return RESULT_SUCCEED;
}

int NetworkServer::Setup()
{
	int ret = CreateListenSocketAndAcceptClient();
	if (ret != RESULT_SUCCEED)
		return RESULT_ERROR;

	return RESULT_SUCCEED;
}

int NetworkServer::Send(const char* Context)
{
	// Why are you sending empty buffer?
	if (Context == nullptr)
		return RESULT_ERROR;

	int ret = send(ClientSocket, Context, (int)(strlen(Context) + 1), 0);
	if (ret == SOCKET_ERROR)
		printf("[Server] Send failed with error: %d\n", WSAGetLastError());
	
	return ret;
}

int NetworkServer::Receive(char* Context)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int ret = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (ret > 0)
	{
		strcpy_s(Context, sizeof(recvbuf) , recvbuf);
	}
	else if (ret == 0)
	{
		// Nothing coming in
	}
	else
	{
		// Failure
		printf("[Server] Receive failed with error: %d\n", WSAGetLastError());
	}

	return ret;
}

int NetworkServer::Shutdown()
{
	int ret = shutdown(ClientSocket, SD_SEND);
	if (ret == SOCKET_ERROR)
		printf("[Server] Shutdown failed with error: %d\n", WSAGetLastError());
	
	closesocket(ClientSocket);
	WSACleanup();

	return (ret == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
}

int NetworkServer::CreateListenSocketAndAcceptClient()
{
	// Create a SOCKET for connecting to server
	ListenSocket = socket(AddressResult->ai_family, AddressResult->ai_socktype, AddressResult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) 
	{
		printf("[Server] Socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(AddressResult);
		WSACleanup();
		return RESULT_ERROR;
	}

	// Setup the TCP listening socket
	int ret = bind(ListenSocket, AddressResult->ai_addr, (int)AddressResult->ai_addrlen);
	if (ret == SOCKET_ERROR) 
	{
		printf("[Server] Bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(AddressResult);
		closesocket(ListenSocket);
		WSACleanup();
		return RESULT_ERROR;
	}

	freeaddrinfo(AddressResult);

	ret = listen(ListenSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		printf("[Server] Listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return RESULT_ERROR;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) 
	{
		printf("[Server] Accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return RESULT_ERROR;
	}

	// No longer need server socket
	closesocket(ListenSocket);


	return RESULT_SUCCEED;
}

//////////////////////////////////////////////////////////////////////////////////////

int NetworkClient::Initialize()
{
	NetworkCommon::Initialize();

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int ret = getaddrinfo(Target, DEFAULT_PORT, &hints, &AddressResult);
	if (ret != 0)
	{
		printf("[Client] getaddrinfo failed with error: %d\n", ret);
		WSACleanup();
		return RESULT_ERROR;
	}

	return RESULT_SUCCEED;
}

int NetworkClient::Setup()
{
	int ret = CreateSocketAndConnect();
	if (ret != RESULT_SUCCEED)
		return RESULT_ERROR;

	return RESULT_SUCCEED;
}

int NetworkClient::Send(const char* Context)
{
	if (Context == nullptr)
		return RESULT_ERROR;

	//Send one more character to allow termnating character to be sent.
	int ret = send(ConnectSocket, Context, (int)(strlen(Context)+1), 0);
	if (ret == SOCKET_ERROR)
		printf("[Client] Send failed with error: %d\n", WSAGetLastError());

	return ret;
}

int NetworkClient::Receive(char* Context)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int ret = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (ret > 0)
	{
		strcpy_s(Context, sizeof(recvbuf), recvbuf);
	}
	else if (ret == 0)
	{
		// Nothing coming in
	}
	else
	{
		printf("[Client] Receive failed with error: %d\n", WSAGetLastError());
	}

	return ret;
}

int NetworkClient::Shutdown()
{
	int ret = shutdown(ConnectSocket, SD_SEND);
	if (ret == SOCKET_ERROR)
		printf("[Client] Shutdown failed with error: %d\n", WSAGetLastError());
	
	closesocket(ConnectSocket);
	WSACleanup();

	return (ret == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
}

int NetworkClient::CreateSocketAndConnect()
{
	// Attempt to connect to an address until one succeeds
	for (auto* ptr = AddressResult; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			printf("[Client] Socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return RESULT_ERROR;
		}

		// Connect to server.
		int ret = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (ret == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(AddressResult);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("[Client] Unable to connect to server!\n");
		WSACleanup();
		return RESULT_ERROR;
	}

	return RESULT_SUCCEED;
}
