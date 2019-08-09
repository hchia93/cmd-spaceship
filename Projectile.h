#pragma once
#include "Utils.h"
#include "Interface.h"

class Projectile : public IPoolable, public ITickable, public IMovable, public IGameEventHandler
{
public:
	Projectile();
	Projectile(FLocation2D StartLocation, EDirection Direction, void* Instigator);
	~Projectile();

	//IMovable
	virtual void Move() override;
	//~IMovable

	//ITickable
	virtual void Tick(float deltaTime) override;
	//~ITickable

	//IPoolable
	virtual void Initialize() override;
	virtual void Activate() override;
	virtual void Sleep() override;
	//~IPoolable

	//IGameEventHandler
	virtual void OnDispatchEvent(const void* Object, EGameEvent Event) override;
	virtual void OnReceiveEvent(const void* Object, EGameEvent Event) override;
	//IGameEventHandler

private: 
	FLocation2D		CurrentPosition;
	EDirection		ForwardDirection; 
};

