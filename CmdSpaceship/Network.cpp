#include "Network.h"

// Standard library includes
#include <string>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <thread>

// Project includes
#include "Inputs.h"

NetworkManager::NetworkManager()
{
    SERVER_ONLY(m_Network = std::make_unique<NetworkServer>())
    CLIENT_ONLY(m_Network = std::make_unique<NetworkClient>())
    m_LastSendTime = std::chrono::steady_clock::now();
    m_LastReceiveTime = std::chrono::steady_clock::now();
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
    if (bInitialized)
    {
#if DEBUG_LOG_FILE
        SERVER_ONLY(netSendLog.open("NetSendLogServer.txt"))
        CLIENT_ONLY(netSendLog.open("NetSendLogClient.txt"))
#endif // DEBUG_LOG_FILE

        while (!bShouldShutdown)
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_LastSendTime);
            
            if (deltaTime >= NETWORK_UPDATE_INTERVAL)
            {
                // Read from Pending GameInput
                if (auto pendingChar = m_InputManager->GetPendingGameInputToSend())
                {
                    std::string key = GetNetStringToken(INPUT_CHANNEL);
                    key += *pendingChar;

                    int result = m_Network->Send(key);
#if DEBUG_LOG_FILE
                    netSendLog.write(key.c_str(), key.length());
#endif // DEBUG_LOG_FILE
                    if (result > 0)
                    {
                        m_InputManager->UpdatePendingSendGameInputQueue();
                    }
                }
                m_LastSendTime = currentTime;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}

void NetworkManager::TaskReceive()
{
    std::string data;
    data.reserve(DEFAULT_BUFLEN);

    if (bInitialized)
    {
#if DEBUG_LOG_FILE
        SERVER_ONLY(netRecvLog.open("NetRecvLogServer.txt"))
        CLIENT_ONLY(netRecvLog.open("NetRecvLogClient.txt"))
#endif//DEBUG_LOG_FILE

        while (!bShouldShutdown)
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_LastReceiveTime);
            
            if (deltaTime >= NETWORK_UPDATE_INTERVAL)
            {
                int result = m_Network->Receive(data);
                if (result > 0)
                {
#if DEBUG_LOG_FILE
                    netRecvLog.write(data.c_str(), data.length());
#endif //DEBUG_LOG_FILE
                    // INPUT CHANNEL
                    if (data[1] == '0')
                    {
                        m_InputManager->ReceiveRemoteGameInput(data[3]);
                    }
                    else if (data[1] == '1')
                    {
                        m_InputManager->ReceiveRemoteCoordinate(data);
                    }
                    else if (data[1] == '2') // Receive a win packet
                    {
                        m_InputManager->bHasWinner = true;
                        m_InputManager->bLoseFlag = true;
                        if (m_OnWinMessageReceived)
                        {
                            m_OnWinMessageReceived();
                        }
                    }
                    else if (data[1] == '3') // Receive a reset packet
                    {
                        m_InputManager->SetRemoteReadyToReset();
                    }
                }
                m_LastReceiveTime = currentTime;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}

void NetworkManager::Send(std::string_view context, ENetChannel ID)
{
    assert(ID != 0); // Input handled by Threading Tick already, sending explicitly is not allowed.

    std::string data = GetNetStringToken(ID);
    data += context;
    m_Network->Send(data);
}

std::string NetworkManager::GetNetStringToken(ENetChannel netChannel)
{
    std::stringstream ss;
    ss << "{" << static_cast<int>(netChannel) << "}";
    return ss.str();
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

int NetworkServer::Send(std::string_view context)
{
    if (context.empty())
    {
        return RESULT_ERROR;
    }

    int result = send(m_ClientSocket, context.data(), static_cast<int>(context.length() + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Server] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkServer::Receive(std::string& context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int result = recv(m_ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
    if (result > 0)
    {
        context.assign(recvbuf, result);
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
    int result = bind(m_ListenSocket, m_AddressResult->ai_addr, static_cast<int>(m_AddressResult->ai_addrlen));
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

    int result = getaddrinfo(m_Target.c_str(), DEFAULT_PORT, &hints, &m_AddressResult);
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

int NetworkClient::Send(std::string_view context)
{
    if (context.empty())
    {
        return RESULT_ERROR;
    }

    int result = send(m_ConnectSocket, context.data(), static_cast<int>(context.length() + 1), 0);
    if (result == SOCKET_ERROR)
    {
        NET_LOG("[Client] Send failed with error: %d\n", WSAGetLastError());
    }

    return result;
}

int NetworkClient::Receive(std::string& context)
{
    char recvbuf[DEFAULT_BUFLEN];
    int result = recv(m_ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
    if (result > 0)
    {
        context.assign(recvbuf, result);
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
        int result = connect(m_ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
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
