#pragma once
#include <queue>
#include <optional>
#include <mutex>

class InputManager
{
public:
	InputManager();
	~InputManager();

	void ListenUserInput();
	void ListenUserInputLines();

	std::optional<char> GetPendingGameInput();
	void UpdateGameInputQueue();

private: 
	
	bool bChatMode = false;
	std::queue<char>		PendingGameInput;
	std::queue<const char*> PendingChatInput;
};

