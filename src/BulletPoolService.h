#pragma once
#include <vector>
#include <memory>
#include "Bullet.h"

class BulletPoolService
{
public:
    Bullet* Request();
    void TickAll();
    Bullet* GetActiveBulletAt(const FLocation2D& location);
    void DeactivateAll();

private:
    std::vector<std::unique_ptr<Bullet>> m_Pool;
};