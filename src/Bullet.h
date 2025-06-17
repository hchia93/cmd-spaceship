#pragma once
#include "Utils.h"

class Spaceship;

struct BulletSpawnParam
{
    Spaceship* instigator;
};

class Bullet
{
public:
    void Activate(BulletSpawnParam param);
    void Deactivate();
    const bool IsActive() { return bIsActive; }

    void Tick();

    FLocation2D GetLocation() const { return m_Location; }
    EDirection GetDirection() const { return m_Direction; }
    Spaceship* GetOwner() { return m_Owner; }

private:
    FLocation2D m_Location;
    EDirection  m_Direction;
    Spaceship* m_Owner;
    bool bIsActive = false;
};

