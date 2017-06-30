#include "renderer.h"

#include "scene.h"

#include "imgui.h"
#include "stb_image.h"

#include "shaders/preamble.glsl"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <SDL.h>

void Renderer::InitNormalDepth() {

    mNormalDepthSP = mShaders.AddProgramFromExts({ "shaders/normaldepth.vert", "shaders/normaldepth.frag" });
    mLightPrepassSP = mShaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lighting.frag" });
    mBlurSP = mShaders.AddProgramFromExts({ "shaders/blur.vert", "shaders/blur.frag" });
    mDepthOfFieldSP = mShaders.AddProgramFromExts({ "shaders/depthoffield.vert", "shaders/depthoffield.frag" });

    mTextureResolution = 1024;

    glDeleteTextures(1, &mNormalTO);
    glGenTextures(1, &mNormalTO);
    glBindTexture(GL_TEXTURE_2D, mNormalTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mTextureResolution, mTextureResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &mDepthTO);
    glGenTextures(1, &mDepthTO);
    glBindTexture(GL_TEXTURE_2D, mDepthTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mTextureResolution, mTextureResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteFramebuffers(1, &mNormalDepthFBO);
    glGenFramebuffers(1, &mNormalDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mNormalDepthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNormalTO, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTO, 0);
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    glDeleteTextures(1, &mLightingTO);
    glGenTextures(1, &mLightingTO);
    glBindTexture(GL_TEXTURE_2D, mLightingTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mTextureResolution, mTextureResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &mBrightnessTO);
    glGenTextures(1, &mBrightnessTO);
    glBindTexture(GL_TEXTURE_2D, mBrightnessTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mTextureResolution, mTextureResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteFramebuffers(1, &mLightPrepassFBO);
    glGenFramebuffers(1, &mLightPrepassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mLightPrepassFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mLightingTO, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mBrightnessTO, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0); //Only need to do this once.
    glDrawBuffer(GL_COLOR_ATTACHMENT1); //Only need to do this once.
    GLuint clearColor[4] = {0, 0, 0, 0};
    glClearBufferuiv(GL_COLOR, 0, clearColor);
    fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    glDeleteTextures(1, &mBlurTO);
    glGenTextures(1, &mBlurTO);
    glBindTexture(GL_TEXTURE_2D, mBlurTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mTextureResolution, mTextureResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glDeleteFramebuffers(1, &mBlurFBO);
    glGenFramebuffers(1, &mBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mBlurFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBlurTO, 0);
    fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
    }
}

void Renderer::Init(Scene* scene)
{
    mScene = scene;

    // feel free to increase the GLSL version if your computer supports it
    mShaders.SetVersion("410");
    mShaders.SetPreambleFile("shaders/preamble.glsl");

    mSceneSP = mShaders.AddProgramFromExts({ "shaders/scene.vert", "shaders/scene.frag" });
    mShadowSP = mShaders.AddProgramFromExts({ "shaders/shadow.vert", "shaders/shadow.frag" });
    mDepthVisSP = mShaders.AddProgramFromExts({ "shaders/depthvis.vert", "shaders/depthvis.frag" });
    mSkyboxSP = mShaders.AddProgramFromExts({ "shaders/skybox.vert", "shaders/skybox.frag" });

    kShadowMapResolution = 2048;
    glGenTextures(1, &mShadowDepthTO);
    glBindTexture(GL_TEXTURE_2D, mShadowDepthTO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowMapResolution,
    kShadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    const float kShadowBorderDepth[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, kShadowBorderDepth);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &mShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowDepthTO, 0);

    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &mNullVAO);
    glBindVertexArray(mNullVAO);
    glBindVertexArray(0);

    //Default to this because it seems to look nice.
    //Can be changed on the fly in the Render Options GUI
    mShadowDepthBias = 15.0f;
    mShadowSlopeScaleBias = 1.0f;

    mSkyboxFaces[0] = "assets/sandstorm/right.tga";
    mSkyboxFaces[1] = "assets/sandstorm/left.tga";
    mSkyboxFaces[2] = "assets/sandstorm/up.tga";
    mSkyboxFaces[3] = "assets/sandstorm/down.tga";
    mSkyboxFaces[4] = "assets/sandstorm/back.tga";
    mSkyboxFaces[5] = "assets/sandstorm/front.tga";

    GLfloat vertices[] = {
            // Positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

    memcpy(mSkyboxVertices, vertices, sizeof(vertices));

    glGenVertexArrays(1, &mSkyboxVAO);
    glGenBuffers(1, &mSkyboxVBO);
    glBindVertexArray(mSkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mSkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mSkyboxVertices), &mSkyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    glGenTextures(1, &mSkyboxTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyboxTextureID);

    int width, height, comp;

    glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyboxTextureID);

    for(GLuint i = 0; i < 6; i++)
    {
        stbi_uc* image = stbi_load(mSkyboxFaces[i].c_str(), &width, &height, &comp, 4);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    InitNormalDepth();
}

void Renderer::Resize(int width, int height)
{
    mBackbufferWidth = width;
    mBackbufferHeight = height;

    // Init Backbuffer FBO
    {                
        glDeleteTextures(1, &mBackbufferColorTO);
        glGenTextures(1, &mBackbufferColorTO);
        glBindTexture(GL_TEXTURE_2D, mBackbufferColorTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mBackbufferWidth, mBackbufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &mBackbufferDepthTO);
        glGenTextures(1, &mBackbufferDepthTO);
        glBindTexture(GL_TEXTURE_2D, mBackbufferDepthTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mBackbufferWidth, mBackbufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteFramebuffers(1, &mBackbufferFBO);
        glGenFramebuffers(1, &mBackbufferFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBackbufferColorTO, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mBackbufferDepthTO, 0);
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer::Render()
{
    mShaders.UpdatePrograms();

    Light& mainLight = mScene->MainLight;
    glm::mat4 lightWorldView = glm::lookAt(mainLight.Position, mainLight.Position + mainLight.Direction, mainLight.Up);
    glm::mat4 lightViewProjection = glm::perspective(mainLight.FovY, 1.0f, 0.01f, 100.0f);
    glm::mat4 lightWorldProjection = lightViewProjection * lightWorldView;

    Camera& mainCamera = mScene->MainCamera;
    glm::mat4 worldView = glm::lookAt(mainCamera.Eye, mainCamera.Eye + mainCamera.Look, mainCamera.Up);
    glm::mat4 viewProjection = glm::perspective(mainCamera.FovY, (float)mBackbufferWidth / mBackbufferHeight, 0.01f, 100.0f);
    glm::mat4 worldProjection = viewProjection * worldView;

    if (ImGui::Begin("Lighting Options", 0, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Checkbox("Show shadow map", &mShowDepthVis);
        ImGui::SliderFloat("Slope Scale Bias", &mShadowSlopeScaleBias, 0.0f, 10.0f);
        ImGui::SliderFloat("Depth Bias", &mShadowDepthBias, 0.0f, 1000.0f);
        ImGui::SliderFloat3("Light Position", value_ptr(mainLight.Position), -50.0f, 50.0f);
        ImGui::SliderFloat3("Light1 Position", value_ptr(mScene->pointLights[mScene->moveablePointLightId].Position), -50.0f, 50.0f);
        ImGui::SliderFloat("Focus Dist", &(mainCamera.FocusDistance), 0.0f, 50.0f);
        ImGui::Checkbox("Enable Free Cam", &(mScene->EnableFreeCamera));

        if(mainLight.Target == mainLight.Position) {
            mainLight.Target += 0.01;
        }
        mainLight.Direction = normalize(mainLight.Target - mainLight.Position);
    }
    ImGui::End();

    // Clear last frame
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mNormalDepthFBO);
        glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mLightPrepassFBO);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mBlurFBO);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    //First pass. Renders normal and depth maps from camera's point of view
    //Also takes into consideration any bump maps that are available
    if (*mNormalDepthSP)
    {
        glUseProgram(*mNormalDepthSP);

        GLint SCENE_MODELWORLD_UNIFORM_LOCATION = glGetUniformLocation(*mNormalDepthSP, "ModelWorld");
        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mNormalDepthSP, "ModelViewProjection");
        GLint SCENE_BUMP_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mNormalDepthSP, "BumpMap");
        GLint SCENE_HAS_BUMP_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mNormalDepthSP, "HasBumpMap");
        GLint SCENE_NORMAL_MODELWORLD_UNIFORM_LOCATION = glGetUniformLocation(*mNormalDepthSP, "Normal_ModelWorld");

        glBindFramebuffer(GL_FRAMEBUFFER, mNormalDepthFBO);
        glViewport(0, 0, mTextureResolution, mTextureResolution);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);

        for (uint32_t instanceID : mScene->Instances)
        {

            const Instance* instance = &mScene->Instances[instanceID];
            const Mesh* mesh = &mScene->Meshes[instance->MeshID];
            const Transform* transform = &mScene->Transforms[instance->TransformID];

            glm::mat4 modelWorld;
            for (const Transform* curr_transform = transform; true; curr_transform = &mScene->Transforms[curr_transform->ParentID]) {
                 modelWorld = translate(-curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = mat4_cast(curr_transform->Rotation) * modelWorld;
                 modelWorld = translate(curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = scale(curr_transform->Scale) * modelWorld;
                 modelWorld = translate(curr_transform->Translation) * modelWorld;

                 if (curr_transform->ParentID == -1) {
                    break;
                 }
            }

            glm::mat3 normal_ModelWorld;
            normal_ModelWorld = mat3_cast(transform->Rotation) * normal_ModelWorld;
            normal_ModelWorld = glm::mat3(scale(1.0f / transform->Scale)) * normal_ModelWorld;
            glProgramUniformMatrix3fv(*mNormalDepthSP, SCENE_NORMAL_MODELWORLD_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(normal_ModelWorld));

            glm::mat4 modelViewProjection = worldProjection * modelWorld;

            glProgramUniformMatrix4fv(*mNormalDepthSP, SCENE_MODELWORLD_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelWorld));
            glProgramUniformMatrix4fv(*mNormalDepthSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjection));

            glBindVertexArray(mesh->MeshVAO);
            for (size_t meshDrawIdx = 0; meshDrawIdx < mesh->DrawCommands.size(); meshDrawIdx++)
            {
                const GLDrawElementsIndirectCommand* drawCmd = &mesh->DrawCommands[meshDrawIdx];
                const Material* material = &mScene->Materials[mesh->MaterialIDs[meshDrawIdx]];

                glActiveTexture(GL_TEXTURE0 + SCENE_BUMP_MAP_TEXTURE_BINDING);
                glProgramUniform1i(*mNormalDepthSP, SCENE_BUMP_MAP_UNIFORM_LOCATION, SCENE_BUMP_MAP_TEXTURE_BINDING);

                if ((int)material->BumpMapID == -1)
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glProgramUniform1i(*mNormalDepthSP, SCENE_HAS_BUMP_MAP_UNIFORM_LOCATION, 0);
                }
                else
                {
                    const BumpMap* bumpMap = &mScene->BumpMaps[material->BumpMapID];
                    glBindTexture(GL_TEXTURE_2D, bumpMap->BumpMapTO);
                    glProgramUniform1i(*mNormalDepthSP, SCENE_HAS_BUMP_MAP_UNIFORM_LOCATION, 1);
                }

                glDrawElementsBaseVertex(GL_TRIANGLES, drawCmd->count, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * drawCmd->firstIndex), drawCmd->baseVertex);
            }
            glBindVertexArray(0);

        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(0);
    }

    //Light pre-pass. Render all the point lights in the scene
    if (*mLightPrepassSP)
    {
        glUseProgram(*mLightPrepassSP);

        GLint SCENE_CAMERADIR_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "CameraDir");
        GLint SCENE_LIGHTPOS_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "LightPos");
        GLint SCENE_LIGHTCOLOUR_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "LightColour");
        GLint SCENE_INVERSEPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "InverseProjection");
        GLint SCENE_INVERSEVIEW_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "InverseView");

        glProgramUniformMatrix4fv(*mLightPrepassSP, SCENE_INVERSEPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(inverse(viewProjection)));
        glProgramUniformMatrix4fv(*mLightPrepassSP, SCENE_INVERSEVIEW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(inverse(worldView)));

        glProgramUniform3fv(*mLightPrepassSP, SCENE_CAMERADIR_UNIFORM_LOCATION, 1, value_ptr(mainCamera.Look));

        glActiveTexture(GL_TEXTURE0 + SCENE_NORMAL_MAP_TEXTURE_BINDING);
        GLint SCENE_NORMAL_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "NormalMap");
        glUniform1i(SCENE_NORMAL_MAP_UNIFORM_LOCATION, SCENE_NORMAL_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mNormalTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_DEPTH_MAP_TEXTURE_BINDING);
        GLint SCENE_DEPTH_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "DepthMap");
        glUniform1i(SCENE_DEPTH_MAP_UNIFORM_LOCATION, SCENE_DEPTH_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mDepthTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_LIGHTING_MAP_TEXTURE_BINDING);
        GLint SCENE_IMAGE_UNIFORM_LOCATION = glGetUniformLocation(*mLightPrepassSP, "Image");
        glUniform1i(SCENE_IMAGE_UNIFORM_LOCATION, SCENE_LIGHTING_MAP_TEXTURE_BINDING);

        glBindFramebuffer(GL_FRAMEBUFFER, mLightPrepassFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers);
        glViewport(0, 0, mTextureResolution, mTextureResolution);
        glEnable(GL_FRAMEBUFFER_SRGB);

        for (uint32_t pointLightId : mScene->pointLights)
        {
            glBindTexture(GL_TEXTURE_2D, mLightingTO);

            PointLight pointLight = mScene->pointLights[pointLightId];

            glProgramUniform3fv(*mLightPrepassSP, SCENE_LIGHTPOS_UNIFORM_LOCATION, 1, value_ptr(pointLight.Position));
            glProgramUniform3fv(*mLightPrepassSP, SCENE_LIGHTCOLOUR_UNIFORM_LOCATION, 1, value_ptr(pointLight.Colour));

            RenderQuad();
        }

        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(0);
    }

    //Light Bloom Pass
    if (*mBlurSP)
    {
        glUseProgram(*mBlurSP);

        GLint SCENE_ISHORIZONTAL_UNIFORM_LOCATION = glGetUniformLocation(*mBlurSP, "IsHorizontalPass");

        glActiveTexture(GL_TEXTURE0 + BLUR_BRIGHTNESS_TEXTURE_BINDING);
        GLint SCENE_IMAGE_UNIFORM_LOCATION = glGetUniformLocation(*mBlurSP, "Image");
        glUniform1i(SCENE_IMAGE_UNIFORM_LOCATION, BLUR_BRIGHTNESS_TEXTURE_BINDING);

        glBindFramebuffer(GL_FRAMEBUFFER, mBlurFBO);
        glEnable(GL_FRAMEBUFFER_SRGB);

        //Do horizontal blur
        glUniform1i(SCENE_ISHORIZONTAL_UNIFORM_LOCATION, true);
        glBindTexture(GL_TEXTURE_2D, mBrightnessTO);
        RenderQuad();

        //Do vertical blur
        glUniform1i(SCENE_ISHORIZONTAL_UNIFORM_LOCATION, false);
        glBindTexture(GL_TEXTURE_2D, mBlurTO);
        RenderQuad();

        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    //Renders shadow map from light's point of view
    if (*mShadowSP)
    {
        glUseProgram(*mShadowSP);

        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mShadowSP, "ModelViewProjection");

        glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, kShadowMapResolution, kShadowMapResolution);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(mShadowSlopeScaleBias, mShadowDepthBias);

        for (uint32_t instanceID : mScene->Instances)
        {

            const Instance* instance = &mScene->Instances[instanceID];
            const Mesh* mesh = &mScene->Meshes[instance->MeshID];
            const Transform* transform = &mScene->Transforms[instance->TransformID];

            glm::mat4 modelWorld;
            for (const Transform* curr_transform = transform; true; curr_transform = &mScene->Transforms[curr_transform->ParentID]) {
                 modelWorld = translate(-curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = mat4_cast(curr_transform->Rotation) * modelWorld;
                 modelWorld = translate(curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = scale(curr_transform->Scale) * modelWorld;
                 modelWorld = translate(curr_transform->Translation) * modelWorld;

                 if (curr_transform->ParentID == -1) {
                    break;
                 }
            }

            glm::mat4 modelViewProjection = lightWorldProjection * modelWorld;

            glProgramUniformMatrix4fv(*mShadowSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjection));

            glBindVertexArray(mesh->MeshVAO);
            for (size_t meshDrawIdx = 0; meshDrawIdx < mesh->DrawCommands.size(); meshDrawIdx++)
            {
                const GLDrawElementsIndirectCommand* drawCmd = &mesh->DrawCommands[meshDrawIdx];

                glDrawElementsBaseVertex(GL_TRIANGLES, drawCmd->count, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * drawCmd->firstIndex), drawCmd->baseVertex);
            }
            glBindVertexArray(0);

        }
        glPolygonOffset(0.0f, 0.0f);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    // render scene
    if (*mSceneSP)
    {
        glUseProgram(*mSceneSP);

        // GL 4.1 = no shader-specified uniform locations. :( Darn you OSX!!!
        GLint SCENE_MODELWORLD_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "ModelWorld");
        GLint SCENE_WORLDMODEL_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "WorldModel");
        GLint SCENE_NORMAL_MODELWORLD_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "Normal_ModelWorld");
        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "ModelViewProjection");
        GLint SCENE_CAMERAPOS_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "CameraPos");
        GLint SCENE_LIGHTPOS_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "LightPos");
        GLint SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "HasDiffuseMap");
        GLint SCENE_AMBIENT_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "Ambient");
        GLint SCENE_DIFFUSE_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "Diffuse");
        GLint SCENE_SPECULAR_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "Specular");
        GLint SCENE_SHININESS_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "Shininess");
        GLint SCENE_DIFFUSE_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "DiffuseMap");
        GLint SCENE_LIGHTMATRIX_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "LightMatrix");
        GLint SCENE_ALPHA_MASK_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "AlphaMask");
        GLint SCENE_HAS_ALPHA_MASK_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "HasAlphaMask");
        GLint SCENE_SPECULAR_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "SpecularMap");
        GLint SCENE_HAS_SPECULAR_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "HasSpecularMap");
        GLint SCENE_NORMALMATRIX_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "NormalMatrix");

        glActiveTexture(GL_TEXTURE0 + SCENE_SHADOW_MAP_TEXTURE_BINDING);
        GLint SCENE_SHADOW_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "ShadowMap");
        glUniform1i(SCENE_SHADOW_MAP_UNIFORM_LOCATION, SCENE_SHADOW_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mShadowDepthTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_NORMAL_MAP_TEXTURE_BINDING);
        GLint SCENE_NORMAL_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "NormalMap");
        glUniform1i(SCENE_NORMAL_MAP_UNIFORM_LOCATION, SCENE_NORMAL_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mNormalTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_LIGHTING_MAP_TEXTURE_BINDING);
        GLint SCENE_LIGHT_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "LightMap");
        glUniform1i(SCENE_LIGHT_MAP_UNIFORM_LOCATION, SCENE_LIGHTING_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mLightingTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_BLOOM_MAP_TEXTURE_BINDING);
        GLint SCENE_BLOOM_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "BloomMap");
        glUniform1i(SCENE_BLOOM_MAP_UNIFORM_LOCATION, SCENE_BLOOM_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mBlurTO);

        glProgramUniform3fv(*mSceneSP, SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, value_ptr(mainCamera.Eye));
        glProgramUniform3fv(*mSceneSP, SCENE_LIGHTPOS_UNIFORM_LOCATION, 1, value_ptr(mainLight.Position));

        glm::mat4 offsetMatrix = glm::mat4(
         0.5f, 0.0f, 0.0f, 0.0f,
         0.0f, 0.5f, 0.0f, 0.0f,
         0.0f, 0.0f, 0.5f, 0.0f,
         0.5f, 0.5f, 0.5f, 1.0f);
        glm::mat4 lightMatrix = offsetMatrix * lightWorldProjection;

        glm::mat4 normalMatrix = offsetMatrix * worldProjection;

        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glViewport(0, 0, mBackbufferWidth, mBackbufferHeight);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);

        for (uint32_t instanceID : mScene->Instances)
        {
            const Instance* instance = &mScene->Instances[instanceID];
            const Mesh* mesh = &mScene->Meshes[instance->MeshID];
            const Transform* transform = &mScene->Transforms[instance->TransformID];

            glm::mat4 modelWorld;
            for (const Transform* curr_transform = transform; true; curr_transform = &mScene->Transforms[curr_transform->ParentID]) {
                 modelWorld = translate(-curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = mat4_cast(curr_transform->Rotation) * modelWorld;
                 modelWorld = translate(curr_transform->RotationOrigin) * modelWorld;
                 modelWorld = scale(curr_transform->Scale) * modelWorld;
                 modelWorld = translate(curr_transform->Translation) * modelWorld;

                 if (curr_transform->ParentID == -1) {
                    break;
                 }
            }

            glm::mat3 normal_ModelWorld;
            normal_ModelWorld = mat3_cast(transform->Rotation) * normal_ModelWorld;
            normal_ModelWorld = glm::mat3(scale(1.0f / transform->Scale)) * normal_ModelWorld;

            glm::mat4 modelViewProjection = worldProjection * modelWorld;
            glm::mat4 modelViewProjectionLight = lightMatrix * modelWorld;
            glm::mat4 modelViewProjectionNormal = normalMatrix * modelWorld;

            glProgramUniformMatrix4fv(*mSceneSP, SCENE_LIGHTMATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjectionLight));
            glProgramUniformMatrix4fv(*mSceneSP, SCENE_NORMALMATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjectionNormal));

            glProgramUniformMatrix4fv(*mSceneSP, SCENE_MODELWORLD_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelWorld));
            glProgramUniformMatrix4fv(*mSceneSP, SCENE_WORLDMODEL_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(glm::inverse(modelWorld)));
            glProgramUniformMatrix3fv(*mSceneSP, SCENE_NORMAL_MODELWORLD_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(normal_ModelWorld));
            glProgramUniformMatrix4fv(*mSceneSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjection));



            glBindVertexArray(mesh->MeshVAO);
            for (size_t meshDrawIdx = 0; meshDrawIdx < mesh->DrawCommands.size(); meshDrawIdx++)
            {
                const GLDrawElementsIndirectCommand* drawCmd = &mesh->DrawCommands[meshDrawIdx];
                const Material* material = &mScene->Materials[mesh->MaterialIDs[meshDrawIdx]];

                glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
                glProgramUniform1i(*mSceneSP, SCENE_DIFFUSE_MAP_UNIFORM_LOCATION, SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
                if ((int)material->DiffuseMapID == -1)
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 0);
                }
                else
                {
                    const DiffuseMap* diffuseMap = &mScene->DiffuseMaps[material->DiffuseMapID];
                    glBindTexture(GL_TEXTURE_2D, diffuseMap->DiffuseMapTO);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 1);
                }

                glActiveTexture(GL_TEXTURE0 + SCENE_ALPHA_MASK_TEXTURE_BINDING);
                glProgramUniform1i(*mSceneSP, SCENE_ALPHA_MASK_UNIFORM_LOCATION, SCENE_ALPHA_MASK_TEXTURE_BINDING);
                if ((int)material->AlphaMaskID == -1)
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_ALPHA_MASK_UNIFORM_LOCATION, 0);
                }
                else
                {
                    const AlphaMask* alphaMask = &mScene->AlphaMasks[material->AlphaMaskID];
                    glBindTexture(GL_TEXTURE_2D, alphaMask->AlphaMaskTO);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_ALPHA_MASK_UNIFORM_LOCATION, 1);
                }

                glActiveTexture(GL_TEXTURE0 + SCENE_SPECULAR_MAP_TEXTURE_BINDING);
                glProgramUniform1i(*mSceneSP, SCENE_SPECULAR_MAP_UNIFORM_LOCATION, SCENE_SPECULAR_MAP_TEXTURE_BINDING);
                if ((int)material->SpecularMapID == -1)
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_SPECULAR_MAP_UNIFORM_LOCATION, 0);
                }
                else
                {
                    const SpecularMap* specularMap = &mScene->SpecularMaps[material->SpecularMapID];
                    glBindTexture(GL_TEXTURE_2D, specularMap->SpecularMapTO);
                    glProgramUniform1i(*mSceneSP, SCENE_HAS_SPECULAR_MAP_UNIFORM_LOCATION, 1);
                }

                glProgramUniform3fv(*mSceneSP, SCENE_AMBIENT_UNIFORM_LOCATION, 1, material->Ambient);
                glProgramUniform3fv(*mSceneSP, SCENE_DIFFUSE_UNIFORM_LOCATION, 1, material->Diffuse);
                glProgramUniform3fv(*mSceneSP, SCENE_SPECULAR_UNIFORM_LOCATION, 1, material->Specular);
                glProgramUniform1f(*mSceneSP, SCENE_SHININESS_UNIFORM_LOCATION, material->Shininess);

                glDrawElementsBaseVertex(GL_TRIANGLES, drawCmd->count, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * drawCmd->firstIndex), drawCmd->baseVertex);
            }
            glBindVertexArray(0);
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(0);
    }

    if(*mSkyboxSP) {
        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mSkyboxSP, "ModelViewProjection");

        //Remove any translation from the camera so that it skybox is always
        //rendered as if the camera was at the origin of the scene
        glm::mat4 worldView = glm::lookAt(glm::vec3(0,0,0), mainCamera.Look, mainCamera.Up);

        glEnable(GL_DEPTH_TEST);

        glUseProgram(*mSkyboxSP);
        glProgramUniformMatrix4fv(*mSkyboxSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(viewProjection*worldView));

        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glBindVertexArray(mSkyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyboxTextureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        glDisable(GL_DEPTH_TEST);
    }

    if(*mDepthOfFieldSP) {
        glUseProgram(*mDepthOfFieldSP);

        GLint SCENE_CAMERAPOS_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "CameraPos");
        GLint SCENE_FOCUSDISTANCE_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "FocusDistance");
        GLint SCENE_INVERSEPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "InverseProjection");
        GLint SCENE_INVERSEVIEW_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "InverseView");
        GLint SCENE_ISHORIZONTAL_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "IsHorizontalPass");

        glProgramUniformMatrix4fv(*mDepthOfFieldSP, SCENE_INVERSEPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(inverse(viewProjection)));
        glProgramUniformMatrix4fv(*mDepthOfFieldSP, SCENE_INVERSEVIEW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(inverse(worldView)));

        glProgramUniform3fv(*mDepthOfFieldSP, SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, value_ptr(mainCamera.Eye));
        glProgramUniform1f(*mDepthOfFieldSP, SCENE_FOCUSDISTANCE_UNIFORM_LOCATION, mainCamera.FocusDistance);

        glActiveTexture(GL_TEXTURE0 + SCENE_DEPTH_MAP_TEXTURE_BINDING);
        GLint SCENE_DEPTH_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "DepthMap");
        glUniform1i(SCENE_DEPTH_MAP_UNIFORM_LOCATION, SCENE_DEPTH_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mDepthTO);

        glActiveTexture(GL_TEXTURE0 + BLUR_BRIGHTNESS_TEXTURE_BINDING);
        GLint SCENE_IMAGE_UNIFORM_LOCATION = glGetUniformLocation(*mDepthOfFieldSP, "Image");
        glUniform1i(SCENE_IMAGE_UNIFORM_LOCATION, BLUR_BRIGHTNESS_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mBackbufferColorTO);

        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glEnable(GL_FRAMEBUFFER_SRGB);

        //Do horizontal blur
        glUniform1i(SCENE_ISHORIZONTAL_UNIFORM_LOCATION, true);
        RenderQuad();

        //Do vertical blur
        glUniform1i(SCENE_ISHORIZONTAL_UNIFORM_LOCATION, false);
        RenderQuad();

        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    if (mShowDepthVis && *mDepthVisSP)
    {
         glUseProgram(*mDepthVisSP);
         GLint DEPTHVIS_TRANSFORM2D_UNIFORM_LOCATION = glGetUniformLocation(*mDepthVisSP, "Transform2D");
         GLint DEPTHVIS_ORTHO_PROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mDepthVisSP, "OrthoProjection");
         GLint DEPTHVIS_DEPTH_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mDepthVisSP, "DepthMap");
         glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
         glViewport(0, 0, mBackbufferWidth, mBackbufferHeight);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         float depthBufferVisSize = 256.0f;
         glm::mat4 transform2D = glm::translate(
                     glm::vec3((float)(mBackbufferWidth - depthBufferVisSize),
                               (float)(mBackbufferHeight - depthBufferVisSize), 0.0f)) * glm::scale(glm::vec3(depthBufferVisSize, depthBufferVisSize, 0.0f));
         glProgramUniformMatrix4fv(*mDepthVisSP, DEPTHVIS_TRANSFORM2D_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(transform2D));
         glm::mat4 ortho = glm::ortho(0.0f, (float)mBackbufferWidth, 0.0f, (float)mBackbufferHeight);
         glUniformMatrix4fv(DEPTHVIS_ORTHO_PROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(ortho));
         glActiveTexture(GL_TEXTURE0 + DEPTHVIS_DEPTH_MAP_TEXTURE_BINDING);
         glUniform1i(DEPTHVIS_DEPTH_MAP_UNIFORM_LOCATION, DEPTHVIS_DEPTH_MAP_TEXTURE_BINDING);
         glBindTexture(GL_TEXTURE_2D, mShadowDepthTO);
         // need to disable depth comparison before sampling with non-shadow sampler
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

         glBindVertexArray(mNullVAO);
         glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
         glBindVertexArray(0);

         // re-enable depth comparison
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

         glDisable(GL_BLEND);
         glBlendFunc(GL_ONE, GL_ZERO);
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         glUseProgram(0);
    }


    // Render ImGui
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        ImGui::Render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // copy to window
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackbufferFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, mBackbufferWidth, mBackbufferHeight,
            0, 0, mBackbufferWidth, mBackbufferHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer::RenderQuad()
{
    GLuint quadVAO = 0;
    GLuint quadVBO;
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void* Renderer::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
