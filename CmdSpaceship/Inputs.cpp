#include <mutex>
#include "Inputs.h"

static std::mutex InputMutex;

std::optional<char> InputManager::GetPendingGameInputToSend()
{
    char result;
    InputMutex.lock();
    size_t size = m_PendingGameInputToSend.size();
    if (size > 0)
    {
        result = m_PendingGameInputToSend.front();
    }

    InputMutex.unlock();

    if (size > 0)
    {
        return result;
    }

    return {};
}

void InputManager::ReceiveRemoteGameInput(char key)
{
    m_PendingRemoteGameInput.push(key);
}

void InputManager::ReceiveLocalGameInput(char key)
{
    m_PendingLocalGameInput.push(key); // for local.

    InputMutex.lock();
    m_PendingGameInputToSend.push(key); // for remote.
    InputMutex.unlock();
}

std::optional<char> InputManager::GetLocalPendingInput()
{
    // Local no need muteex
    char result;
    if (m_PendingLocalGameInput.size() > 0)
    {
        result = m_PendingLocalGameInput.front();
        return result;
    }
    return {};
}


std::optional<char> InputManager::GetRemotePendingInput()
{
    char result;
    if (m_PendingRemoteGameInput.size() > 0)
    {
        result = m_PendingRemoteGameInput.front();
        return result;
    }
    return {};
}

void InputManager::UpdateLocalInputQueue()
{
    m_PendingLocalGameInput.pop();
}

void InputManager::UpdateRemoteInputQueue()
{
    m_PendingRemoteGameInput.pop();
}

void InputManager::ReceiveRemoteCoordinate(std::string_view data)
{
    // Remove token!
    // Make into Location!
    m_coordinateBuffer.push(std::string(data));
}

std::optional<std::string_view> InputManager::GetCoordBuffer()
{
    if (!m_coordinateBuffer.empty())
    {
        return std::string_view(m_coordinateBuffer.front());
    }
    return std::nullopt;
}

void InputManager::UpdateCoordBufferQueue()
{
    if (!m_coordinateBuffer.empty())
    {
        m_coordinateBuffer.pop();
    }
}

void InputManager::UpdatePendingSendGameInputQueue()
{
    InputMutex.lock();
    m_PendingGameInputToSend.pop();
    InputMutex.unlock();
}
