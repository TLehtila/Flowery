#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/plane.h"
#include "helper/objmesh.h" 

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    glm::mat4 rotationMatrix;

    Plane plane;
    std::unique_ptr<ObjMesh> flower;
    std::unique_ptr<ObjMesh> leaf;

    GLuint fsQuad, fboHandle;

    void compile();

    void setupFBO();

    void pass1();
    void pass2();
    //void pass3();

    //float gauss(float, float);

    float rotation;

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void setMatrices();
};

#endif // SCENEBASIC_UNIFORM_H
