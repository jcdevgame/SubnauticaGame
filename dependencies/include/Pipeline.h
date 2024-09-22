#ifndef PIPELINE_H
#define PIPELINE_H

// Third Party Libraries
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// STL libs
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <tuple>
#include <memory>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Camera.hpp"
#include "ExVertexes.h"

struct Application {
    double deltaTime = 0.0;
    double lastTime = 0.0;
    void calculateDeltaTime();

    int             mScreenWidth = 1920;
    int             mScreenHeight = 1080;
    GLFWwindow* mGraphicsApplicationWindow = nullptr;
    GLuint          mGraphicsPipelineShaderProgram = 0;
    bool quitApplication = false;
};

struct Lighting {
    glm::vec3 mLightPos = glm::vec3(6.0f, 20.0f, 0.0f);
    glm::vec3 mLightDir = glm::vec3(0.0f, -1.0f, 0.0f);
    float mLightCol2[3] = { 1.0f, 1.0f, 1.0f };
    glm::vec3 mLightCol = glm::vec3(1.0f, 1.0f, 1.0f);
};

namespace pipeline {
    std::string LoadShaderAsString(const std::string& filename);

    GLuint CompileShader(GLuint type, const std::string& source);

    GLuint CreateShaderProgram(const std::string& vertexshadersource, const std::string& fragmentshadersource);
}

#endif