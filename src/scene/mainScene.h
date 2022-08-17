#ifndef SCENE_H
#define SCENE_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../shader.h"
#include <vector>
#include "../camera.h"

unsigned int blockVAO;
unsigned int* stoneTexture;
int textureCount;
// Load model

Model* playerModel;
int modelCount;
int mazeSize = 50;
int mazeHeight = 2;
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 modelHeight = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 viewPos = modelPos + modelHeight;
vector<vector<vector<int>>> maze;
int modelId = 0;
int DESTROYABLE = 3;

void loadResource(){
    blockVAO = loadBlockVAO();
    stoneTexture = new unsigned int[5]{BlockTextureFromFile("stone.png"), 
    BlockTextureFromFile("cobblestone.png"), 
    BlockTextureFromFile("stonebrick_cracked.png"), 
    BlockTextureFromFile("stonebrick_mossy.png"), 
    BlockTextureFromFile("cobblestone_mossy.png")};
    // textureCount = sizeof(*stoneTexture) / sizeof(unsigned int);
    textureCount = 5;
    Model kleeModel ("data/model/Klee/Klee.smd");
    Model xiaoModel ("data/model/Xiao/Xiao.smd");
    Model albedoModel ("data/model/Albedo/Albedo.smd");
    Model sayuModel ("data/model/Sayu/Sayu.smd");
    Model yaoyaoModel ("data/model/Yaoyao/Yaoyao.smd");
    playerModel = new Model[5]{kleeModel,
                    xiaoModel,
                    albedoModel,
                    sayuModel,
                    yaoyaoModel};
    modelCount = 5;
}

void renderScene(Shader &shader, int renderRange, bool firstPerson, Camera &camera, bool shadow, int pos[], int select){
    glm::mat4 model;
    
    // Block
    shader.use();
    glBindVertexArray(blockVAO);
    glActiveTexture(GL_TEXTURE0);
    for(int i = -mazeSize; i <= mazeSize; ++i){
        for(int j = -mazeSize; j <= mazeSize; ++j){
            if(pow(modelPos[0] - i, 2) + pow(modelPos[2] - j, 2) < pow(renderRange, 2)){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i, 0.0f, j));
                shader.setMat4("model", model);
                for(int h = 0; h <= mazeHeight; ++h){
                    if(maze[h][i+mazeSize][j+mazeSize] > 0){
                        if(!shadow){
                            glBindTexture(GL_TEXTURE_2D, stoneTexture[maze[h][i+mazeSize][j+mazeSize] - 1]);   
                        }
                        if(!shadow && select == 1 && h == pos[0] && i+mazeSize == pos[1] && j+mazeSize == pos[2]){
                            shader.setInt("select", 1);
                        }else{
                            shader.setInt("select", 0);
                        }
                        shader.setMat4("model", model);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                    model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
        }
    }

    glBindVertexArray(0);

    // Draw Model
    if(!firstPerson){
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::translate(model, modelPos);
        model = glm::rotate(model, glm::radians(-camera.Yaw + 90), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
        shader.setMat4("model", model);   
        playerModel[modelId].Draw(shader);
    }
}

void configureSunMatrix(std::string name, Shader &shader, glm::vec3 sun, glm::vec3 lookPos){
    float near_plane = 0.1f, far_plane = 25.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
    glm::mat4 lightView = glm::lookAt(sun, 
                                  lookPos, 
                                  glm::vec3( 0.0f, 1.0f,  0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    shader.use();
    shader.setMat4(name, lightSpaceMatrix);
}

void configureSpotLightMatrix(std::string name, Shader &shader, glm::vec3 light, glm::vec3 lookPos){
    float near_plane = 0.1f;
    float far_plane = 25.0f;
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane); 
    glm::mat4 lightView = glm::lookAt(light, lookPos, glm::vec3( 0.0f, 1.0f,  0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    shader.use();
    shader.setMat4(name, lightSpaceMatrix);
}

#endif