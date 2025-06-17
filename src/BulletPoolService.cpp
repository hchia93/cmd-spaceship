#include "BulletPoolService.h"

Bullet* BulletPoolService::Request()
{
    if (!m_Pool.empty())
    {
        // Find usable first
        for (std::unique_ptr<Bullet>& element : m_Pool)
        {
            if (!element.get()->IsActive())
            {
                return element.get();
            }
        }

        // Else, create
        m_Pool.emplace_back(std::make_unique<Bullet>());
        return m_Pool.back().get();
    }
    else
    {
        // Empty pool always create
        m_Pool.emplace_back(std::make_unique<Bullet>());
        return m_Pool.back().get();
    }
    return nullptr;
}

Bullet* BulletPoolService::GetActiveBulletAt(const FLocation2D& location)
{
    for (std::unique_ptr<Bullet>& element : m_Pool)
    {
        if (element.get()->IsActive())
        {
            if (element.get()->GetLocation() == location)
            {
                return element.get();
            }
        }
    }
    return nullptr;
}

void BulletPoolService::TickAll()
{
    for (std::unique_ptr<Bullet>& element : m_Pool)
    {
        if (element.get()->IsActive())
        {
            element.get()->Tick();
        }
    }
}

void BulletPoolService::DeactivateAll()
{
    for (auto& bullet : m_Pool)
    {
        bullet->Deactivate();
    }
}