
#include "Bullet.h"
#include "Spaceship.h"

Bullet::Bullet()
{
	CurrentLocation = FLocation2D(-1, -1);
	ForwardDirection = EDR_Unknown;
	bIsActive = false;
}

Bullet::Bullet(FLocation2D StartLocation, void* Instigator)
{
	CurrentLocation = StartLocation;
	Ownership = (Spaceship*)Instigator;
}

Bullet::~Bullet()
{
}

void Bullet::Initialize()
{
	// Reverting Projectile Data.
	CurrentLocation = FLocation2D(-1, -1);
	ForwardDirection = EDR_Unknown;
}

void Bullet::Activate(FLocation2D WakeLocation, void* Instigator)
{
	CurrentLocation = WakeLocation;
	Ownership = (Spaceship*)Instigator;
	bIsActive = true;
}

void Bullet::Sleep()
{
	bIsActive = false;
}

bool Bullet::IsActive()
{
	return bIsActive;
}
