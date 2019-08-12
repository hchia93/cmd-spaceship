#include <chrono>
#include <mutex>
#include "Network.h"
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
	char buff[DEFAULT_BUFLEN];
	const char* Data = buff;

	while (true)
	{
		auto start = std::chrono::high_resolution_clock::now();
		if (pNetwork && pInputs && NetworkTime >= 0.02f)
		{
			// Read from Pending GameInput
			if (std::optional<char> PendingChar = pInputs->GetPendingGameInput())
			{
				char key[1];
				key[0] = *PendingChar;
				//key[1] = '\0';
				const char* source = key;

				strcpy_s(buff, sizeof(source), source);
				
				int ret = pNetwork->Send(Data);
				if (ret > 0)
				{
					pInputs->UpdateGameInputQueue();
				}
			}

			NetworkTime = 0;
		}
		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		NetworkTime += elapsed.count();
	}
}

void NetworkManager::TaskReceive()
{
	int ret;
	char recvbuf[DEFAULT_BUFLEN];

	while (true)
	{
		auto start = std::chrono::high_resolution_clock::now();
		if (pNetwork && NetworkTime >= 0.02f)
		{
			ret = pNetwork->Receive(recvbuf);
			if (ret > 0)
			{
				//Received.
				printf("%s", recvbuf);
			}

			NetworkTime = 0;
		}
		auto finish = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> elapsed = finish - start;
		NetworkTime += elapsed.count();
	}
}

void NetworkManager::Send(const char * Context, int ID)
{
	// This function is only used to send extra data with ID that needs to be handle.
	// Will conflict with the main send functions. 
	if (pNetwork)
		pNetwork->Send(Context);
}

void NetworkManager::Receive(const char* Context, int ID)
{
	// This function is only used to send extra data with ID that needs to be handle.
	// Will conflict with the main receive functions.
	if (pNetwork)
		pNetwork->Receive(Context);
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

	int ret = send(ClientSocket, Context, (int)strlen(Context), 0);
	if (ret == SOCKET_ERROR)
		printf("[Server] Send failed with error: %d\n", WSAGetLastError());
	
	return ret;
}

int NetworkServer::Receive(const char* Context)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int ret = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (ret > 0)
	{
		printf("%s", recvbuf);
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

	int ret = send(ConnectSocket, Context, (int)strlen(Context), 0);
	if (ret == SOCKET_ERROR)
		printf("[Client] Send failed with error: %d\n", WSAGetLastError());

	return ret;
}

int NetworkClient::Receive(const char* Context)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	int ret = recv(ConnectSocket, recvbuf, 1 /*recvbuflen*/, 0);
	if (ret > 0)
	{
		printf("%s", recvbuf);
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
