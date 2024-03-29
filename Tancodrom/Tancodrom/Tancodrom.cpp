﻿#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>

#define GLM_FORCE_CTOR_INIT 
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "MoveableObject.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
bool isDayTime = false;


Camera* pCamera = nullptr;
bool isTankMoving = false;
bool isHelicopterMoving = false;
bool freeCameraView = true;

MoveableObject* currentObject;

unsigned int CreateTexture(const std::string& strTexturePath)
{
    unsigned int textureId = -1;

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Failed to load texture: " << strTexturePath << std::endl;
    }
    stbi_image_free(data);

    return textureId;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


void renderScene(const Shader& shader);
void renderFloor();
void renderModel(Shader& ourShader, Model& ourModel, const glm::vec3& position, float rotationAngle, const glm::vec3& scale);

double deltaTime = 0.0f; // time between current frame and last frame
double lastFrame = 0.0f;

float skyboxVertices[] = {
    -1.0f,  -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,
     -1.0f, -1.0f, -1.0f,
     -1.0f, 1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
    1,2,6,
    6,5,1,
    0,4,7,
    7,3,0,
    4,5,6,
    6,7,4,
    0,3,2,
    2,1,0,
    0,1,5,
    5,4,0,
    3,7,6,
    6,2,3
};

double currentMoveTank = 0.0f;
double currentRotatePropeller = 0.0f;

std::vector<glm::vec3> tanksPositions =
{
    glm::vec3(5.0f, -1.55f, 10.0f),
    glm::vec3(-5.0f, -1.55f, 10.5f),
    glm::vec3(10.0f, -1.55f, 20.0f),
    glm::vec3(-10.0f, -1.55f, 20.0f),
    glm::vec3(0.0f, -1.55f, 20.0f)
};

std::vector<glm::vec3> helicoptersBodyPositions =
{
    glm::vec3(-18.0f, 5.0f, -10.0f),
    glm::vec3(18.0f, 5.0f, -10.0f)
};

std::vector<glm::vec3> helicoptersPropellerPositions =
{
    glm::vec3(-18.0f, 6.0f, -10.0f),
    glm::vec3(18.0f, 6.0f, -10.0f)
};

Model tankModel, helicopterModel, propellerModel;

MoveableObject tankVehicle, helicopterVehicle, propeller;

std::vector<glm::vec3> mountainsPositions =
{
    glm::vec3(-20.5f, -1.55f, 10.0f),
    glm::vec3(50.0f, -1.55f, -25.0f),
    glm::vec3(20.0f, -1.55f, 20.0f),
    glm::vec3(-35.0f, -1.55f, -15.0f),
    glm::vec3(-25.0f, -1.55f, -30.0f),
    glm::vec3(-50.0f, -1.55f, -35.0f),
    //far
    glm::vec3(-30.0f, -1.55f, -150.0f),
    glm::vec3(-15.0f, -1.55f, -145.0f),
    glm::vec3(15.0f, -1.55f, -155.0f),

    //far far
    glm::vec3(70.0f, -1.55f, 200.0f),
    glm::vec3(-70.0f, -1.55f, 200.0f),
    glm::vec3(90.0f, -1.55f, 160.0f),

    glm::vec3(160.0f, -1.55f, 100.0f),
    glm::vec3(-160.0f, -1.55f, 100.0f)
};

std::vector<glm::vec3> mountainsScales =
{
    glm::vec3(0.05f),
    glm::vec3(0.8f),
    glm::vec3(0.05f),
    glm::vec3(0.1f),
    glm::vec3(0.1f),
    glm::vec3(0.6f),
    //far
    glm::vec3(0.6f),
    glm::vec3(0.8f),
    glm::vec3(0.7f),
    //far far
    glm::vec3(0.6f),
    glm::vec3(0.6f),
    glm::vec3(0.8f),

    glm::vec3(0.4f),
    glm::vec3(0.4f)
};


std::vector<std::string> facesDay
{
    "skybox_images\\skybox_right.jpg",
    "skybox_images\\skybox_left.jpg",
    "skybox_images\\skybox_top.jpg",
    "skybox_images\\skybox_bottom.jpg",
    "skybox_images\\skybox_back.jpg",
    "skybox_images\\skybox_front.jpg"
};

std::vector<std::string>facesNight
{

    "skybox_images_night\\skybox_night_front.jpg",
    "skybox_images_night\\skybox_night_back.jpg",
    "skybox_images_night\\skybox_night_left.jpg",
    "skybox_images_night\\skybox_night_right.jpg",
    "skybox_images_night\\skybox_night_top.jpg",
    "skybox_images_night\\skybox_night_bottom.jpg"
};

float blendFactor = 0;
float ambientFactor = 0.9;

int main(int argc, char** argv)
{
    std::string strFullExeFileName = argv[0];
    std::string strExePath;
    const size_t last_slash_idx = strFullExeFileName.rfind('\\');
    if (std::string::npos != last_slash_idx)
    {
        strExePath = strFullExeFileName.substr(0, last_slash_idx);
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tancodrom", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glewInit();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 7.0f, 25.0f));

    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shadowMappingShader("ShadowMapping.vs", "ShadowMapping.fs");
    Shader shadowMappingDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");
    Shader ModelShader("ModelShader.vs", "ModelShader.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");

    // load textures
    // -------------
    unsigned int floorTexture = CreateTexture("Assets\\skybox_bottom2.jpg");

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shadowMappingShader.Use();
    shadowMappingShader.SetInt("diffuseTexture", 0);
    shadowMappingShader.SetInt("shadowMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

    glEnable(GL_CULL_FACE);

    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    unsigned int cubemapTexture;

    unsigned int cubemapTextureNight;

    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < facesDay.size(); i++)
    {
        unsigned char* data = stbi_load(facesDay[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D
            (
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGB,
                width,
                height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                data
            );
            glGetError();
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << facesDay[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glGenTextures(1, &cubemapTextureNight);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //night skybox
    for (unsigned int i = 0; i < facesNight.size(); i++) {
        unsigned char* data = stbi_load(facesNight[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            glGetError();
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << facesNight[i] << std::endl;
        }
    }

    tankModel = Model("Assets\\tank.obj");
    tankVehicle = MoveableObject(tankModel, SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, -1.55f, 0.0f));

    currentObject = &tankVehicle;

    helicopterModel = Model("Assets\\body.obj");
    helicopterVehicle = MoveableObject(helicopterModel, SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 5.0f, 0.0f));
    helicopterVehicle.SetRotation(90.0f);

    propellerModel = Model("Assets\\propeller.obj");
    propeller = MoveableObject(propellerModel, SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 6.0f, 0.0f));

    Model mountainModel("Assets\\mountain.obj");
    Model tankModel("Assets\\tank.obj");

    while (!glfwWindowShouldClose(window))
    {

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
        glDepthMask(GL_TRUE);
        double currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;

        glm::mat4 lightRotationMatrix = glm::mat4(
            glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            glm::vec4(sin(lastFrame), 0.0f, cos(lastFrame), 1.0f)
        );

        glm::vec4 rotatedLightPos = lightRotationMatrix * glm::vec4(lightPos, 1.0f);

        float near_plane = 1.0f, far_plane = 10.5f;
        lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view
        shadowMappingDepthShader.Use();
        shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        renderScene(shadowMappingDepthShader);

        currentMoveTank += 0.0005;
        currentRotatePropeller += 10.0f;
        propeller.SetRotation(propeller.GetRotation() + currentRotatePropeller);

        float tankRotation = 0.0f;
        glm::vec3 tankScale = glm::vec3(0.5f);
        glm::vec3 helicopterScale = glm::vec3(1.5f);
        float helicopterRotation = 90.0f;
        glm::vec3 propellerScale = glm::vec3(1.5f);

        renderModel(shadowMappingDepthShader, tankVehicle.GetVehicleModel(), tankVehicle.GetPosition(), tankVehicle.GetRotation(), tankScale);

        renderModel(shadowMappingDepthShader, helicopterVehicle.GetVehicleModel(), helicopterVehicle.GetPosition(), helicopterVehicle.GetRotation(), helicopterScale);

        renderModel(shadowMappingDepthShader, propeller.GetVehicleModel(), propeller.GetPosition(), propeller.GetRotation(), propellerScale);

        for (auto& tankPosition : tanksPositions)
        {
            renderModel(shadowMappingDepthShader, tankModel, tankPosition - glm::vec3(0.0f, 0.0f, currentMoveTank), tankRotation, tankScale);
        }

        for (auto& helicopterBody : helicoptersBodyPositions)
        {
            renderModel(shadowMappingDepthShader, helicopterModel, helicopterBody - glm::vec3(0.0f, 0.0f, currentMoveTank * 3), helicopterRotation, helicopterScale);
        }

        for (auto& helicopterPropeller : helicoptersPropellerPositions)
        {
            renderModel(shadowMappingDepthShader, propellerModel, helicopterPropeller - glm::vec3(0.0f, 0.0f, currentMoveTank * 3), helicopterRotation + currentRotatePropeller, propellerScale);
        }

        float mountainRotation = 0.0f;
        glm::vec3 mountainScale = glm::vec3(0.1f);

        for (int i = 0; i < mountainsPositions.size(); i++)
        {
            renderModel(shadowMappingDepthShader, mountainModel, mountainsPositions[i] - glm::vec3(0.0f, 0.0f, 0.0f), mountainRotation, mountainsScales[i]);
        }

        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadowMappingShader.Use();
        glm::mat4 projection = pCamera->GetProjectionMatrix();
        glm::mat4 view = pCamera->GetViewMatrix(currentObject);
        shadowMappingShader.SetMat4("projection", projection);
        shadowMappingShader.SetMat4("view", view);
        shadowMappingShader.SetFloat("ambientFactor", ambientFactor);
        // set light uniforms
        shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
        shadowMappingShader.SetVec3("lightPos", lightPos);
        shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glDisable(GL_CULL_FACE);
        renderScene(shadowMappingShader);

        renderModel(ModelShader, tankVehicle.GetVehicleModel(), tankVehicle.GetPosition(), tankVehicle.GetRotation(), tankScale);

        renderModel(ModelShader, helicopterVehicle.GetVehicleModel(), helicopterVehicle.GetPosition(), helicopterVehicle.GetRotation(), helicopterScale);
        renderModel(ModelShader, propeller.GetVehicleModel(), propeller.GetPosition(), propeller.GetRotation(), propellerScale);

        for (auto& tankPosition : tanksPositions)
        {
            renderModel(ModelShader, tankModel, tankPosition - glm::vec3(0.0f, 0.0f, currentMoveTank), tankRotation, tankScale);
        }

        for (int i = 0; i < mountainsPositions.size(); i++)
        {
            renderModel(ModelShader, mountainModel, mountainsPositions[i] - glm::vec3(0.0f, 0.0f, 0.0f), mountainRotation, mountainsScales[i]);
        }

        for (auto& helicopterBody : helicoptersBodyPositions)
        {
            renderModel(ModelShader, helicopterModel, helicopterBody - glm::vec3(0.0f, 0.0f, currentMoveTank * 3), helicopterRotation, helicopterScale);
        }

        for (auto& helicopterPropeller : helicoptersPropellerPositions)
        {
            renderModel(ModelShader, propellerModel, helicopterPropeller - glm::vec3(0.0f, 0.0f, currentMoveTank * 3), helicopterRotation + currentRotatePropeller, propellerScale);
        }

        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // White light
        glm::vec3 lightDir = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)); // Example direction
        glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.31f); // Example color (rust)

        ModelShader.Use();
        ModelShader.SetVec3("lightColor", lightColor);
        ModelShader.SetVec3("lightDir", lightDir);
        ModelShader.SetVec3("objectColor", objectColor);

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        skyboxShader.Use();

        skyboxShader.SetInt("skybox1", 0);
        skyboxShader.SetInt("skybox2", 1);
        skyboxShader.SetFloat("blendFactor", blendFactor);

        glm::mat4 viewSB = glm::mat4(glm::mat3(pCamera->GetViewMatrix(currentObject))); 

        glm::mat4 projectionSB = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        skyboxShader.SetMat4("view", viewSB);
        skyboxShader.SetMat4("projection", projectionSB);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);

        glBindVertexArray(skyboxVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete pCamera;
    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{
    // floor
    glm::mat4 model;
    float deltaY = -1.0f;
    model = glm::translate(model, glm::vec3(0.0f, deltaY, 0.0f));
    shader.SetMat4("model", model);
    renderFloor();
}


unsigned int planeVAO = 0;
void renderFloor()
{
    unsigned int planeVBO;

    if (planeVAO == 0)
    {
        float planeVertices[] = {
            200.0f, -0.5f,  200.0f,  0.0f, 1.0f, 0.0f,  200.0f,  0.0f,
            -200.0f, -0.5f,  200.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
            -200.0f, -0.5f, -200.0f,  0.0f, 1.0f, 0.0f,   0.0f, 200.0f,

            200.0f, -0.5f,  200.0f,  0.0f, 1.0f, 0.0f,  200.0f,  0.0f,
            -200.0f, -0.5f, -200.0f,  0.0f, 1.0f, 0.0f,   0.0f, 200.0f,
            200.0f, -0.5f, -200.0f,  0.0f, 1.0f, 0.0f,  200.0f, 200.0f
        };
        // plane VAO
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }

    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

unsigned int modelVAO = 0;
unsigned int modelVBO = 0;
unsigned int modelEBO;
void renderModel(Shader& ourShader, Model& ourModel, const glm::vec3& position, float rotationAngle, const glm::vec3& scale) 
{
    ourShader.Use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position); 
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 
    model = glm::scale(model, scale); 

    glm::mat4 viewMatrix = pCamera->GetViewMatrix(currentObject);
    glm::mat4 projectionMatrix = pCamera->GetProjectionMatrix();

    ourShader.SetMat4("model", model);
    ourShader.SetMat4("view", viewMatrix);
    ourShader.SetMat4("projection", projectionMatrix);

    ourModel.Draw(ourShader);
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
    {
        blendFactor = std::min(blendFactor + 0.001, 1.0);
        ambientFactor = std::max(ambientFactor - 0.001, 0.34);
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        blendFactor = std::max(blendFactor - 0.001, 0.0);
        ambientFactor = std::min(ambientFactor + 0.001, 0.9);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        pCamera->SetPosition(tankVehicle.GetPosition() + glm::vec3(0.0f, 3.0f, 8.5f));
        freeCameraView = false;
        isHelicopterMoving = false;
        isTankMoving = true;
        pCamera->SetFreeCamera(false);
        currentObject = &tankVehicle;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        pCamera->SetPosition(helicopterVehicle.GetPosition() + glm::vec3(0.0f, 2.0f, 12.5f));
        freeCameraView = false;
        isTankMoving = false;
        isHelicopterMoving = true;
        pCamera->SetFreeCamera(false);
        currentObject = &helicopterVehicle;
    }
    
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        freeCameraView = true;
        pCamera->Reset(SCR_WIDTH, SCR_HEIGHT);
        isTankMoving = false;
        isHelicopterMoving = false;
        pCamera->SetFreeCamera(true);
    }

    //tank Movement
    if (isTankMoving)
    {

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            tankVehicle.ProcessKeyboard(V_FORWARD, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            tankVehicle.ProcessKeyboard(V_BACKWARD, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            tankVehicle.ProcessKeyboard(V_LEFT, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            tankVehicle.ProcessKeyboard(V_RIGHT, (float)deltaTime);
        }
    }

    if (isHelicopterMoving)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            helicopterVehicle.ProcessKeyboard(V_FORWARD, (float)deltaTime);
            propeller.ProcessKeyboard(V_FORWARD, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            helicopterVehicle.ProcessKeyboard(V_BACKWARD, (float)deltaTime);
            propeller.ProcessKeyboard(V_BACKWARD, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            helicopterVehicle.ProcessKeyboard(V_LEFT, (float)deltaTime);
            propeller.ProcessKeyboard(V_LEFT, (float)deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            helicopterVehicle.ProcessKeyboard(V_RIGHT, (float)deltaTime);
            propeller.ProcessKeyboard(V_RIGHT, (float)deltaTime);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        pCamera->ProcessKeyboard(UP, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        pCamera->ProcessKeyboard(DOWN, (float)deltaTime);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        pCamera->Reset(width, height);

    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (freeCameraView)
        pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
    if (freeCameraView)
        pCamera->ProcessMouseScroll((float)yOffset);
}