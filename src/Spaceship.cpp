#include "Spaceship.h"
#include "Bullet.h"

Spaceship::Spaceship(SpaceShipSpawnParam param)
{
    m_NetRole = param.netRole;
    m_Location = param.spawnLocation;
    m_RequestBulletDelegate = param.spawnBulletFunction;
}

Bullet* Spaceship::Shoot()
{
    if (m_RequestBulletDelegate)
    {
        if (Bullet* pBullet = m_RequestBulletDelegate())
        {
            BulletSpawnParam param;
            param.instigator = this;

            pBullet->Activate(param);
            return pBullet;
        }
    }
    return nullptr;
}

void Spaceship::MoveLeft()
{
    const bool isLocal = m_NetRole == ENetRole::LOCAL;
    if (isLocal)
    {
        m_Location.X -= 1;
        if (m_Location.X <= 2)
        {
            m_Location.X = 2;
        }

    }
    else
    {
        m_Location.X += 1;
        if (m_Location.X >= SCREEN_X_MAX - 3)
        {
            m_Location.X = SCREEN_X_MAX - 3;
        }
    }
}

void Spaceship::MoveRight()
{
    const bool isLocal = m_NetRole == ENetRole::LOCAL;

    if (isLocal)
    {
        m_Location.X += 1;
        if (m_Location.X >= SCREEN_X_MAX - 3)
        {
            m_Location.X = SCREEN_X_MAX - 3;
        }
    }
    else
    {
        m_Location.X -= 1;
        if (m_Location.X <= 2)
        {
            m_Location.X = 2;
        }
    }
}

