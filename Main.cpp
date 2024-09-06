#include "Pipeline.h"
#include "objects.hpp"
#include "player.hpp"
#include <enet/enet.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Application gApp;
player gLocalPlayer;

// vvvvvvvvvvvvvvv Global Variables vvvvvvvvvvvvvv
std::vector<objects::baseObject* >gGameObjects;
std::vector<objects::baseHitbox*>gGameHitboxes;

objects::baseObject gMesh1;
objects::baseObject gMesh2;
ExVertexes gPreMade;
Lighting gLight;

float gSpeed = 0.001f;

GLuint gTexArray;
const char* gTexturePaths[3]{
    "assets/textures/wine.jpg",
    "assets/textures/GrassTextureTest.jpg",
    "assets/textures/hitboxtexture.jpg"
};
// ^^^^^^^^^^^^^^^ Global Variables ^^^^^^^^^^^^^^^

void loadTextureArray2D(const char* paths[], int layerCount, GLuint* TextureArray) {
    glGenTextures(1, TextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *TextureArray);

    int width, height, channels;
    unsigned char* data = stbi_load(paths[0], &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << paths[0] << std::endl;
        return;
    }

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layerCount);

    for (int i = 0; i < layerCount; ++i) {
        data = stbi_load(paths[i], &width, &height, &channels, 4);
        if (!data) {
            std::cerr << "Failed to load texture: " << paths[i] << std::endl;
            continue;
        }
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

// ERROR HANDLING STUFF
static void GLClearAllErrors() {
    while (glGetError() != GL_NO_ERROR) {
    }
}

static bool GLCheckErrorStatus(const char* function, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL Error:" << error << "Line:" << line << "Function:" << function << std::endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

void CreateGraphicsPipeline() {

    std::string vertexShaderSource = pipeline::LoadShaderAsString("./shaders/AMD_vert.glsl");
    std::string fragmentShaderSource = pipeline::LoadShaderAsString("./shaders/AMD_frag.glsl");

    gApp.mGraphicsPipelineShaderProgram = pipeline::CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void GetOpenGLVersionInfo() {
    std::cout << "-------------------- \n";
    std::cout << "OpenGL version info: \n";
    std::cout << "Vendor:" << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer:" << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version:" << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "-------------------- \n";
    std::cout << "\n";
    std::cout << "-------------------- \n";
    std::cout << "Program Log: \n";
}

void InitializeProgram(Application* app)
{
    if (!glfwInit()) {
        std::cout << "GLFW could not initialize\n";
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    app->mGraphicsApplicationWindow = glfwCreateWindow(app->mScreenWidth, app->mScreenHeight, "Sigma", NULL, NULL);

    if (app->mGraphicsApplicationWindow == nullptr) {
        std::cout << "GLFW window was not able to be created\n";
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(app->mGraphicsApplicationWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Glad is not initialized";
        exit(1);
    }

    GetOpenGLVersionInfo();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CCW);
}


//Vertex specification
void VertexSpecification(objects::baseObject* mesh, int textureIndex) {

    const std::vector<GLfloat>& vertexData = mesh->m_vertexData;
    const std::vector<GLuint>& indexBufferData = mesh->m_indexBufferData;

    glGenVertexArrays(1, &mesh->mVertexArrayObject);
    glBindVertexArray(mesh->mVertexArrayObject);

    glGenBuffers(1, &mesh->mVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->mIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 11, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 11, (GLvoid*)(sizeof(GLfloat) * 3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 11, (GLvoid*)(sizeof(GLfloat) * 6));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 11, (GLvoid*)(sizeof(GLfloat) * 8));

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    mesh->texArrayIndex = textureIndex;
}

//Game Loop
void Input() {
    static double mouseX = gApp.mScreenHeight / 2;
    static double mouseY = gApp.mScreenWidth / 2;

    glfwPollEvents();

    if (glfwWindowShouldClose(gApp.mGraphicsApplicationWindow)) {
        std::cout << "-------------------- \n";
        std::cout << "\n";
        std::cout << "Exiting Program... \n";
        gApp.quitApplication = true;
    }

    double xpos, ypos;
    glfwGetCursorPos(gApp.mGraphicsApplicationWindow, &xpos, &ypos);
    gLocalPlayer.m_PlayerCamera.MouseLook(xpos, ypos);

    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_W) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveForward(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_S) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveBackward(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_A) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveLeft(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_D) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveRight(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_1) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.enabled = false;
        glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_2) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.enabled = true;
        glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_3) == GLFW_PRESS) {
        gSpeed = 0.01;
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_4) == GLFW_PRESS) {
        gSpeed = 0.0001;
    }
}

void PreDraw() {
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
    glClearColor(0.0f, 1.f, 1.f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    for (auto& mesh : gGameObjects) {
        mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
    }

    for (auto& mesh : gGameHitboxes) {
        mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
    }

    glm::mat4 view = gLocalPlayer.m_PlayerCamera.GetViewMatrix();

    GLint u_ViewLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ViewMatrix");
    if (u_ViewLocation >= 0) {
        glUniformMatrix4fv(u_ViewLocation, 1, GL_FALSE, &view[0][0]);
    }
    else {
        std::cout << "Could not find u_ViewMatrix, check spelling? \n";
        exit(EXIT_FAILURE);
    }

    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)gApp.mScreenWidth / (float)gApp.mScreenHeight, 0.1f, 50.0f);
    GLint u_ProjectionLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_Projection");
    if (u_ProjectionLocation >= 0) {
        glUniformMatrix4fv(u_ProjectionLocation, 1, GL_FALSE, &perspective[0][0]);
    }
    else {
        std::cout << "Could not find u_Projection, check spelling? \n";
        exit(EXIT_FAILURE);
    }

    // Pass the light position to the shader
    GLint u_LightPosLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_LightPos");
    glUniform3fv(u_LightPosLocation, 1, glm::value_ptr(gLight.mLightPos));

    GLint u_LightColorLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_LightColor");
    glUniform3fv(u_LightColorLocation, 1, glm::value_ptr(gLight.mLightCol));

    // Define the light projection matrix
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
    glm::mat4 lightView = glm::lookAt(gLight.mLightPos, gLight.mLightPos + gLight.mLightDir, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Set up the shader
    GLint lightSpaceMatrixLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_LightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
}

void Draw() {
    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    GLint u_ModelMatrixLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ModelMatrix");
    GLint u_TextureArrayIndexLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_TextureArrayIndex");
    if (u_ModelMatrixLocation >= 0 && u_TextureArrayIndexLocation >= 0) {
        glActiveTexture(GL_TEXTURE0); // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D_ARRAY, gTexArray); // Bind the texture array
        glUniform1i(glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "textureArray"), 0); // Set the sampler to use texture unit 0

        for (auto& mesh : gGameObjects) {
            if (mesh->visible) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                glUniform1i(u_TextureArrayIndexLocation, mesh->texArrayIndex);

                glBindVertexArray(mesh->mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        for (auto& mesh : gGameHitboxes) {
            if (mesh->visible) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                glUniform1i(u_TextureArrayIndexLocation, mesh->texArrayIndex);

                glBindVertexArray(mesh->mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
    }
    else {
        std::cout << "Could not find u_ModelMatrix or u_TextureArrayIndex, check spelling? \n";
        exit(EXIT_FAILURE);
    }

    glUseProgram(0);
}

void MainLoop()
{
    glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
    glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!gApp.quitApplication)
    {
        Input();
        PreDraw();
        Draw();

        glfwSwapBuffers(gApp.mGraphicsApplicationWindow);
    }
}

void CleanUp()
{
    glfwDestroyWindow(gApp.mGraphicsApplicationWindow);
    glfwTerminate();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void clientActions() {
    InitializeProgram(&gApp);
    loadTextureArray2D(gTexturePaths, 3, &gTexArray);

    gMesh1.m_vertexData = gPreMade.m_cubeVertexData;
    gMesh1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;

    gMesh2.loadFromOBJ("assets/models/wine.obj");

    VertexSpecification(&gMesh1, 1);
    VertexSpecification(&gMesh2, 0);

    gGameObjects.push_back(&gMesh1);
    gGameObjects.push_back(&gMesh2);

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();
}

int main(int argc, char* args[]) {
    clientActions();

    return 0;
}