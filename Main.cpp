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
#include <unordered_map>
#include <optional>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Libraries
#include "Camera.hpp"
#include "ExVertexes.h"

struct Vertex {
    GLfloat x, y, z; // Position
    GLfloat r, g, b; // Color
    GLfloat u, v;    // Texture coordinates
    GLfloat nx, ny, nz; // Normals

    Vertex(GLfloat x = 0.0f, GLfloat y = 0.0f, GLfloat z = 0.0f,
        GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f,
        GLfloat u = 0.0f, GLfloat v = 0.0f,
        GLfloat nx = 0.0f, GLfloat ny = 0.0f, GLfloat nz = 0.0f)
        : x(x), y(y), z(z), r(r), g(g), b(b), u(u), v(v), nx(nx), ny(ny), nz(nz) {}
};

struct Triangle {
    Vertex v0, v1, v2;
};

std::vector<Vertex> loadOBJWithAssimp(const std::string& filename, std::vector<GLuint>& indices) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<Vertex> vertices;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex;
            vertex.x = mesh->mVertices[j].x;
            vertex.y = mesh->mVertices[j].y;
            vertex.z = mesh->mVertices[j].z;

            if (mesh->HasNormals()) {
                vertex.nx = mesh->mNormals[j].x;
                vertex.ny = mesh->mNormals[j].y;
                vertex.nz = mesh->mNormals[j].z;
            }

            if (mesh->mTextureCoords[0]) {
                vertex.u = mesh->mTextureCoords[0][j].x;
                vertex.v = mesh->mTextureCoords[0][j].y;
            }
            else {
                vertex.u = 0.0f;
                vertex.v = 0.0f;
            }

            vertex.r = 1.0f; // Default color
            vertex.g = 1.0f;
            vertex.b = 1.0f;

            vertices.push_back(vertex);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    return vertices;
}

std::vector<Triangle> loadTrianglesWithAssimp(const std::string& filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<Triangle> triangles;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        std::vector<Vertex> vertices(mesh->mNumVertices);

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            vertices[j].x = mesh->mVertices[j].x;
            vertices[j].y = mesh->mVertices[j].y;
            vertices[j].z = mesh->mVertices[j].z;

            if (mesh->HasNormals()) {
                vertices[j].nx = mesh->mNormals[j].x;
                vertices[j].ny = mesh->mNormals[j].y;
                vertices[j].nz = mesh->mNormals[j].z;
            }

            if (mesh->mTextureCoords[0]) {
                vertices[j].u = mesh->mTextureCoords[0][j].x;
                vertices[j].v = mesh->mTextureCoords[0][j].y;
            }
            else {
                vertices[j].u = 0.0f;
                vertices[j].v = 0.0f;
            }

            vertices[j].r = 1.0f; // Default color
            vertices[j].g = 1.0f;
            vertices[j].b = 1.0f;
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            if (face.mNumIndices == 3) { // Ensure the face is a triangle
                Triangle triangle;
                triangle.v0 = vertices[face.mIndices[0]];
                triangle.v1 = vertices[face.mIndices[1]];
                triangle.v2 = vertices[face.mIndices[2]];
                triangles.push_back(triangle);
            }
        }
    }

    return triangles;
}

struct Application {
    int             mScreenWidth = 1920;
    int             mScreenHeight = 1080;
    GLFWwindow* mGraphicsApplicationWindow = nullptr;
    GLuint          mGraphicsPipelineShaderProgram = 0;
    bool quitApplication = false;
    Camera mCamera;
};

struct Lighting {
    glm::vec3 mLightPos = glm::vec3(6.0f, 20.0f, 0.0f);
    glm::vec3 mLightDir = glm::vec3(0.0f, -1.0f, 0.0f);
    float mLightCol2[3] = { 1.0f, 1.0f, 1.0f };
    glm::vec3 mLightCol = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct baseObject {
    GLuint mVertexArrayObject = 0;
    GLuint mVertexBufferObject = 0;
    GLuint mIndexBufferObject = 0;
    glm::mat4 model;

    float m_xPos = 0.0f;
    float m_yPos = 0.0f;
    float m_zPos = g_uOffset;

    float g_uOffset = -2.0f;
    float g_uRotate = 0.0f;
    float g_uScale = 0.5f;

    std::vector<GLfloat> m_vertexData;
    std::vector<GLuint> m_indexBufferData;
    std::vector<Triangle> m_triangles;

    unsigned int textureID;
    GLuint64 textureHandle;

    void moveObject(float x, float y, float z) {
        m_xPos += x;
        m_yPos += y;
        m_zPos += z;
    }

    void setObjectPosition(float x, float y, float z) {
        m_xPos = x;
        m_yPos = y;
        m_zPos = z;
    }

    void loadFromOBJ(const std::string& filename) {
        std::vector<GLuint> indices;
        std::vector<Vertex> vertices = loadOBJWithAssimp(filename, indices);

        m_vertexData.clear();
        for (const auto& vertex : vertices) {
            m_vertexData.insert(m_vertexData.end(), { vertex.x, vertex.y, vertex.z, vertex.r, vertex.g, vertex.b, vertex.u, vertex.v, vertex.nx, vertex.ny, vertex.nz });
        }

        m_indexBufferData = indices;
    }

    void loadTrianglesFromOBJ(const std::string& filename) {
        m_triangles = loadTrianglesWithAssimp(filename);
    }
};

struct baseModel : baseObject{
    std::vector<baseObject>modelObjects;

    int addObject(baseObject obj) { 
        modelObjects.push_back(obj); 
        size_t pos = modelObjects.size() - 1; 
        return pos; 
    }

    void moveModel(float x, float y, float z) {
        for (auto& objects : modelObjects) {
            objects.m_xPos += (x - m_xPos);
            objects.m_yPos += (y - m_yPos);
            objects.m_zPos += (z - m_zPos);
        }
    }

    void setModelPosition(float x, float y, float z) {
        for (auto& objects : modelObjects) {
            m_xPos = x;
            m_yPos = y;
            m_zPos = z;
        }
    }
};

// vvvvvvvvvvvvvvv Global Variables vvvvvvvvvvvvvvv
Application gApp;

std::vector<baseObject* >gGameObjects;
std::vector<baseModel* >gGameModels;
baseObject gMesh1;
baseObject gMesh2;

unsigned int gTexture;
unsigned int gGrassTexture;
ExVertexes gPreMade;

GLuint gDepthMapFBO;
GLuint gDepthMap;
const unsigned int gSHADOW_WIDTH = 1024, gSHADOW_HEIGHT = 1024;

Lighting gLight;

float gSpeed = 0.001f;
// ^^^^^^^^^^^^^^^ Global Variables ^^^^^^^^^^^^^^^

std::vector<GLuint64> textureHandles;

GLuint64 loadBindlessTexture2D(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        std::cout << "Texture Loaded by STB Image!" << std::endl;
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cout << "Unsupported number of channels: " << nrChannels << std::endl;
            stbi_image_free(data);
            return 0;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }
    stbi_image_free(data);

    // Generate and make the texture handle resident
    GLuint64 textureHandle = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(textureHandle);

    return textureHandle;
}

void loadTextures() {
    textureHandles.push_back(loadBindlessTexture2D("assets/textures/wine.jpg"));
    textureHandles.push_back(loadBindlessTexture2D("assets/textures/GrassTextureTest.png"));
}

unsigned int loadTexture2D(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        std::cout << "Texture Loaded by STB Image!" << std::endl;
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cout << "Unsupported number of channels: " << nrChannels << std::endl;
            stbi_image_free(data);
            return -1;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }
    stbi_image_free(data);
    return texture;
}

//Collision
/*
std::optional<glm::vec3> rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayVector, const Triangle& triangle) {
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = triangle.v1.position - triangle.v0.position;
    glm::vec3 edge2 = triangle.v2.position - triangle.v0.position;
    glm::vec3 h = glm::cross(rayVector, edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return std::nullopt; // This ray is parallel to this triangle.
    }
    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - triangle.v0.position;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return std::nullopt;
    }
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayVector, q);
    if (v < 0.0f || u + v > 1.0f) {
        return std::nullopt;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);
    if (t > EPSILON) { // ray intersection
        return rayOrigin + rayVector * t;
    }
    else { // This means that there is a line intersection but not a ray intersection.
        return std::nullopt;
    }
}


bool checkTriangleCollision(const Triangle& t1, const Triangle& t2) {
    // Implement the Möller–Trumbore intersection algorithm
    // This is a complex algorithm, so refer to detailed resources for implementation
    return false; // Placeholder
}

void checkCollisions(const std::vector<Triangle>& model1, const std::vector<Triangle>& model2) {
    for (const auto& t1 : model1) {
        for (const auto& t2 : model2) {
            if (checkTriangleCollision(t1, t2)) {
                // Handle collision
                std::cout << "Collision detected!" << std::endl;
            }
        }
    }
}

struct BoundingSphere {
    glm::vec3 center;
    float radius;
};

bool checkSphereTriangleCollision(const BoundingSphere& sphere, const Triangle& triangle) {
    // Implement sphere-triangle collision detection
    return false; // Placeholder
}

void checkCameraCollisions(const Camera& camera, const std::vector<Triangle>& model) {
    BoundingSphere cameraSphere;
    cameraSphere.center = camera.mEye;
    cameraSphere.radius = 1.0f; // Adjust the radius as needed

    for (const auto& triangle : model) {
        if (checkSphereTriangleCollision(cameraSphere, triangle)) {
            // Handle collision
            std::cout << "Collision detected with camera!" << std::endl;
        }
    }
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

*/

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

std::string LoadShaderAsString(const std::string& filename) {
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()) {
        while (std::getline(myFile, line)) {
            result += line + '\n';
        }
        myFile.close();
    }

    return result;
}

GLuint CompileShader(GLuint type, const std::string& source) {
    GLuint shaderObject;

    if (type == GL_VERTEX_SHADER) {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (type == GL_FRAGMENT_SHADER) {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexshadersource, const std::string& fragmentshadersource) {
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexshadersource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentshadersource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);
    glValidateProgram(programObject);

    return programObject;
}

void CreateGraphicsPipeline() {

    std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");

    gApp.mGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void CreateShadowMap() {
    glGenFramebuffers(1, &gDepthMapFBO);
    glGenTextures(1, &gDepthMap);

    glBindTexture(GL_TEXTURE_2D, gDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gSHADOW_WIDTH, gSHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    /*
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(app->mGraphicsApplicationWindow, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    */

    GetOpenGLVersionInfo();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CCW);

    CreateShadowMap();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(app->mGraphicsApplicationWindow, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

//Vertex specification
void VertexSpecification(baseObject* mesh, int textureIndex) {
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

    mesh->textureHandle = textureHandles[textureIndex];

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
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
    gApp.mCamera.MouseLook(xpos, ypos);

    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_W) == GLFW_PRESS) {
        gApp.mCamera.MoveForward(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_S) == GLFW_PRESS) {
        gApp.mCamera.MoveBackward(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_A) == GLFW_PRESS) {
        gApp.mCamera.MoveLeft(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_D) == GLFW_PRESS) {
        gApp.mCamera.MoveRight(gSpeed);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_1) == GLFW_PRESS) {
        gApp.mCamera.enabled = false;
        glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_2) == GLFW_PRESS) {
        gApp.mCamera.enabled = true;
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

    // Bind the first texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture);

    // Bind the second texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gDepthMap);

    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    for (auto& mesh : gGameObjects) {
        mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->g_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->g_uScale, mesh->g_uScale, mesh->g_uScale));
    }

    for (auto& meshObject : gGameModels) {
        for (auto& mesh : meshObject->modelObjects) {
            mesh.model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh.m_xPos, mesh.m_yPos, mesh.m_zPos));
            mesh.model = glm::rotate(mesh.model, glm::radians(mesh.g_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
            mesh.model = glm::scale(mesh.model, glm::vec3(mesh.g_uScale, mesh.g_uScale, mesh.g_uScale));
        }
    }

    glm::mat4 view = gApp.mCamera.GetViewMatrix();

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
    GLint u_TextureHandleLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_TextureHandle");
    if (u_ModelMatrixLocation >= 0 && u_TextureHandleLocation >= 0) {
        for (auto& mesh : gGameObjects) {
            glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
            glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

            glBindVertexArray(mesh->mVertexArrayObject);
            glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        for (auto& meshObject : gGameModels) {
            for (auto& mesh : meshObject->modelObjects) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh.model[0][0]);
                glUniformHandleui64ARB(u_TextureHandleLocation, mesh.textureHandle);

                glBindVertexArray(mesh.mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh.m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
    }
    else {
        std::cout << "Could not find u_ModelMatrix or u_TextureHandle, check spelling? \n";
        exit(EXIT_FAILURE);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Lighting Settings");
    ImGui::ColorEdit3("Lighting Color", gLight.mLightCol2);
    gLight.mLightCol = {gLight.mLightCol2[0], gLight.mLightCol2[1] , gLight.mLightCol2[3] };
    ImGui::InputFloat("Lighting X", &gLight.mLightPos.x, 0.01f, 5.0f, "%.3f");
    ImGui::InputFloat("Lighting Y", &gLight.mLightPos.y, 0.01f, 5.0f, "%.3f");
    ImGui::InputFloat("Lighting Y", &gLight.mLightPos.z, 0.01f, 5.0f, "%.3f");
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glUseProgram(0);
}

void MainLoop()
{
    glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
    glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!gApp.quitApplication)
    {
        Input();

        // Render the scene from the light's perspective to create the shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
        glViewport(0, 0, gSHADOW_WIDTH, gSHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        Draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render the scene from the camera's perspective
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

int main(int argc, char* args[]) {
    baseObject objMesh1;
    objMesh1.loadFromOBJ("assets/models/wine.obj");

    baseObject objMesh2;
    objMesh1.loadFromOBJ("assets/models/wine.obj");

    baseObject objMesh3;
    objMesh2.loadFromOBJ("assets/models/4wayroom.obj");

    baseObject objMesh4;
    objMesh2.loadFromOBJ("assets/models/4wayroom.obj");

    InitializeProgram(&gApp);

    loadTextures();

    gMesh1.m_vertexData = objMesh2.m_vertexData;
    gMesh1.m_indexBufferData = objMesh2.m_indexBufferData;

    gMesh2.m_vertexData = objMesh1.m_vertexData;
    gMesh2.m_indexBufferData = objMesh1.m_indexBufferData;
    gMesh2.moveObject(0.0f, 0.125f, 0.0f);

    VertexSpecification(&gMesh1, 1);
    VertexSpecification(&gMesh2, 0);

    gGameObjects.push_back(&gMesh1);
    gGameObjects.push_back(&gMesh2);

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}
