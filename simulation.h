#pragma once

#include <cstddef>
#include "opengl.h"
#include "packed_freelist.h"
#include "scene.h"

struct SDL_Window;
union SDL_Event;
class Scene;

class Simulation
{
    Scene* mScene;

    int mDeltaMouseX;
    int mDeltaMouseY;

    packed_freelist<Instance> mSpinningInstances;

public:
    void Init(Scene* scene);
    void HandleEvent(const SDL_Event& ev);
    void Update(float deltaTime);

    void* operator new(size_t sz);
};
