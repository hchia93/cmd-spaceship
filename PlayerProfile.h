#pragma once
#include <string>

class PlayerProfile
{
public:
	PlayerProfile();
	~PlayerProfile();


	void SetPlayerName(std::string Name);
	std::string GetLocalPlayerName();
	std::string RequestRemotePlayerName();
	void Win();
	void Lose();

private:
	int WinStreak			= 0;
	int WinCount			= 0;
	int LoseCount			= 0 ;
	bool bWasLastWin		= false;
	std::string PlayerName  = "default";
};

