#include "Spaceship.h"
#include "Bullet.h"

Spaceship::Spaceship()
{
}


Spaceship::~Spaceship()
{
	
}

Bullet* Spaceship::Shoot()
{
	for (auto* p : SharedPool)
	{
		if (!p) continue;

		if (!p->IsActive())
		{
			p->Initialize();
			return p;
		}
	}

	Bullet* pBullet = new Bullet(GetLocation(), this);
	SharedPool.push_back(pBullet);
	return (Bullet*)SharedPool.back();
}

void Spaceship::UpdatePool()
{
	for (auto* p : SharedPool)
	{
		if (p)
		{
			if (p->GetForwardDirection() == EDR_Up)
			{
				if (p->GetLocation().Y >= 0 && p->GetLocation().Y <= SCREEN_Y_MAX)
				{
					p->SetLocation(p->GetLocation() + FLocation2D(0, 1));
				}
				else
				{
					p->Sleep();
				}
			}

			if (p->GetForwardDirection() == EDR_Down)
			{
				if (p->GetLocation().Y >= 0 && p->GetLocation().Y <= SCREEN_Y_MAX)
				{
					p->SetLocation(p->GetLocation() + FLocation2D(0, -1));
				}
				else
				{
					p->Sleep();
				}
			}
		}
	}
}

bool Spaceship::FindLocalBullet(int row, int col)
{
	for (auto* p : SharedPool)
	{
		if (p && p->GetForwardDirection() == EDR_Up)
		{
			if (FLocation2D::IsMatch(row, col, p->GetLocation()))
				return true;
		}
	}
	return false;
}

bool Spaceship::FindRemoteBullet(int row, int col)
{
	for (auto* p : SharedPool)
	{
		if (p && p->GetForwardDirection() == EDR_Down)
		{
			if (FLocation2D::IsMatch(row, col, p->GetLocation()))
				return true;
		}
	}
	return false;
}

void Spaceship::KillPool()
{
	for (auto* p : SharedPool)
		delete p;
}

std::vector<Bullet*> Spaceship::SharedPool = [] {
	std::vector<Bullet*> v;
	for(int i = 0 ; i < 10; ++i)
		v.push_back(new Bullet());
	return v;
}();