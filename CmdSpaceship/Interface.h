#pragma once
#include "Utils.h"

class IPoolable
{
public:
    virtual void Initialize() = 0;
    virtual void Activate(FLocation2D WakeLocation, void* Instigator) = 0;
    virtual void Sleep() = 0;
    virtual bool IsActive() = 0;
};