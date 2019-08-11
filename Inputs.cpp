#include <iostream>

#include "Inputs.h"

static std::mutex InputMutex;

InputManager::InputManager()
{
}


InputManager::~InputManager()
{
}

void InputManager::ListenUserInput()
{
	char keypress;
	while (true)
	{
		if (!bChatMode)
		{
			std::cin.get(keypress);
			InputMutex.lock();
			PendingGameInput.push(keypress);
			InputMutex.unlock();
		}
	}
}

void InputManager::ListenUserInputLines()
{
	//std::string UserInput;
	while (true)
	{
		if (bChatMode)
		{
			//std::cin >> UserInput;
		}
	}
}

std::optional<char> InputManager::GetPendingGameInput()
{ 
	InputMutex.lock();
	char result;
	if (PendingGameInput.size() > 0) 
		result = PendingGameInput.front();
	InputMutex.unlock();

	if (PendingGameInput.size() > 0)
		return result;

	return {};

}

void InputManager::UpdateGameInputQueue()
{
	InputMutex.lock();
	if (PendingGameInput.size() > 0)
		PendingGameInput.pop();
	InputMutex.unlock();
}
