#include "Network.h"

// Standard library includes
#include <string>
#include <sstream>
#include <cassert>
#include <cstdio>

// Project includes
#include "Inputs.h"

NetworkManager::NetworkManager()
{
    SERVER_ONLY(pNetwork = std::make_unique<NetworkServer>())
    CLIENT_ONLY(pNetwork = std::make_unique<NetworkClient>())
}

NetworkManager::~NetworkManager()
{
    if (pNetwork)
    {
        pNetwork->Shutdown();
    }
  
#if DEBUG_LOG_FILE
    netSendLog.close();
    netRecvLog.close();
#endif // DEBUG_LOG_FILE
}

void NetworkManager::SetInputManager(InputManager* Manager)
{
    pInputManager = Manager;
}

void NetworkManager::Initialize()
{
    if (pNetwork)
    {
        int result = pNetwork->Initialize();
        if (result != RESULT_SUCCEED)
        {
            return;
        }

        result = pNetwork->Setup();
        if (result != RESULT_SUCCEED)
        {
            return;
        }
        
        if (pInputManager == nullptr)
        {
            return;
        }

        bInitialized = true;
    }
}

void NetworkManager::TaskSend()
{
    char Data[DEFAULT_BUFLEN];

    if (bInitialized)
    {

#if DEBUG_LOG_FILE
        SERVER_ONLY(netSendLog.open("NetSendLogServer.txt"))
        CLIENT_ONLY(netSendLog.open("NetSendLogClient.txt"))
#endif // DEBUG_LOG_FILE

        while (true)
        {
            // Read from Pending GameInput
            if (std::optional<char> PendingChar = pInputManager->GetPendingGameInputToSend())
            {
                char key[5];
                GetNetStringToken(key, ENET_INPUT_CHANNEL);
                key[3] = *PendingChar;
                key[4] = '\0';
                const char* source = key;

                strcpy_s(Data, sizeof(source), source);

                int ret = pNetwork->Send(Data);
#if DEBUG_LOG_FILE
                netSendLog.write(Data, 32);
#endif // DEBUG_LOG_FILE
                if (ret > 0)
                {
                    pInputManager->UpdatePendingSendGameInputQueue();
                }
            }
        }
    }
}

void NetworkManager::TaskReceive()
{
    char Data[DEFAULT_BUFLEN];

    if (bInitialized)
    {
#if DEBUG_LOG_FILE
        SERVER_ONLY(netRecvLog.open("NetRecvLogServer.txt"))
        CLIENT_ONLY(netRecvLog.open("NetRecvLogClient.txt"))
#endif//DEBUG_LOG_FILE

        while (true)
        {
            int result = pNetwork->Receive(Data);
            if (result > 0)
            {
#if DEBUG_LOG_FILE
                netRecvLog.write(Data, 32);
#endif //DEBUG_LOG_FILE
                // INPUT CHANNEL
                if (Data[1] == '0')
                {
                    pInputManager->ReceiveRemoteGameInput(Data[3]);
                }
                else if (Data[1] == '1')
                {
                    char BulletCoord[16]; // Copy to this one as the Data is mod
                    const char* coordData = BulletCoord;
                    strcpy_s(BulletCoord, 16, Data);
                    pInputManager->ReceiveRemoteCoordinate(BulletCoord);
                }
                else if (Data[1] == '2') // Receive a win packet
                {
                    pInputManager->bHasWinner = true;
                    pInputManager->bLoseFlag = true;
                }
            }
        }
    }
}

void NetworkManager::Send(const char* Context, ENetChannel ID)
{
    assert(ID != 0); // Input handled by Threading Tick already, sending explicitly is not allowed.

    char Data[DEFAULT_BUFLEN];
    char Token[4];
    GetNetStringToken(Token, ID);

    strcpy_s(Data, 4, Token); // Copy {token} first, then append context.
    const char* ptr = Data;
    strcpy_s(Data + 3, sizeof(Context), Context);

    pNetwork->Send(Data);
}

void NetworkManager::GetNetStringToken(char* Destination, ENetChannel NetChannel)
{
    if (Destination == nullptr)
        return;

    std::stringstream ss;
    ss << (int)NetChannel;
    std::string token = "{" + ss.str() + "}";

    strcpy_s(Destination, 4, token.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////

int NetworkCommon::Initialize()
{
    // Initialize Winsock
    WSADATA wsaData;

    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        NET_LOG("WSAStartup failed with error: %d\n", ret);
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////////////

int NetworkServer::Initialize()
{
    if (NetworkCommon::Initialize() != 0)
    {
        return RESULT_ERROR;
    }
       

    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &AddressResult);
    if (result != 0)
    {
        NET_LOG("[Server] getaddrinfo failed with error: %d\n", ret);
        WSACleanup();
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}

int NetworkServer::Setup()
{
    int result = CreateListenSocketAndAcceptClient();
    if (result != RESULT_SUCCEED)
    {
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}

int NetworkServer::Send(const char* Context)
{
    // Why are you sending empty buffer?
    if (Context == nullptr)
    {
        return RESULT_ERROR;
    }

    int result = send(ClientSocket, Context, (int)(strlen(Context) + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkServer::Receive(char* Context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int result = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (result > 0)
    {
        strcpy_s(Context, sizeof(recvbuf), recvbuf);
    }
    else if (result == 0)
    {
        // Nothing coming in
    }
    else
    {
        NET_LOG("[Server] Receive failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkServer::Shutdown()
{
    int result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Shutdown failed with error: %d\n", WSAGetLastError());
    }

    closesocket(ClientSocket);
    WSACleanup();

    return (result == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
}

int NetworkServer::CreateListenSocketAndAcceptClient()
{
    // Create a SOCKET for connecting to server
    ListenSocket = socket(AddressResult->ai_family, AddressResult->ai_socktype, AddressResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        NET_LOG("[Server] Socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(AddressResult);
        WSACleanup();
        return RESULT_ERROR;
    }

    // Setup the TCP listening socket
    int result = bind(ListenSocket, AddressResult->ai_addr, (int)AddressResult->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(AddressResult);
        closesocket(ListenSocket);
        WSACleanup();
        return RESULT_ERROR;
    }

    freeaddrinfo(AddressResult);

    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return RESULT_ERROR;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        NET_LOG("[Server] Accept failed with error: %d\n", WSAGetLastError());
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

    int result = getaddrinfo(Target, DEFAULT_PORT, &hints, &AddressResult);
    if (result != 0)
    {
        NET_LOG("[Client] getaddrinfo failed with error: %d\n", ret);
        WSACleanup();
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}

int NetworkClient::Setup()
{
    int result = CreateSocketAndConnect();
    if (result != RESULT_SUCCEED)
    {
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}

int NetworkClient::Send(const char* Context)
{
    if (Context == nullptr)
    {
        return RESULT_ERROR;
    }

    //Send one more character to allow termnating character to be sent.
    int result = send(ConnectSocket, Context, (int)(strlen(Context) + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Client] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkClient::Receive(char* Context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int result = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (result > 0)
    {
        strcpy_s(Context, sizeof(recvbuf), recvbuf);
    }
    else if (result == 0)
    {
        // Nothing coming in
    }
    else
    {
        NET_LOG("[Client] Receive failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkClient::Shutdown()
{
    int result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Client] Shutdown failed with error: %d\n", WSAGetLastError());
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return (result == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
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
            NET_LOG("[Client] Socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return RESULT_ERROR;
        }

        // Connect to server.
        int result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
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
        NET_LOG("[Client] Unable to connect to server!\n");
        WSACleanup();
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}
