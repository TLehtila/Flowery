#include "scenebasic_uniform.h"

#include <sstream>

#include <cstdio>
#include <cstdlib>

#include "helper/texture.h"

#include <string>
using std::string;

#include <glm/glm.hpp>

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

#include <glm/gtc/matrix_transform.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() : plane(50.0f, 50.0f, 1, 1) {
    flower = ObjMesh::load("media/flowerTri.obj", false, true);
    leaf = ObjMesh::load("media/leaf.obj", false, true);
}


void SceneBasic_Uniform::initScene()
{
    compile();

    glEnable(GL_DEPTH_TEST);

    //top-down view, up vector set to z instead of y
    view = glm::lookAt(vec3(0.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));

    projection = mat4(1.0f);

    //initialise rotation
    rotation = 0.0f;

    //load flower texture
    flowerTex = Texture::loadTexture("media/texture/flowerCol.jpg");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, flowerTex);
     
    //load leaf texture
    leafTex = Texture::loadTexture("media/texture/leafCol.jpg");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, leafTex);



    float x, z;

    //set 3 light positions around the flower, slightly above it
    for (int i = 0; i < 3; i++) {
        std::stringstream name;
        name << "lights[" << i << "].Position";
        x = 2.0f * cosf((glm::two_pi<float>() / 3) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 3) * i);

        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 5.2f, z + 1.0f, 0.0f));
    }


    //set each light uniforms
    prog.setUniform("lights[0].La", vec3(0.3f, 0.3f, 0.3f));
    prog.setUniform("lights[1].La", vec3(0.1f, 0.1f, 0.1f));
    prog.setUniform("lights[2].La", vec3(0.5f, 0.5f, 0.5f));

    prog.setUniform("lights[0].Ld", vec3(0.5f, 0.5f, 0.5f));
    prog.setUniform("lights[1].Ld", vec3(0.1f, 0.1f, 0.1f));
    prog.setUniform("lights[2].Ld", vec3(0.8f, 0.8f, 0.8f));

    prog.setUniform("lights[0].Ls", vec3(0.2f, 0.2f, 0.2f));
    prog.setUniform("lights[1].Ls", vec3(0.2f, 0.2f, 0.2f));
    prog.setUniform("lights[2].Ls", vec3(0.2f, 0.2f, 0.2f));
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    //set material properties for flower
    prog.setUniform("Material.Kd", 1.5f, 1.5f, 1.5f);
    prog.setUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ka", 1.5f, 1.5f, 1.5f);
    prog.setUniform("Material.Shininess", 5.0f);

    //translation, scaling, rotating, rendering
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.1f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    rotation += 0.1f;
    model = glm::rotate(model, glm::radians(rotation), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, flowerTex);
    flower->render();

    //translation, scaling, rotating, rendering
    model = mat4(1.0f);
    model = glm::translate(model, vec3(1.0f, 1.1f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    setMatrices();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, leafTex);
    leaf->render();

    //set material properties for plane
    prog.setUniform("Material.Kd", 0.1f, 0.1f, 0.1f); 
    prog.setUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ka", 0.8f, 0.8f, 0.8f);
    prog.setUniform("Material.Shininess", 5.0f);

    //render plane
    model = mat4(1.0f);
    setMatrices();
    plane.render();
}


void SceneBasic_Uniform::update(float t) {
    //this is needed even if empty
}

void SceneBasic_Uniform::setMatrices() {
    mat4 mv = view * model;

    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(view[0]), vec3(view[1]), vec3(view[2])));
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ProjectionMatrix", projection);

}

void SceneBasic_Uniform::resize(int w, int h)
{

    glViewport(0, 0, w, h);
    width = w;
    height = h;

    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}
