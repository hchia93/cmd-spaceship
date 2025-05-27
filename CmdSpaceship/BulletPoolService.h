#pragma once
#include <vector>
#include <memory>
#include "Bullet.h"

class BulletPoolService
{
public:
    Bullet* Request();
    void TickAll();

    Bullet* GetActiveBulletAt(FLocation2D Location);
private:
    std::vector<std::unique_ptr<Bullet>> m_Pool;
};