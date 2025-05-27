#include "Spaceship.h"
#include "Bullet.h"

Spaceship::Spaceship(SpaceShipSpawnParam Param)
{
    m_NetRole = Param.NetRole;
    m_Location = Param.SpawnLocation;
    m_RequestBulletDelegate = Param.SpawnFunction;
}

Bullet* Spaceship::Shoot()
{
    if (m_RequestBulletDelegate)
    {
        if (Bullet* pBullet = m_RequestBulletDelegate())
        {
            BulletSpawnParam param;
            param.Instigator = this;

            pBullet->Activate(param);
        }
    }
    return nullptr;
}

void Spaceship::MoveLeft()
{
    if (m_NetRole == ENetRole::LOCAL)
    {
        m_Location.X -= 1;
    }
    else
    {
        m_Location.X += 1;
    }
}

void Spaceship::MoveRight()
{
    if (m_NetRole == ENetRole::LOCAL)
    {
        m_Location.X += 1;
    }
    else
    {
        m_Location.X -= 1;
    }
}

