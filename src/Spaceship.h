#pragma once

#include <functional>

#include "Macro.h"
#include "Utils.h"
#include "BulletPoolService.h"

class Bullet;

struct SpaceShipSpawnParam
{
    std::function<Bullet* ()> spawnBulletFunction;
    FLocation2D spawnLocation;
    ENetRole netRole;
};

class Spaceship
{
public:
    Spaceship(SpaceShipSpawnParam param);

    Bullet* Shoot();
    void MoveLeft();
    void MoveRight();

    FLocation2D GetLocation() const { return m_Location; }
    const ENetRole GetNetRole() { return m_NetRole; }

    void BindRequestBulletDelegate(std::function<Bullet*()> function) { m_RequestBulletDelegate = function; }

private:
    ENetRole m_NetRole;
    FLocation2D m_Location;
    EDirection m_Direction;

    std::function<Bullet*()> m_RequestBulletDelegate;
};

