#pragma once
#include <vector>
class InputManager
{
public:
	InputManager();
	~InputManager();

	void ListenUserInput();

private: 
	
	std::vector<const char*> InputLines;
};

