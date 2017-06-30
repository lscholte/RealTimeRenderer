#pragma once

#include "shaderset.h"

struct SDL_Window;
class Scene;

class Renderer
{
    Scene* mScene;

    // ShaderSet explanation:
    // https://nlguillemot.wordpress.com/2016/07/28/glsl-shader-live-reloading/
    ShaderSet mShaders;

    GLuint* mSceneSP;
    GLuint* mShadowSP;

    int mBackbufferWidth;
    int mBackbufferHeight;
    GLuint mBackbufferFBO;
    GLuint mBackbufferColorTO;
    GLuint mBackbufferDepthTO;

    GLuint mShadowDepthTO;
    GLsizei kShadowMapResolution;
    GLuint mShadowFBO;
    GLfloat mShadowSlopeScaleBias;
    GLfloat mShadowDepthBias;

    GLsizei mTextureResolution;

    GLuint* mNormalDepthSP;
    GLuint mNormalDepthFBO;
    GLuint mNormalTO;
    GLuint mDepthTO;

    GLuint mLightPrepassFBO;
    GLuint* mLightPrepassSP;
    GLuint mLightingTO;
    GLuint mBrightnessTO;

    GLuint mBlurFBO;
    GLuint mBlurTO;
    GLuint* mBlurSP;

    GLuint* mDepthOfFieldSP;

    GLuint* mDepthVisSP;
    GLuint mNullVAO;
    bool mShowDepthVis;

    GLuint* mSkyboxSP;
    GLuint mSkyboxTextureID;
    GLfloat mSkyboxVertices[108];
    std::string mSkyboxFaces[6];
    GLuint mSkyboxVAO, mSkyboxVBO;

public:
    void Init(Scene* scene);
    void InitNormalDepth();
    void Resize(int width, int height);
    void Render();

    void RenderQuad();

    void* operator new(size_t sz);
};
