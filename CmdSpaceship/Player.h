#pragma once
#include <string>

class Player
{
public:
	Player();
	~Player();

	void SetPlayerName(std::string Name);
	std::string GetLocalPlayerName();
	std::string RequestRemotePlayerName();

private:
	std::string PlayerName = "default";
};

