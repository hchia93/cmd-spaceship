#include <mutex>
#include "Inputs.h"

static std::mutex InputMutex;

InputManager::InputManager()
{
    ClearAllInputs();
}

void InputManager::ClearAllInputs()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    std::queue<char>().swap(m_PendingGameInputToSend);
    std::queue<char>().swap(m_PendingLocalGameInput);
    std::queue<char>().swap(m_PendingRemoteGameInput);
    std::queue<std::string>().swap(m_coordinateBuffer);
}

std::optional<char> InputManager::GetPendingGameInputToSend()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingGameInputToSend.empty())
    {
        char result = m_PendingGameInputToSend.front();
        return result;
    }
    return {};
}

void InputManager::ReceiveRemoteGameInput(char key)
{
    std::lock_guard<std::mutex> lock(InputMutex);
    m_PendingRemoteGameInput.push(key);
}

void InputManager::ReceiveLocalGameInput(char key)
{
    std::lock_guard<std::mutex> lock(InputMutex);
    m_PendingLocalGameInput.push(key); // for local.
    m_PendingGameInputToSend.push(key); // for remote.
}

std::optional<char> InputManager::GetLocalPendingInput()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingLocalGameInput.empty())
    {
        char result = m_PendingLocalGameInput.front();
        return result;
    }
    return {};
}

std::optional<char> InputManager::GetRemotePendingInput()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingRemoteGameInput.empty())
    {
        char result = m_PendingRemoteGameInput.front();
        return result;
    }
    return {};
}

void InputManager::UpdateLocalInputQueue()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingLocalGameInput.empty())
    {
        m_PendingLocalGameInput.pop();
    }
}

void InputManager::UpdateRemoteInputQueue()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingRemoteGameInput.empty())
    {
        m_PendingRemoteGameInput.pop();
    }
}

void InputManager::ReceiveRemoteCoordinate(std::string_view data)
{
    std::lock_guard<std::mutex> lock(InputMutex);
    m_coordinateBuffer.push(std::string(data));
}

std::optional<std::string_view> InputManager::GetCoordBuffer()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_coordinateBuffer.empty())
    {
        return std::string_view(m_coordinateBuffer.front());
    }
    return std::nullopt;
}

void InputManager::UpdateCoordBufferQueue()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_coordinateBuffer.empty())
    {
        m_coordinateBuffer.pop();
    }
}

void InputManager::UpdatePendingSendGameInputQueue()
{
    std::lock_guard<std::mutex> lock(InputMutex);
    if (!m_PendingGameInputToSend.empty())
    {
        m_PendingGameInputToSend.pop();
    }
}
