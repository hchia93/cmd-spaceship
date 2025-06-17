#include "Bullet.h"
#include "Spaceship.h"

void Bullet::Activate(BulletSpawnParam param)
{
    if (param.instigator != nullptr)
    {
        m_Location = param.instigator->GetLocation();
        const bool isLocal = param.instigator->GetNetRole() == ENetRole::LOCAL;

        // Resolved Nozzle Location
        if (isLocal)
        {
            m_Location.Y = param.instigator->GetLocation().Y - 1;
        }
        else
        {
            m_Location.Y = param.instigator->GetLocation().Y + 1;
        }

        m_Direction = isLocal ? EDirection::UP : EDirection::DOWN;
        m_Owner = param.instigator;
        bIsActive = true;
    }
}

void Bullet::Tick()
{
    switch (m_Direction)
    {
    case EDirection::UP:
    {
        if (m_Location.Y >= 0 && m_Location.Y <= SCREEN_Y_MAX - 1)
        {
            m_Location.Y -= 1;
        }
        else
        {
            Deactivate();
        }
        break;
    }
    case EDirection::DOWN:
    {
        if (m_Location.Y >= 0 && m_Location.Y <= SCREEN_Y_MAX - 1)
        {
            m_Location.Y += 1;
        }
        else
        {
            Deactivate();
        }
        break;
    }
    }
}

void Bullet::Deactivate()
{
    bIsActive = false;
}
