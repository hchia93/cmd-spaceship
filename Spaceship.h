#pragma once

#include <string>
#include "Interface.h"
#include "Utils.h"

class Spaceship : public ITickable, public IMovable, public IGameEventHandler
{
public:
	Spaceship();
	~Spaceship();

	//IMovable
	virtual void Move() override;
	//~IMovable

	//ITickable
	virtual void Tick(float deltaTime) override;
	//~ITickable

	//IGameEventHandler
	virtual void OnDispatchEvent(const void* Object, EGameEvent Event) override;
	virtual void OnReceiveEvent(const void* Object, EGameEvent Event) override;
	//IGameEventHandler

	void SetPlayerName(std::string Name) { PlayerName = Name; }
	void Shoot() {};

private:
	FLocation2D		CurrentLocation;
	EDirection      ForwardDirection;

	std::string		PlayerName;
};

