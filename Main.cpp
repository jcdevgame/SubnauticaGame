#include "Pipeline.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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

struct baseObject {
    bool visible = true;

    GLuint mVertexArrayObject = 0;
    GLuint mVertexBufferObject = 0;
    GLuint mIndexBufferObject = 0;
    glm::mat4 model;

    float m_xPos = 0.0f;
    float m_yPos = 0.0f;
    float m_zPos = m_uOffset;

    float m_uOffset = -2.0f;
    float m_uRotate = 0.0f;
    float m_uScale = 0.5f;

    float m_uScaleX = 1.0f;
    float m_uScaleY = 1.0f;
    float m_uScaleZ = 1.0f;;

    std::vector<GLfloat> m_vertexData;
    std::vector<GLuint> m_indexBufferData;

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

    void scaleObject(float sx, float sy, float sz) {
        m_uScaleX = sx;
        m_uScaleY = sy;
        m_uScaleZ = sz;
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
};

struct baseHitbox : baseObject {
    bool visible = true;
    bool negative = false;

    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 mPreviousPos;
    Camera* mCamera; // Add a reference to the camera

    void updateBounds(float offset = 0.05f) {
        if (m_vertexData.empty()) return;

        min = glm::vec3(std::numeric_limits<float>::max());
        max = glm::vec3(std::numeric_limits<float>::lowest());

        for (size_t i = 0; i < m_vertexData.size(); i += 11) { // Assuming each vertex has 11 attributes
            glm::vec3 vertex(m_vertexData[i], m_vertexData[i + 1], m_vertexData[i + 2]);

            // Apply scaling
            vertex.x *= m_uScaleX;
            vertex.y *= m_uScaleY;
            vertex.z *= m_uScaleZ;

            // Update min and max coordinates
            if (vertex.x < min.x) min.x = vertex.x;
            if (vertex.y < min.y) min.y = vertex.y;
            if (vertex.z < min.z) min.z = vertex.z;

            if (vertex.x > max.x) max.x = vertex.x;
            if (vertex.y > max.y) max.y = vertex.y;
            if (vertex.z > max.z) max.z = vertex.z;
        }

        // Apply translation to min and max coordinates
        min += glm::vec3(m_xPos, m_yPos, m_zPos);
        max += glm::vec3(m_xPos, m_yPos, m_zPos);

        // Apply offset to min and max coordinates
        min -= glm::vec3(offset);
        max += glm::vec3(offset);
    }

    void moveHitbox(float x, float y, float z) {
        min.x += x;
        max.x += x;
        m_xPos += x;

        min.y += y;
        max.y += y;
        m_yPos += y;

        min.z += z;
        max.z += z;
        m_zPos += z;

        // Update camera position
        if (mCamera) {
            mCamera->mEye.x += x;
            mCamera->mEye.y += y;
            mCamera->mEye.z += z;
        }
    }

    void setHitboxPosition(float x, float y, float z) {
        min.x += (x - min.x);
        min.y += (y - min.y);
        min.z += (z - min.z);

        max.x += (x - max.x);
        max.y += (y - max.y);
        max.z += (z - max.z);

        m_xPos = x;
        m_yPos = y;
        m_zPos = z;

        // Update camera position
        if (mCamera) {
            mCamera->mEye.x = x;
            mCamera->mEye.y = y;
            mCamera->mEye.z = z;
        }
    }

    bool isPointInside(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z;
    }

    glm::vec3 CalculatePenetrationDepth(const baseHitbox& box1, const baseHitbox& box2) {
        glm::vec3 penetrationDepth(0.0f);

        if (box1.max.x > box2.min.x && box1.min.x < box2.max.x) {
            penetrationDepth.x = std::min(box1.max.x - box2.min.x, box2.max.x - box1.min.x);
        }
        if (box1.max.y > box2.min.y && box1.min.y < box2.max.y) {
            penetrationDepth.y = std::min(box1.max.y - box2.min.y, box2.max.y - box1.min.y);
        }
        if (box1.max.z > box2.min.z && box1.min.z < box2.max.z) {
            penetrationDepth.z = std::min(box1.max.z - box2.min.z, box2.max.z - box1.min.z);
        }

        return penetrationDepth;
    }

    bool isPointInsidePositiveOnly(const glm::vec3& point, const baseHitbox& positiveHitbox, const std::vector<baseHitbox*>& negativeHitboxes) {
        if (!positiveHitbox.isPointInside(point)) {
            return false;
        }

        for (const auto& negBox : negativeHitboxes) {
            if (negBox->negative && negBox->isPointInside(point)) {
                return false;
            }
        }

        return true;
    }

    bool CheckCollision(const baseHitbox& box1, const baseHitbox& box2, const std::vector<baseHitbox*>& negativeHitboxes) {
        bool xOverlap = box1.max.x >= box2.min.x && box1.min.x <= box2.max.x;
        bool yOverlap = box1.max.y >= box2.min.y && box1.min.y <= box2.max.y;
        bool zOverlap = box1.max.z >= box2.min.z && box1.min.z <= box2.max.z;

        if (!(xOverlap && yOverlap && zOverlap)) {
            return false;
        }

        // Calculate the overlap region
        glm::vec3 overlapMin = glm::max(box1.min, box2.min);
        glm::vec3 overlapMax = glm::min(box1.max, box2.max);

        // Check if any point within the overlap region is inside the positive hitbox but outside the negative hitbox
        for (float x = overlapMin.x; x <= overlapMax.x; x += 0.01f) {
            for (float y = overlapMin.y; y <= overlapMax.y; y += 0.01f) {
                for (float z = overlapMin.z; z <= overlapMax.z; z += 0.01f) {
                    glm::vec3 point(x, y, z);
                    if (isPointInsidePositiveOnly(point, box1, negativeHitboxes) || isPointInsidePositiveOnly(point, box2, negativeHitboxes)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void ResolveCollision(baseHitbox& box1, const baseHitbox& box2, const std::vector<baseHitbox*>& negativeHitboxes) {
        // Check if collision should be ignored due to negative hitbox
        if (!CheckCollision(box1, box2, negativeHitboxes)) {
            std::cout << "Collision ignored due to negative hitbox." << std::endl;
            return;
        }

        glm::vec3 penetrationDepth = CalculatePenetrationDepth(box1, box2);

        // Move only box1 based on the smallest penetration depth
        if (penetrationDepth.x < penetrationDepth.y && penetrationDepth.x < penetrationDepth.z) {
            if (box1.m_xPos < box2.m_xPos) {
                box1.moveHitbox(-penetrationDepth.x, 0, 0);
            }
            else {
                box1.moveHitbox(penetrationDepth.x, 0, 0);
            }
        }
        else if (penetrationDepth.y < penetrationDepth.x && penetrationDepth.y < penetrationDepth.z) {
            if (box1.m_yPos < box2.m_yPos) {
                box1.moveHitbox(0, -penetrationDepth.y, 0);
            }
            else {
                box1.moveHitbox(0, penetrationDepth.y, 0);
            }
        }
        else {
            if (box1.m_zPos < box2.m_zPos) {
                box1.moveHitbox(0, 0, -penetrationDepth.z);
            }
            else {
                box1.moveHitbox(0, 0, penetrationDepth.z);
            }
        }
    }
};

void UpdateCameraHitbox(const Camera& camera, baseHitbox& CamHitbox) {
    glm::vec3 camPos = camera.mEye;
    CamHitbox.setHitboxPosition(camPos.x, camPos.y, camPos.z);
    CamHitbox.updateBounds(0.05f); // Update the bounds after setting the position
}

void UpdateCameraPosition(const baseHitbox& CamHitbox, Camera& camera) {
    camera.mEye = glm::vec3(CamHitbox.m_xPos, CamHitbox.m_yPos, CamHitbox.m_zPos);
}

struct baseEntity : baseObject {
    baseHitbox* mEntityHitbox;
    baseObject* mEntityModel;
    Application* mEntityApplication;

    float time = 0.0f;
    float step = 0.0000005f;

    void singleActionScript() {
        std::cout << "Single Action Script Called! \n";
    }
    void updatedActionScript() {
        glm::vec3 playerPos = mEntityApplication->mCamera.mEye;
        glm::vec3 curPos = glm::vec3(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
        time += step;
        if (time > 1.0f) time = 1.0f;
        glm::vec3 newPos = (curPos + time * (playerPos - curPos));
        mEntityModel->m_xPos = newPos.x;
        mEntityModel->m_yPos = newPos.y;
        mEntityModel->m_zPos = newPos.z;
    }
    
    void initializeEntity() {
        mEntityHitbox->setHitboxPosition(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
        singleActionScript();
    }
    
    void updateEntity() {
        updatedActionScript();
        mEntityHitbox->updateBounds(0.12);
    }
};

struct baseModel : baseObject {
    std::vector<baseObject*>modelObjects;
    std::vector<baseHitbox*>modelHitbox;

    int addObject(baseObject* obj) {
        modelObjects.push_back(obj);
        size_t pos = modelObjects.size() - 1;
        return pos;
    }

    void moveModel(float x, float y, float z) {
        for (auto& objects : modelObjects) {
            m_xPos += x;
            m_yPos += y;
            m_zPos += z;
        }
    }

    void setModelPosition(float x, float y, float z) {
        for (auto& objects : modelObjects) {
            objects->m_xPos += (x - m_xPos);
            objects->m_yPos += (y - m_yPos);
            objects->m_zPos += (z - m_zPos);
        }
    }
};

void showHitboxes(std::vector <baseObject*> hitboxes) {
    for (auto& hit : hitboxes) {

    }
}

// vvvvvvvvvvvvvvv Global Variables vvvvvvvvvvvvvvv
Application gApp;

std::vector<baseObject* >gGameObjects;
std::vector<baseModel* >gGameModels;
std::vector<baseHitbox*>gGameHitboxes;
std::vector<baseHitbox*> gGameNegativeHitboxes;

baseObject gMesh1;
baseObject gMesh2;

baseHitbox gHitbox1;
baseHitbox gEntityHitbox1;
baseHitbox gCamHitbox;

baseEntity gEntity1;

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
    textureHandles.push_back(loadBindlessTexture2D("assets/textures/hitboxtexture.png"));
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

    std::string vertexShaderSource = pipeline::LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = pipeline::LoadShaderAsString("./shaders/frag.glsl");

    gApp.mGraphicsPipelineShaderProgram = pipeline::CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(app->mGraphicsApplicationWindow, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ShowBaseModelProperties() {
    ImGui::Begin("Base Model Properties");

    for (size_t i = 0; i < gGameObjects.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            ImGui::InputFloat3("Position X", &gGameObjects[i]->m_xPos);
            ImGui::InputFloat3("Position Y", &gGameObjects[i]->m_yPos);
            ImGui::InputFloat3("Position Z", &gGameObjects[i]->m_zPos);
            ImGui::InputFloat3("Scale X", &gGameObjects[i]->m_uScaleX);
            ImGui::InputFloat3("Scale Y", &gGameObjects[i]->m_uScaleY);
            ImGui::InputFloat3("Scale Z", &gGameObjects[i]->m_uScaleZ);
            ImGui::InputFloat("Rotation", &gGameObjects[i]->m_uRotate);

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();

    ImGui::Begin("Base Hitboxes Properties");
    for (size_t i = 0; i < gGameHitboxes.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            ImGui::InputFloat3("Position X", &gGameHitboxes[i]->m_xPos);
            ImGui::InputFloat3("Position Y", &gGameHitboxes[i]->m_yPos);
            ImGui::InputFloat3("Position Z", &gGameHitboxes[i]->m_zPos);
            ImGui::InputFloat3("Scale X", &gGameHitboxes[i]->m_uScaleX);
            ImGui::InputFloat3("Scale Y", &gGameHitboxes[i]->m_uScaleY);
            ImGui::InputFloat3("Scale Z", &gGameHitboxes[i]->m_uScaleZ);
            ImGui::InputFloat("Rotation", &gGameHitboxes[i]->m_uRotate);

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
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
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
    }

    for (auto& mesh : gGameHitboxes) {
        mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
    }

    for (auto& mesh : gGameNegativeHitboxes) {
        mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
        mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
        mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
    }

    for (auto& meshObject : gGameModels) {
        for (auto& mesh : meshObject->modelObjects) {
            mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
            mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
            mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
        }
        for (auto& mesh : meshObject->modelHitbox) {
            mesh->model = glm::translate(glm::mat4(1.0f), glm::vec3(mesh->m_xPos, mesh->m_yPos, mesh->m_zPos));
            mesh->model = glm::rotate(mesh->model, glm::radians(mesh->m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
            mesh->model = glm::scale(mesh->model, glm::vec3(mesh->m_uScaleX, mesh->m_uScaleY, mesh->m_uScaleZ));
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

    UpdateCameraHitbox(gApp.mCamera, gCamHitbox);
    gHitbox1.updateBounds(0.12f); // Apply offset to the hitbox bounds

    bool isColliding = gCamHitbox.CheckCollision(gCamHitbox, gHitbox1, gGameNegativeHitboxes);

    if (isColliding) {
        gCamHitbox.ResolveCollision(gCamHitbox, gHitbox1, gGameNegativeHitboxes); // Resolve the collision
        UpdateCameraPosition(gCamHitbox, gApp.mCamera); // Update the camera position
    }

    gEntity1.updateEntity();
}

void Draw() {
    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    GLint u_ModelMatrixLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_ModelMatrix");
    GLint u_TextureHandleLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "u_TextureHandle");
    if (u_ModelMatrixLocation >= 0 && u_TextureHandleLocation >= 0) {
        for (auto& mesh : gGameObjects) {
            if (mesh->visible) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

                glBindVertexArray(mesh->mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        for (auto& mesh : gGameHitboxes) {
            if (mesh->visible) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

                glBindVertexArray(mesh->mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        for (auto& mesh : gGameNegativeHitboxes) {
            if (mesh->visible) {
                glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

                glBindVertexArray(mesh->mVertexArrayObject);
                glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        for (auto& meshObject : gGameModels) {
            for (auto& mesh : meshObject->modelObjects) {
                if (mesh->visible) {
                    glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                    glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

                    glBindVertexArray(mesh->mVertexArrayObject);
                    glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }
            for (auto& mesh : meshObject->modelHitbox) {
                if (mesh->visible) {
                    glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &mesh->model[0][0]);
                    glUniformHandleui64ARB(u_TextureHandleLocation, mesh->textureHandle);

                    glBindVertexArray(mesh->mVertexArrayObject);
                    glDrawElements(GL_TRIANGLES, mesh->m_indexBufferData.size(), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }
        }
    }
    else {
        std::cout << "Could not find u_ModelMatrix or u_TextureHandle, check spelling? \n";
        exit(EXIT_FAILURE);
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Lighting settings window
    ImGui::Begin("Lighting Settings");
    ImGui::ColorEdit3("Lighting Color", gLight.mLightCol2);
    gLight.mLightCol = { gLight.mLightCol2[0], gLight.mLightCol2[1], gLight.mLightCol2[2] };
    ImGui::InputFloat("Lighting X", &gLight.mLightPos.x, 0.01f, 5.0f, "%.3f");
    ImGui::InputFloat("Lighting Y", &gLight.mLightPos.y, 0.01f, 5.0f, "%.3f");
    ImGui::InputFloat("Lighting Z", &gLight.mLightPos.z, 0.01f, 5.0f, "%.3f");
    ImGui::End();

    // Base model properties window
    ShowBaseModelProperties();

    // Render ImGui
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
    objMesh2.loadFromOBJ("assets/models/4wayroom.obj");

    InitializeProgram(&gApp);

    loadTextures();

    gMesh1.m_vertexData = objMesh2.m_vertexData;
    gMesh1.m_indexBufferData = objMesh2.m_indexBufferData;

    gMesh2.m_vertexData = objMesh1.m_vertexData;
    gMesh2.m_indexBufferData = objMesh1.m_indexBufferData;
    gMesh2.moveObject(0.0f, 0.125f, 0.0f);

    gHitbox1.m_vertexData = gPreMade.m_cubeVertexData;
    gHitbox1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;

    gEntity1.mEntityHitbox = &gHitbox1;
    gEntity1.mEntityModel = &gMesh2;
    gEntity1.mEntityApplication = &gApp;
    gEntity1.initializeEntity();

    VertexSpecification(&gMesh1, 1);
    VertexSpecification(&gMesh2, 0);
    VertexSpecification(&gHitbox1, 2);

    gGameObjects.push_back(&gMesh1);
    gGameObjects.push_back(&gMesh2);

    gGameHitboxes.push_back(&gHitbox1);

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}