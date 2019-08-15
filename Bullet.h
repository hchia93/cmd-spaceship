#pragma once
#include "Utils.h"
#include "Interface.h"

class Spaceship;

class Bullet : public IPoolable
{
public:
	explicit		Bullet();
	explicit		Bullet(FLocation2D StartLocation, void* Instigator);

	//IPoolable
	virtual void	Initialize() override;
	virtual void	Activate(FLocation2D WakeLocation , void* Instigator) override;
	virtual void	Sleep() override;
	virtual bool	IsActive() override;
	//~IPoolable

	FLocation2D		GetLocation() const { return CurrentLocation; }
	EDirection		GetForwardDirection() const { return ForwardDirection;  }
	void			SetLocation(FLocation2D Location) { CurrentLocation = Location;}
	void			SetForwardDirection(EDirection Direction) { ForwardDirection = Direction;  }

private: 
	FLocation2D		CurrentLocation;
	EDirection		ForwardDirection; 
	Spaceship*		Ownership;
	bool			bIsActive = false;
};

