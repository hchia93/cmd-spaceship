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
    SERVER_ONLY(m_Network = std::make_unique<NetworkServer>())
    CLIENT_ONLY(m_Network = std::make_unique<NetworkClient>())
}

NetworkManager::~NetworkManager()
{
    if (m_Network)
    {
        m_Network->Shutdown();
    }
  
#if DEBUG_LOG_FILE
    netSendLog.close();
    netRecvLog.close();
#endif // DEBUG_LOG_FILE
}

void NetworkManager::SetInputManager(InputManager* Manager)
{
    m_InputManager = Manager;
}

void NetworkManager::Initialize()
{
    if (m_Network)
    {
        int result = m_Network->Initialize();
        if (result != RESULT_SUCCEED)
        {
            return;
        }

        result = m_Network->Setup();
        if (result != RESULT_SUCCEED)
        {
            return;
        }
        
        if (m_InputManager == nullptr)
        {
            return;
        }

        bInitialized = true;
    }
}

void NetworkManager::TaskSend()
{
    char data[DEFAULT_BUFLEN];

    if (bInitialized)
    {

#if DEBUG_LOG_FILE
        SERVER_ONLY(netSendLog.open("NetSendLogServer.txt"))
        CLIENT_ONLY(netSendLog.open("NetSendLogClient.txt"))
#endif // DEBUG_LOG_FILE

        while (true)
        {
            // Read from Pending GameInput
            if (std::optional<char> PendingChar = m_InputManager->GetPendingGameInputToSend())
            {
                char key[5];
                GetNetStringToken(key, ENET_INPUT_CHANNEL);
                key[3] = *PendingChar;
                key[4] = '\0';
                const char* source = key;

                strcpy_s(data, sizeof(source), source);

                int ret = m_Network->Send(data);
#if DEBUG_LOG_FILE
                netSendLog.write(data, 32);
#endif // DEBUG_LOG_FILE
                if (ret > 0)
                {
                    m_InputManager->UpdatePendingSendGameInputQueue();
                }
            }
        }
    }
}

void NetworkManager::TaskReceive()
{
    char data[DEFAULT_BUFLEN];

    if (bInitialized)
    {
#if DEBUG_LOG_FILE
        SERVER_ONLY(netRecvLog.open("NetRecvLogServer.txt"))
        CLIENT_ONLY(netRecvLog.open("NetRecvLogClient.txt"))
#endif//DEBUG_LOG_FILE

        while (true)
        {
            int result = m_Network->Receive(data);
            if (result > 0)
            {
#if DEBUG_LOG_FILE
                netRecvLog.write(data, 32);
#endif //DEBUG_LOG_FILE
                // INPUT CHANNEL
                if (data[1] == '0')
                {
                    m_InputManager->ReceiveRemoteGameInput(data[3]);
                }
                else if (data[1] == '1')
                {
                    char bulletCoord[16]; // Copy to this one as the Data is mod
                    const char* coordData = bulletCoord;
                    strcpy_s(bulletCoord, 16, data);
                    m_InputManager->ReceiveRemoteCoordinate(bulletCoord);
                }
                else if (data[1] == '2') // Receive a win packet
                {
                    m_InputManager->bHasWinner = true;
                    m_InputManager->bLoseFlag = true;
                }
            }
        }
    }
}

void NetworkManager::Send(const char* context, ENetChannel ID)
{
    assert(ID != 0); // Input handled by Threading Tick already, sending explicitly is not allowed.

    char data[DEFAULT_BUFLEN];
    char token[4];
    GetNetStringToken(token, ID);

    strcpy_s(data, 4, token); // Copy {token} first, then append context.
    const char* ptr = data;
    strcpy_s(data + 3, sizeof(context), context);

    m_Network->Send(data);
}

void NetworkManager::GetNetStringToken(char* destination, ENetChannel netChannel)
{
    if (destination == nullptr)
    {
        return;
    }

    std::stringstream ss;
    ss << (int)netChannel;
    std::string token = "{" + ss.str() + "}";

    strcpy_s(destination, 4, token.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////

int NetworkCommon::Initialize()
{
    // Initialize Winsock
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        NET_LOG("WSAStartup failed with error: %d\n", result);
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

    int result = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &m_AddressResult);
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

int NetworkServer::Send(const char* context)
{
    // Why are you sending empty buffer?
    if (context == nullptr)
    {
        return RESULT_ERROR;
    }

    int result = send(m_ClientSocket, context, (int)(strlen(context) + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkServer::Receive(char* context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int result = recv(m_ClientSocket, recvbuf, recvbuflen, 0);
    if (result > 0)
    {
        strcpy_s(context, sizeof(recvbuf), recvbuf);
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
    int result = shutdown(m_ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Shutdown failed with error: %d\n", WSAGetLastError());
    }

    closesocket(m_ClientSocket);
    WSACleanup();

    return (result == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
}

int NetworkServer::CreateListenSocketAndAcceptClient()
{
    // Create a SOCKET for connecting to server
    m_ListenSocket = socket(m_AddressResult->ai_family, m_AddressResult->ai_socktype, m_AddressResult->ai_protocol);
    if (m_ListenSocket == INVALID_SOCKET)
    {
        NET_LOG("[Server] Socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(m_AddressResult);
        WSACleanup();
        return RESULT_ERROR;
    }

    // Setup the TCP listening socket
    int result = bind(m_ListenSocket, m_AddressResult->ai_addr, (int)m_AddressResult->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(m_AddressResult);
        closesocket(m_ListenSocket);
        WSACleanup();
        return RESULT_ERROR;
    }

    freeaddrinfo(m_AddressResult);

    result = listen(m_ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_ListenSocket);
        WSACleanup();
        return RESULT_ERROR;
    }

    // Accept a client socket
    m_ClientSocket = accept(m_ListenSocket, nullptr, nullptr);
    if (m_ClientSocket == INVALID_SOCKET)
    {
        NET_LOG("[Server] Accept failed with error: %d\n", WSAGetLastError());
        closesocket(m_ListenSocket);
        WSACleanup();
        return RESULT_ERROR;
    }

    // No longer need server socket
    closesocket(m_ListenSocket);


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

    int result = getaddrinfo(m_Target, DEFAULT_PORT, &hints, &m_AddressResult);
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

int NetworkClient::Send(const char* context)
{
    if (context == nullptr)
    {
        return RESULT_ERROR;
    }

    //Send one more character to allow termnating character to be sent.
    int result = send(m_ConnectSocket, context, (int)(strlen(context) + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Client] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkClient::Receive(char* context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int result = recv(m_ConnectSocket, recvbuf, recvbuflen, 0);
    if (result > 0)
    {
        strcpy_s(context, sizeof(recvbuf), recvbuf);
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
    int result = shutdown(m_ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Client] Shutdown failed with error: %d\n", WSAGetLastError());
    }

    closesocket(m_ConnectSocket);
    WSACleanup();

    return (result == SOCKET_ERROR) ? RESULT_ERROR : RESULT_SUCCEED;
}

int NetworkClient::CreateSocketAndConnect()
{
    // Attempt to connect to an address until one succeeds
    for (auto* ptr = m_AddressResult; ptr != nullptr; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        m_ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (m_ConnectSocket == INVALID_SOCKET)
        {
            NET_LOG("[Client] Socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return RESULT_ERROR;
        }

        // Connect to server.
        int result = connect(m_ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            closesocket(m_ConnectSocket);
            m_ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(m_AddressResult);

    if (m_ConnectSocket == INVALID_SOCKET)
    {
        NET_LOG("[Client] Unable to connect to server!\n");
        WSACleanup();
        return RESULT_ERROR;
    }

    return RESULT_SUCCEED;
}
