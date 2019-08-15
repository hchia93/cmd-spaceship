#pragma once

#include <vector>
#include "Utils.h"

class Bullet;

class Spaceship
{
public:

	Bullet*										Shoot();

	FLocation2D									GetLocation() const { return CurrentLocation;  }
	void										SetLocation(FLocation2D Location) { CurrentLocation = Location; CLAMP(CurrentLocation); }
	static void									UpdatePool();
	static FHitResult							CheckHit(Spaceship* Othership);

	static bool									FindLocalBullet(int row, int col);
	static bool									FindRemoteBullet(int row, int col);
	static std::vector<Bullet*>					SharedPool;
	static void									KillPool();

private:
	FLocation2D									CurrentLocation;
	EDirection									ForwardDirection;
};

