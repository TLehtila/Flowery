#include "scenebasic_uniform.h"

#include <sstream>

#include <cstdio>
#include <cstdlib>

#include "helper/texture.h"
#include "helper/particleutils.h"

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

SceneBasic_Uniform::SceneBasic_Uniform() : time(0), deltaT(0), nParticles(100), particleLifetime(50.5f), drawBuf(1), emitterPos(0.0f, -50.0f, 0.0f), emitterDir(1, 0, 1){
    flower = ObjMesh::load("media/flower1.obj", false, true);
    leaf = ObjMesh::load("media/leaf1.obj", false, true);
    leafP = ObjMesh::load("media/leaf1.obj", false, true);
}


void SceneBasic_Uniform::initScene()
{
    compile();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    setupFBO2();

    glActiveTexture(GL_TEXTURE0);
    ParticleUtils::createRandomTex1D(nParticles * 4);

    initBuffers();
    model = mat4(1.0f);

    prog.use();


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

    prog.setUniform("ParticleLifetime", particleLifetime);
    prog.setUniform("RandomTex", 0);
    prog.setUniform("Accel", vec3(0.0f, 0.4f, 0.0f));
    prog.setUniform("Emitter", emitterPos);
    prog.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));

    // Array for full-screen quad
    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };


    //initialise rotation
    flowerRotation = 0.0f;

    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates
    glBindVertexArray(0);

    prog.setUniform("EdgeThreshold", 0.05f);    //for yellow edge
    prog.setUniform("EdgeThresholdTwo", 0.005f);    //for white edge

    //atlas texture for flower and leaf
    GLuint atlasTex = Texture::loadTexture("media/texture/atlas1.png");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, atlasTex);


    float weights[5], sum, sigma2 = 8.0f;
    // Compute and sum the weights
    weights[0] = gauss(0, sigma2);
    sum = weights[0];
    for (int i = 1; i < 5; i++) {
        weights[i] = gauss(float(i), sigma2);
        sum += 2 * weights[i];
    }
    // Normalize the weights and set the uniform
    for (int i = 0; i < 5; i++) {
        std::stringstream uniName;
        uniName << "Weight[" << i << "]";
        float val = weights[i] / sum;
        prog.setUniform(uniName.str().c_str(), val);
    }
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");

        //set up transform feedback
        GLuint progHandle = prog.getHandle();
        const char* outputNames[] = { "PPosition", "Velocity", "Age", "Rotation"};
        glTransformFeedbackVaryings(progHandle, 4, outputNames, GL_SEPARATE_ATTRIBS);

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

    pass0();            //comment out other passes to see particles

    pass1();
    pass2();
    pass3();
    pass4();
}

void SceneBasic_Uniform::pass0() {
    prog.setUniform("Pass", 0);

    prog.setUniform("Time", time);
    prog.setUniform("ParticlePass", 1);
    prog.setUniform("DeltaT", deltaT);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(particleArray[1 - drawBuf]);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glVertexAttribDivisor(3, 0);
    glVertexAttribDivisor(4, 0);
    glVertexAttribDivisor(5, 0);
    glVertexAttribDivisor(6, 0);

    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    prog.setUniform("ParticlePass", 2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view = glm::lookAt(vec3(0.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 10.0f));
    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 100.0f);

    //set material properties for leafparticles
    prog.setUniform("Material.Kd", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
    prog.setUniform("Material.Shininess", 100.0f);
    model = mat4(1.0f);
    setMatrices();

    glBindVertexArray(particleArray[drawBuf]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glDrawElementsInstanced(GL_TRIANGLES, leafP->getNumVerts(), GL_UNSIGNED_INT, 0, nParticles);

    //swap buffers
    drawBuf = 1 - drawBuf;
}

void SceneBasic_Uniform::pass1() {

    prog.setUniform("Pass", 1);

    prog.setUniform("ParticlePass", 0);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //top-down view, up vector set to z instead of y
    view = glm::lookAt(vec3(0.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 10.0f));

    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 100.0f);

    //set material properties for leaves
    prog.setUniform("Material.Kd", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
    prog.setUniform("Material.Shininess", 5.0f);

    //translation, scaling, rotating, rendering leaf 1
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f, 1.0f, 1.0f));

    model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    leaf->render();

    //translation, scaling, rotating, rendering leaf 2
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f, 1.0f, 1.0f));

    model = glm::rotate(model, glm::radians(-45.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    leaf->render();

    //translation, scaling, rotating, rendering leaf 3
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f, 1.0f, 1.0f));

    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    leaf->render();


    //set material properties for flower
    prog.setUniform("Material.Kd", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Ks", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 5.0f);

    //translation, scaling, rendering, rotating
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.1f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    flowerRotation += 0.1f;
    model = glm::rotate(model, glm::radians(flowerRotation), vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    flower->render();

}

void SceneBasic_Uniform::pass2()
{
    prog.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex);

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();


    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::pass3()
{
    prog.setUniform("Pass", 3);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTex2);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass4()
{
    prog.setUniform("Pass", 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::update(float t) {
    deltaT = t - time;
    time = t;
}

void SceneBasic_Uniform::setMatrices() {
    mat4 mv = view * model;

    prog.setUniform("MV", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(view[0]), vec3(view[1]), vec3(view[2])));
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("Proj", projection);

}

void SceneBasic_Uniform::resize(int w, int h)
{

    glViewport(0, 0, w, h);
    width = w;
    height = h;

    //projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setupFBO2() {
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    // Create the texture object
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        renderTex, 0);

    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf);

    // Create the texture object
    glActiveTexture(GL_TEXTURE1); // Use texture unit 0
    glGenTextures(1, &renderTex2);
    glBindTexture(GL_TEXTURE_2D, renderTex2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        renderTex2, 0);


    // Create the depth buffer
    GLuint depthBuf2;
    glGenRenderbuffers(1, &depthBuf2);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf2);
    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);


    // Create the texture object
    glGenTextures(1, &intermediateTex);
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        intermediateTex, 0);


    // No depth buffer needed for this FBO
    // Set the targets for the fragment output variables
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}

void SceneBasic_Uniform::initBuffers() {
    //generate buffers
    glGenBuffers(2, posBuf);
    glGenBuffers(2, velBuf);
    glGenBuffers(2, age);
    glGenBuffers(2, particleRotation);

    //allocate space for buffers
    int size = nParticles * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);

    glBindBuffer(GL_ARRAY_BUFFER, particleRotation[0]);
    glBufferData(GL_ARRAY_BUFFER, 2 * size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, particleRotation[1]);
    glBufferData(GL_ARRAY_BUFFER, 2 * size, 0, GL_DYNAMIC_COPY);

    //fill first age buffer
    std::vector<GLfloat> initialAges(nParticles);
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++) {
        initialAges[i] = rate * (i - nParticles);
    }
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), initialAges.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //create vertex arrays for each set of buffers
    glGenVertexArrays(2, particleArray);

    //set up particle array 0
    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leafP->getElementBuffer());

    glBindBuffer(GL_ARRAY_BUFFER, leafP->getPositionBuffer());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, leafP->getNormalBuffer());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, particleRotation[0]);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(6);

    //set up particle array 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leafP->getElementBuffer());

    glBindBuffer(GL_ARRAY_BUFFER, leafP->getPositionBuffer());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, leafP->getNormalBuffer());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, particleRotation[1]);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);

    //set up feedback objects
    glGenTransformFeedbacks(2, feedback);

    //transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 3, particleRotation[0]);

    //transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 3, particleRotation[1]);
    
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}