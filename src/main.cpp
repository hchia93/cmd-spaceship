#include "GameWorld.h"

// todo next: settle all the bullet spawning and pooling requests.
// potentially need to fix up the location and motion resolver.

int main()
{
    GameWorld world;
    while (!world.IsExiting())
    {
        world.Update();
    }
    world.Finalize();
    return 0;
}
