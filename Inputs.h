#pragma once
#include <queue>
#include <optional>

class InputManager
{
public:
	InputManager();
	~InputManager();

	// Helper function to send to network thread.
	std::optional<char> GetPendingGameInputToSend();
	void UpdatePendingSendGameInputQueue();

	// Helper function to send to game to handle (Local)
	void ReceiveRemoteGameInput(char Key); // push
	void ReceiveLocalGameInput(char Key); // push
	std::optional<char> GetLocalPendingInput(); // get
	std::optional<char> GetRemotePendingInput(); // get
	void UpdateLocalInputQueue(); // pop
	void UpdateRemoteInputQueue(); // pop

	// Diplay Bullet Fire Position from Another Person perspective
	void ReceiveRemoteCoordinate(const char* Data);
	std::optional<const char*> GetCoordBuffer();
	void UpdateCoordBufferQueue();
	
	bool bHasWinner = false;
	bool bLoseFlag = false;

private: 
	
	std::queue<char>		PendingGameInputToSend;
	std::queue<char>		PendingLocalGameInput;
	std::queue<char>		PendingRemoteGameInput;

	std::queue<const char*> CoordBuffer;
};

