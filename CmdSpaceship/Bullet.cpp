
#include "Bullet.h"
#include "Spaceship.h"

void Bullet::Activate(BulletSpawnParam Param)
{
    if (Param.Instigator != nullptr)
    {
        m_Location = Param.Instigator->GetLocation();
        bool IsLocal = Param.Instigator->GetNetRole() == ENetRole::LOCAL;

        // Resolved Location
        if (IsLocal)
        {
            m_Location.Y = Param.Instigator->GetLocation().Y - 1;
        }
        else
        {
            m_Location.Y = Param.Instigator->GetLocation().Y + 1;
        }

        m_Direction = IsLocal ? EDirection::Up : EDirection::Down;
        m_Owner = Param.Instigator;
        bIsActive = true;
    }
}

void Bullet::Tick()
{
    switch (m_Direction)
    {
    case EDirection::Up:
    {
        if (m_Location.X >= 0 && m_Location.X <= SCREEN_X_MAX - 1)
        {
            m_Location.X += 1;
        }
        else
        {
            Deactivate();
        }
        break;
    }
    case EDirection::Down:
    {
        if (m_Location.X >= 0 && m_Location.X <= SCREEN_X_MAX - 1)
        {
            m_Location.X -= 1;
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
