#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/plane.h"
#include "helper/objmesh.h" 
#include "helper/grid.h" 

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    glm::mat4 rotationMatrix;

    GLuint fsQuad, fboHandle, fboHandle2, renderTex, renderTex2;
    GLuint posBuf[2], velBuf[2], age[2], particleRotation[2];
    GLuint particleArray[2];
    GLuint feedback[2];

    int nParticles;
    float particleLifetime;
    float time, deltaT;
    int drawBuf;
    glm::vec3 emitterPos, emitterDir;

    GLuint renderFBO, intermediateFBO;
    GLuint intermediateTex;

    float flowerRotation;

    std::unique_ptr<ObjMesh> flower;
    std::unique_ptr<ObjMesh> leaf;
    std::unique_ptr<ObjMesh> leafP;

    void compile();

    void setupFBO2();

    void pass0();
    void pass1();
    void pass2();
    void pass3();
    void pass4();

    void initBuffers();

    float gauss(float, float);


public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void setMatrices();
};

#endif // SCENEBASIC_UNIFORM_H
