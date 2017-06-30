#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include <glm/gtc/type_ptr.hpp>

#include <SDL.h>

#include "catmullromspline.hpp"
#include "glm/ext.hpp"
#include <iostream>

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    mSpinningInstances = packed_freelist<Instance>(4096);

    std::vector<uint32_t> loadedMeshIDs;

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/cube/cube.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {
        uint32_t newInstanceID;
        AddMeshInstance(mScene, loadedMeshID, &newInstanceID);

        // scale up the cube
        uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
        scene->Transforms[newTransformID].Scale = glm::vec3(2.0f);
    }

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/teapot/teapot.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {
        // place a teapot on top of the cube
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
            uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
            scene->Transforms[newTransformID].Translation += glm::vec3(0.0f, 2.0f, 0.0f);
        }

        // place a teapot on the side
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
            uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
            scene->Transforms[newTransformID].Translation += glm::vec3(3.0f, 1.0f, 4.0f);
        }

        // place another teapot on the side
        {
            uint32_t newInstanceID;
            AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
            uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
            scene->Transforms[newTransformID].Translation += glm::vec3(3.0f, 1.0f, -4.0f);
        }

        {
            uint32_t parentInstanceID, childInstanceID;
            AddMeshInstance(scene, loadedMeshID, &parentInstanceID);
            uint32_t parentTransformID = scene->Instances[parentInstanceID].TransformID;
            scene->Transforms[parentTransformID].Translation += glm::vec3(-5.0f, 1.0f, -5.0f);


            AddMeshInstance(scene, loadedMeshID, &childInstanceID);
            uint32_t childTransformID = scene->Instances[childInstanceID].TransformID;
            scene->Transforms[childTransformID].ParentID = parentTransformID;
            scene->Transforms[childTransformID].Translation = glm::vec3(4.0f, 0.0f, 0.0f);
            scene->Transforms[childTransformID].RotationOrigin = -scene->Transforms[childTransformID].Translation;
            mSpinningInstances.insert(scene->Instances[childInstanceID]);
        }
    }

//    loadedMeshIDs.clear();
//    LoadMeshesFromFile(mScene, "assets/floor/floor.obj", &loadedMeshIDs);
//    for (uint32_t loadedMeshID : loadedMeshIDs)
//    {
//        AddMeshInstance(mScene, loadedMeshID, nullptr);
//    }

    loadedMeshIDs.clear();
    LoadMeshesFromFile(mScene, "assets/sponza/sponza.obj", &loadedMeshIDs);
    for (uint32_t loadedMeshID : loadedMeshIDs)
    {
        //Ignore this one because it is ugly
        if (scene->Meshes[loadedMeshID].Name == "sponza_04")
        {
            continue;
        }
        uint32_t newInstanceID;
        AddMeshInstance(mScene, loadedMeshID, &newInstanceID);
        uint32_t newTransformID = scene->Instances[newInstanceID].TransformID;
        scene->Transforms[newTransformID].Scale = glm::vec3(1.0f / 50.0f);
    }

    Camera mainCamera;
    mainCamera.Eye = glm::vec3(10,5,0);
    glm::vec3 target = glm::vec3(0.0f);
    mainCamera.Look = normalize(target - mainCamera.Eye);
    mainCamera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera.FovY = glm::radians(70.0f);
    mScene->MainCamera = mainCamera;
    mainCamera.FocusDistance = 5;

    Light mainLight;
    mainLight.Position = glm::vec3(10,50,5);
    mainLight.Target = glm::vec3(0.0f);
    mainLight.Direction = normalize(mainLight.Target - mainLight.Position);
    mainLight.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainLight.FovY = glm::radians(70.0f);
    mScene->MainLight = mainLight;

    mScene->pointLights = packed_freelist<PointLight>(4096);

    PointLight whitePointLight;
    whitePointLight.Colour = glm::vec3(1,1,1);
    whitePointLight.Position = glm::vec3(0,3,0);
    mScene->moveablePointLightId = mScene->pointLights.insert(whitePointLight);

    PointLight orangePointLight;
    orangePointLight.Colour = glm::vec3(0.95, 0.63, 0.25);

    orangePointLight.Position = glm::vec3(22, 8, 8);
    mScene->pointLights.insert(orangePointLight);

    orangePointLight.Position = glm::vec3(-22, 8, -8);
    mScene->pointLights.insert(orangePointLight);

    orangePointLight.Position = glm::vec3(-22, 8, 8);
    mScene->pointLights.insert(orangePointLight);

    orangePointLight.Position = glm::vec3(22, 8, -8);
    mScene->pointLights.insert(orangePointLight);

    glm::vec3 cameraControlPoints[] = {
        glm::vec3(-22, 5, 0),
        glm::vec3(0, 8, 0), //start
        glm::vec3(22, 5, 0),
        glm::vec3(22, 5, -8),
        glm::vec3(0, 5, -10),
        glm::vec3(-22, 5, -8),
        glm::vec3(-22, 5, 0),
        glm::vec3(0, 8, 0),
        glm::vec3(22, 5, 0),
        glm::vec3(22, 5, 8),
        glm::vec3(0, 5, 10),
        glm::vec3(-22, 5, 8),
        glm::vec3(-22, 5, 0),
        glm::vec3(0, 8, 0), //end
        glm::vec3(22, 5, 0)
    };
    mScene->CameraPath = new CatmullRomSpline(cameraControlPoints, sizeof(cameraControlPoints)/sizeof(glm::vec3));

    glm::vec3 cameraLookControlPoints[] = {
        glm::vec3(27, 5, 0),
        glm::vec3(27, 8, 0), //start
        glm::vec3(23, 5, -11),
        glm::vec3(0, 5, -8),
        glm::vec3(-27, 5, -8),
        glm::vec3(-23, 5, 11),
        glm::vec3(27, 5, 0),
        glm::vec3(27, 8, 0),
        glm::vec3(23, 5, 11),
        glm::vec3(0, 5, 8),
        glm::vec3(-27, 5, 8),
        glm::vec3(-23, 0, -11),
        glm::vec3(27, 5, 0),
        glm::vec3(27, 8, 0), //end
        glm::vec3(23, 5, -11)
    };
    mScene->CameraLook = new CatmullRomSpline(cameraLookControlPoints, sizeof(cameraLookControlPoints)/sizeof(glm::vec3));

    mScene->EnableFreeCamera = true;
}

void Simulation::HandleEvent(const SDL_Event& ev)
{
    if (ev.type == SDL_MOUSEMOTION)
    {
        mDeltaMouseX += ev.motion.xrel;
        mDeltaMouseY += ev.motion.yrel;
    }
}

void Simulation::Update(float deltaTime)
{
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);

    float angularVelocity = 30.0f; // rotating at 30 degrees per second
    for(uint32_t instanceID : mSpinningInstances) {
        mScene->Transforms[mSpinningInstances[instanceID].TransformID].Rotation *=
         glm::angleAxis(
         glm::radians(angularVelocity * deltaTime),
         glm::vec3(0.0f, 1.0f, 0.0f));
    }

    int mx, my;
    Uint32 mouse = SDL_GetMouseState(&mx, &my);

    if (!(mScene->EnableFreeCamera))
    {
        mScene->MainCamera.Eye = mScene->CameraPath->getNextSamplePoint();
        glm::vec3 look = mScene->CameraLook->getNextSamplePoint() - mScene->MainCamera.Eye;
        mScene->MainCamera.Look = normalize(look);
        mScene->MainCamera.FocusDistance = length(look);
    }
    else if ((mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)
    {
        flythrough_camera_update(
            value_ptr(mScene->MainCamera.Eye),
            value_ptr(mScene->MainCamera.Look),
            value_ptr(mScene->MainCamera.Up),
            NULL,
            deltaTime,
            5.0f, // eye_speed
            0.1f, // degrees_per_cursor_move
            80.0f, // max_pitch_rotation_degrees
            mDeltaMouseX, mDeltaMouseY,
            keyboard[SDL_SCANCODE_W], keyboard[SDL_SCANCODE_A], keyboard[SDL_SCANCODE_S], keyboard[SDL_SCANCODE_D],
            keyboard[SDL_SCANCODE_SPACE], keyboard[SDL_SCANCODE_LCTRL],
            0);
    }

    mDeltaMouseX = 0;
    mDeltaMouseY = 0;
}

void* Simulation::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
    
