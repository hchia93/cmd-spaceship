
#include "Projectile.h"


Projectile::Projectile()
{
}

Projectile::Projectile(FLocation2D StartLocation, EDirection Direction, void * Instigator)
{
}


Projectile::~Projectile()
{
}

void Projectile::Move()
{
}

void Projectile::Tick(float deltaTime)
{
}

void Projectile::Initialize()
{
	// Reverting Projectile Data.
	CurrentPosition = FLocation2D(-1, -1);
	ForwardDirection = EDR_Unknown;
}

void Projectile::Activate()
{
}

void Projectile::Sleep()
{
}

void Projectile::OnDispatchEvent(const void * Object, EGameEvent Event)
{
}

void Projectile::OnReceiveEvent(const void * Object, EGameEvent Event)
{
}
