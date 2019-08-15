
#include "Inputs.h"
#include <mutex>

static std::mutex InputMutex;

InputManager::InputManager()
{
}


InputManager::~InputManager()
{
}

std::optional<char> InputManager::GetPendingGameInputToSend()
{ 
	char result;
	int size = 0; 
	InputMutex.lock();
	size = PendingGameInputToSend.size();
	if (size > 0)
		result = PendingGameInputToSend.front();
	InputMutex.unlock();

	if (size > 0)
		return result;

	return {};

}

void InputManager::ReceiveRemoteGameInput(char Key)
{
	PendingRemoteGameInput.push(Key);
}

void InputManager::ReceiveLocalGameInput(char Key)
{
	PendingLocalGameInput.push(Key); // for local.

	InputMutex.lock();
	PendingGameInputToSend.push(Key); // for remote.
	InputMutex.unlock();
}

std::optional<char> InputManager::GetLocalPendingInput()
{
	// Local no need muteex
	char result;
	if (PendingLocalGameInput.size() > 0)
	{
		result = PendingLocalGameInput.front();
		return result;
	}
	return {};
}


std::optional<char> InputManager::GetRemotePendingInput()
{
	char result;
	if (PendingRemoteGameInput.size() > 0)
	{
		result = PendingRemoteGameInput.front();
		return result;
	}
	return {};
}

void InputManager::UpdateLocalInputQueue()
{
	PendingLocalGameInput.pop();
}

void InputManager::UpdateRemoteInputQueue()
{
	PendingRemoteGameInput.pop();
}

void InputManager::ReceiveRemoteCoordinate(const char * Data)
{
	// Remove token!

	// Make into Location!
	CoordBuffer.push(Data);

}

std::optional<const char*> InputManager::GetCoordBuffer()
{
	if (CoordBuffer.size() > 0)
	{
		return CoordBuffer.front();
	}
	return {};
}

void InputManager::UpdateCoordBufferQueue()
{
	CoordBuffer.pop();
}

void InputManager::UpdatePendingSendGameInputQueue()
{
	InputMutex.lock();
	PendingGameInputToSend.pop();
	InputMutex.unlock();
}
