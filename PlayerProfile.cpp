#include "PlayerProfile.h"



PlayerProfile::PlayerProfile()
{
#if SERVER
	PlayerName = "Server";
#elif CLIENT
	PlayerName = "Client";
#elif 
	PlayerName = "default";
#endif
}

PlayerProfile::~PlayerProfile()
{
}

std::string PlayerProfile::GetLocalPlayerName()
{
	return PlayerName;
}

std::string PlayerProfile::RequestRemotePlayerName()
{
	// Receive via dispatcher, and broadcast on dispatcher?
	return std::string();
}

void PlayerProfile::SetPlayerName(std::string Name)
{
	PlayerName = Name;
}

void PlayerProfile::Win()
{
	// Send Packet to both side?
}

void PlayerProfile::Lose()
{
	// Send Packet to both side?
}
