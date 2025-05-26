#include "Player.h"

Player::Player()
{
#if SERVER_BUILD
	PlayerName = "Server";
#else CLIENT_BUILD
	PlayerName = "Client";
#endif //SERVER_BUILD
}

Player::~Player()
{
}

std::string Player::GetLocalPlayerName()
{
	return PlayerName;
}

std::string Player::RequestRemotePlayerName()
{
	// Receive via dispatcher, and broadcast on dispatcher?
	return std::string();
}

void Player::SetPlayerName(std::string Name)
{
	PlayerName = Name;
}