#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "shader.h"
#include "model/block.h"
#include "model/skyblock.h"
#include "model/mesh.h"
#include "model/model.h"
#include "camera.h"
#include "scene/mainScene.h"
#include "algorithm/mazeGenerator.cpp"
#include "algorithm/interaction.cpp"

unsigned int texture;

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
bool canMove(glm::vec3 pos, glm::vec3 dir);
void blockChange(int placeBlock);
void resetMaze();

int lightStatus = 0;
float renderRange = 12.0;
float renderFadeStart = 8.0;
float renderFadeEnd = 10.0;
int selected;
int ret[3];
int ret2[3];

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 0.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool firstPerson = false;
int lightMax = 1;
float rotateDegree = 0.0f;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float modelSpeed = 5.0f;

int main()
{   
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Final Project - Kevin Xia", NULL, NULL);
    if (window == NULL){
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_click_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
    // build and compile our shader zprogram
    // ------------------------------------
    Shader shader("./src/shader/shader.vs", "./src/shader/shader.fs");
    Shader depthShader("./src/shader/depth.vs", "./src/shader/depth.fs");
    Shader sunShader("./src/shader/sun.vs", "./src/shader/sun.fs");
    Shader skyShader("./src/shader/skyblock.vs", "./src/shader/skyblock.fs");

    unsigned int skyblockVAO = loadSkybox();
    unsigned int skyblockTexture = loadCubemapTexture();

    // shader configuration
    // --------------------
    skyShader.use();
    skyShader.setInt("skybox", 0);

    stbi_set_flip_vertically_on_load(false);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load Resource
    loadResource();

    // Depth Map
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMapFBO2;
    glGenFramebuffers(1, &depthMapFBO2);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int depthMap2;
    glGenTextures(1, &depthMap2);
    glBindTexture(GL_TEXTURE_2D, depthMap2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap2, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Shader
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowSunMap", 1);
    shader.setInt("shadowLightMap", 2);


    // Load Maze
    resetMaze();

    // Data
    float currentFrame;
    glm::vec3 sun;
    glm::vec3 spotDirection;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 sunColor;
    glm::vec3 sunAmbient;
    glm::vec3 sunDiffuse;
    glm::vec3 lightColor;
    glm::vec3 lightAmbient;
    glm::vec3 lightDiffuse;
    glm::mat4 model;
    while(!glfwWindowShouldClose(window)){
        selected = raytracing(viewPos, camera.Front, 5, &maze, ret, ret2);
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        sun = glm::vec3(0.0f, renderRange * sin(glfwGetTime() / 10), renderRange * cos(glfwGetTime() / 10));
        // sun = glm::vec3(0.0f, renderRange * sin(0.3f), -renderRange * cos(0.3f));
        sun += modelPos;

        // Input
        processInput(window);

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Spot Light
        spotDirection = camera.Front;
        // Render to depth map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        // Render Sun Shadow
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        configureSunMatrix("lightSpaceMatrix", depthShader, sun, modelPos);
        renderScene(depthShader, renderRange, false, camera, true, ret, selected);
        // Render Spot Shadow
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO2);
        glClear(GL_DEPTH_BUFFER_BIT);
        if(lightStatus == 0){
            configureSpotLightMatrix("lightSpaceMatrix", depthShader, viewPos, viewPos + spotDirection);
            renderScene(depthShader, renderRange, true, camera, true, ret, selected);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render scene
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        configureSunMatrix("sunSpaceMatrix", shader, sun, modelPos);
        configureSpotLightMatrix("spotSpaceMatrix", shader, viewPos, viewPos + spotDirection);
        // View
        if(firstPerson){
            view = camera.GetViewMatrix();
        }else{
            view = glm::lookAt(modelPos + glm::vec3(-glm::cos(glm::radians(camera.Yaw)) * 5, 10.0f, -glm::sin(glm::radians(camera.Yaw)) * 5), viewPos, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // Sun
        sunColor = glm::vec3(0.99f, 0.98f, 0.83f);
        sunAmbient = sunColor * glm::vec3(0.1f);
        sunDiffuse = sunColor * glm::vec3(0.8f);
        // Light
        lightColor = glm::vec3(0.0f,0.0f,0.0f);
        if(lightStatus == 0){
            lightColor = glm::vec3(1.0f,1.0f,1.0f);
        }
        lightAmbient = lightColor * glm::vec3(0.1f);
        lightDiffuse = lightColor * glm::vec3(0.8f);

        shader.use();
        // Set Render Value
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);     
        shader.setMat4("projection", projection);
        shader.setVec3("modelPos", modelPos);
        shader.setFloat("renderFadeStart", renderFadeStart);
        shader.setFloat("renderFadeEnd", renderFadeEnd);
        // Set Light Value
        shader.setVec3("dirLight.direction", -(sun - modelPos));
        shader.setVec3("dirLight.ambient", sunAmbient);
        shader.setVec3("dirLight.diffuse", sunDiffuse);
        shader.setVec3("dirLight.specular", sunColor);
        shader.setVec3("spotLight.ambient", lightAmbient);
        shader.setVec3("spotLight.diffuse", lightDiffuse);
        shader.setVec3("spotLight.specular", lightColor);
        shader.setVec3("spotLight.position", viewPos);
        shader.setVec3("spotLight.direction", spotDirection);
        shader.setFloat("spotLight.cutOff",   glm::cos(glm::radians(12.5f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        shader.setFloat("spotLight.constant", 1.0f);
        shader.setFloat("spotLight.linear", 0.09f);
        shader.setFloat("spotLight.quadratic", 0.032f);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap2);
        renderScene(shader, renderRange, firstPerson, camera, false, ret, selected);

        // Sun
        sunShader.use();
        glBindVertexArray(blockVAO);
        sunShader.setMat4("view", view);
        sunShader.setMat4("projection", projection);
        model = glm::mat4(1.0f);
        model = glm::translate(model, sun);
        sunShader.setMat4("model", model);
        sunShader.setVec3("sunColor", sunColor);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Skybox Data
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyShader.use();
        skyShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
        skyShader.setMat4("projection", projection);

        // Skybox Cube
        glBindVertexArray(skyblockVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyblockTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    glfwTerminate();
    return 0;
}

bool canMove(glm::vec3 pos, glm::vec3 dir, float modelSpeed, float deltaTime){
    int row = (int)round(pos[0] + modelSpeed * deltaTime * dir[0])+mazeSize;
    int height = (int)round(pos[1] + modelSpeed * deltaTime * dir[1]);
    int col = (int)round(pos[2] + modelSpeed * deltaTime * dir[2])+mazeSize;
    if(row < 0 || row >= mazeSize * 2 + 1 || col < 0 || col >= mazeSize * 2 + 1){
        return true;
    }
    int result = 1;
    for(int i = (int)pos[1]; i < 2; ++i){
        if(i >= mazeHeight){
            continue;
        }
        if(maze[i+1][row][col] != 0){
            result = 0;
        }
    }
    return result;
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods){
    if(selected == 1){
        if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
            blockChange(1);
        }else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
            blockChange(0);
        }
    }
}


double prevTime = 0;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
        if(glfwGetTime() - prevTime > 0.25){
            ++lightStatus;
            if(lightStatus > lightMax){
                lightStatus = 0;
            }
            prevTime = glfwGetTime();
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        glm::vec3 movDir = glm::vec3(-glm::cos(glm::radians(camera.Yaw)), 0.0f, -glm::sin(glm::radians(camera.Yaw)));
        if(canMove(modelPos, movDir, modelSpeed, deltaTime)){
            camera.Position += modelSpeed * deltaTime * movDir;
            modelPos += modelSpeed * deltaTime * movDir;
            viewPos = modelPos + modelHeight;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        glm::vec3 movDir = glm::vec3(glm::cos(glm::radians(camera.Yaw)), 0.0f, glm::sin(glm::radians(camera.Yaw)));
        if(canMove(modelPos, movDir, modelSpeed, deltaTime)){
            camera.Position += modelSpeed * deltaTime * movDir;
            modelPos += modelSpeed * deltaTime * movDir;
            viewPos = modelPos + modelHeight;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        glm::vec3 movDir = glm::vec3(glm::cos(glm::radians(camera.Yaw - 90)), 0.0f, glm::sin(glm::radians(camera.Yaw - 90)));
        if(canMove(modelPos, movDir, modelSpeed, deltaTime)){
            camera.Position += modelSpeed * deltaTime * movDir;
            modelPos += modelSpeed * deltaTime * movDir;
            viewPos = modelPos + modelHeight;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        glm::vec3 movDir = glm::vec3(glm::cos(glm::radians(camera.Yaw + 90)), 0.0f, glm::sin(glm::radians(camera.Yaw + 90)));
        if(canMove(modelPos, movDir, modelSpeed, deltaTime)){
            camera.Position += modelSpeed * deltaTime * movDir;
            modelPos += modelSpeed * deltaTime * movDir;
            viewPos = modelPos + modelHeight;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        camera.rotate(-5.0f);
    }
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        camera.rotate(5.0f);
    }
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        if(glfwGetTime() - prevTime > 0.5){
            resetMaze();
            prevTime = glfwGetTime();
        }
    }
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
        if(glfwGetTime() - prevTime > 0.5){
            firstPerson = !firstPerson;
            prevTime = glfwGetTime();
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    if(!firstPerson){
        yoffset = 0;
    }
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void blockChange(int placeBlock){
    if(placeBlock){
        int* block = &maze[ret2[0]][ret2[1]][ret2[2]];
        int row = (int)round(modelPos[0])+mazeSize;
        int height = (int)round(modelPos[1]);
        int col = (int)round(modelPos[2])+mazeSize;

        if(ret2[1] == row && ret2[2] == col){
            if(ret2[0] == height + 1 || ret2[0] == height + 2){
                return;
            }
        }
        // printf("Block add at (%d,%d,%d)!\n", oldRow, oldHeight, oldCol);
        *block = DESTROYABLE;
    }else{
        int* block = &maze[ret[0]][ret[1]][ret[2]];
        if(*block == DESTROYABLE){
            // printf("Block remove at (%d,%d,%d)!\n", row, height, col);
            *block = 0;
        }
    }
}

void resetMaze(){
    maze = generateMaze(mazeSize * 2 + 1, mazeSize * 2 + 1, mazeHeight, textureCount);
    glm::vec3 trans = glm::vec3(0.0f, 0.0f, 0.0f) - modelPos;
    modelPos += trans;
    viewPos = modelPos + modelHeight;
    camera.Position += trans;
    modelId = rand() % modelCount;
    // printMaze(&maze);
}