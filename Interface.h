#pragma once
#include "Utils.h"

class IMovable
{
public:
	virtual void Move() = 0;
};

class ITickable
{
public:
	virtual void Tick(float deltaTime) = 0;
};
class IPoolable
{
public:
	virtual void Initialize() = 0;
	virtual void Activate() = 0;
	virtual void Sleep() = 0;
};

class IGameEventHandler
{
public:
	virtual void OnDispatchEvent(const void* Object, EGameEvent Event) = 0;
	virtual void OnReceiveEvent(const void* Object, EGameEvent Event) = 0;
};

class IInputListener
{
	virtual void OnReceiveInput() = 0;
};