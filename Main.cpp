#pragma region Includes
#include "Pipeline.h"
#include "objects.hpp"
#include "player.hpp"
#include <enet/enet.h>
#include <rooms.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

Application gApp;
player gLocalPlayer;
#pragma endregion
#pragma region Custom Objects
struct cockroachEntity : objects::baseEntity {
    float time = 0.0f;
    float step = 0.00025f;
    float playerDist;
    bool attacking = true;

    float homePointDist;
    glm::vec3 homepoint = glm::vec3(0, 0, 0);

    float getPlayerDist() {
        glm::vec3 playerPosition = gLocalPlayer.m_PlayerCamera.mEye;
        glm::vec3 currentPosition = glm::vec3(m_xPos, m_yPos, m_zPos);
        float plrDistance = glm::distance(playerPosition, currentPosition);
        return plrDistance;
    }

    float getHomepointDist() {
        glm::vec3 playerPosition = homepoint;
        glm::vec3 currentPosition = glm::vec3(m_xPos, m_yPos, m_zPos);
        float ointDist = glm::distance(playerPosition, currentPosition);
        return ointDist;
    }

    void singleActionScript() override {
        std::cout << "Single Action Script Called! \n";
    }

    void gobacktohomepoint() {
        glm::vec3 playerPos = homepoint;
        glm::vec3 curPos = glm::vec3(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
        time += step * gApp.deltaTime;
        if (time > 1.0f) time = 1.0f;
        glm::vec3 newPos = (curPos + time * (playerPos - curPos));
        mEntityModel->m_xPos = newPos.x;
        mEntityModel->m_yPos = newPos.y;
        mEntityModel->m_zPos = newPos.z;

        // Check if the cockroach has reached the homepoint
        if (glm::distance(newPos, homepoint) <= 0.25f) { // Adjust the distance threshold as needed
            attacking = true;
            time = 0.0f; // Reset time for the next movement
        }
    }

    void updatedActionScript() override {
        float distance = getPlayerDist();

        if (distance <= 6.0f && attacking) {
            glm::vec3 playerPos = gLocalPlayer.m_PlayerCamera.mEye;
            glm::vec3 curPos = glm::vec3(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
            time += step * gApp.deltaTime;
            if (time > 1.0f) time = 1.0f;
            glm::vec3 newPos = (curPos + time * (playerPos - curPos));
            mEntityModel->m_xPos = newPos.x;
            mEntityModel->m_yPos = newPos.y;
            mEntityModel->m_zPos = newPos.z;
        }
        else if (!attacking) {
            gobacktohomepoint();
        }
    }
};

struct oxygenArtifact : objects::gameArtifact {
    void onCollectFunction() override {
        std::cout << "Refilled Oxygen";
        gLocalPlayer.playerCurrentOxygen = 100;
        m_xPos = 100000000.0f;
    }
};

struct nuclearArtifact : objects::gameArtifact {
    void onCollectFunction() override {
        std::cout << "Collected Nuclear Core! \n";
    }
};
#pragma endregion
#pragma region Global Variables
std::vector<objects::baseObject* >gGameObjects;
std::vector<objects::baseHitbox*>gGameHitboxes;
std::vector<objects::baseHitbox*>gGameNegativeHitboxes;
std::vector<objects::baseHitbox*>gWallsAndFloors;
std::vector<objects::baseHitbox*>gWalls;
std::vector<objects::baseHitbox*>gFloors;
std::vector<objects::gameArtifact*>gRoomArtifacts;
std::vector<objects::gameArtifact*>gArtifacts;
std::vector<objects::gameDoor*>gGameDoors;
std::vector<objects::gameDoor*>gRoomDoors;

objects::baseObject gMesh1;
objects::baseObject gMesh2;
objects::baseHitbox gWall1;
objects::baseHitbox gWall2;
objects::baseHitbox gWall3;
objects::baseHitbox gWall4;
objects::baseHitbox gFloor1;
objects::baseHitbox gFloor2;
objects::baseHitbox gCockroachHitbox1;
objects::baseHitbox gCamHitbox;

cockroachEntity cockroachEntity1;

objects::gameArtifact testArtifact;
objects::gameDoor l_Door1;
objects::gameDoor l_Door2;

oxygenArtifact oxygenArtifact1;
nuclearArtifact gNuclearCore;

glm::mat4 perspective;
ExVertexes gPreMade;
Lighting gLight;

roomLoading::submarineRoom room;

double gDeltaTime = 0.0;
float gSpeed = 5;
char gDocumentCreature;
bool gDocumentingCreature = false;

bool documentedCreatures[2]{
    false, // Cockroach [0]
    false  // Next Entity [1]
};

GLuint gTexArray;
const char* gTexturePaths[4] = {
    "assets/textures/wine.jpg",
    "assets/textures/GrassTextureTest.jpg",
    "assets/textures/hitboxtexture.jpg",
    "assets/textures/TexturePlz.jpg",
};
#pragma endregion
#pragma region Graphics Pipeline Initialization
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
#pragma endregion
#pragma region Application Initialization
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(app->mGraphicsApplicationWindow, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
#pragma endregion
#pragma region Object Initialization
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
#pragma endregion
#pragma region ImGUI Windows
void ShowPlayerProperties() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 60), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(320, 60), ImGuiCond_Always);
    ImGui::Begin("Health", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    float healthPercentage = gLocalPlayer.playerCurrentHealth / gLocalPlayer.playerMaxHealth;

    // Set the size of the health bar
    ImVec2 barSize = ImVec2(300, 20);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.00, 0.98, 0.65, 1.0));
    ImGui::ProgressBar(healthPercentage, barSize, "");
    ImGui::PopStyleColor();
    ImGui::End();

    gLocalPlayer.playerCurrentOxygen -= 0.0001;
    if (gLocalPlayer.playerCurrentOxygen <= 0) { gLocalPlayer.playerCurrentHealth -= 0.005; }

    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 120), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(320, 60), ImGuiCond_Always);
    ImGui::Begin("Oxygen", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    float oxygenPercentage = gLocalPlayer.playerCurrentOxygen / gLocalPlayer.playerMaxOxygen;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.00, 1.00, 1.00, 1.0));
    ImGui::ProgressBar(oxygenPercentage, barSize, "");
    ImGui::PopStyleColor();
    ImGui::End();
}

bool showDocumentEntityWindow = true;
bool showDocumentedEntityWindow = false;

void ShowDocumentEntityProperty() {
    if (gDocumentCreature == 'C' && showDocumentEntityWindow) {
        gLocalPlayer.m_PlayerCamera.enabled = false;
        glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        ImGui::Begin("Documented Creature:", nullptr, ImGuiWindowFlags_NoCollapse);

        // Add the text
        ImGui::Text("Shrimp thingy");
        ImGui::TextDisabled("The Shrimp thingy is an aggressive creature,");
        ImGui::TextDisabled("that does quick attacks and runs away.");
        ImGui::TextDisabled("");
        ImGui::TextDisabled("");
        ImGui::TextDisabled("TAB to open Documented Creatures tab");

        if (ImGui::Button("Document")) {
            gDocumentingCreature = false;
            gDocumentCreature = NULL;

            gLocalPlayer.m_PlayerCamera.enabled = true;
            glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
            glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Hide the window
            showDocumentEntityWindow = false;
            documentedCreatures[0] = true;
        }

        ImGui::End();
    }
}

void showDocumentedCreatures() {
    if (showDocumentedEntityWindow)
    {
        gLocalPlayer.m_PlayerCamera.enabled = false;
        glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ImGui::Begin("Documented Creatures:", nullptr, ImGuiWindowFlags_NoCollapse);

        if (documentedCreatures[0]) {
            ImGui::Text("Shrimp thingy");
        }

        ImGui::End();
    }
}

void wallEditing() {
    ImGui::Begin("Room Wall Editing");

    for (size_t i = 0; i < gWalls.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            // Position X
            ImGui::Text("Position X");
            ImGui::SameLine();
            if (ImGui::Button("-##PosX")) {
                gWalls[i]->m_xPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gWalls[i]->m_xPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosX")) {
                gWalls[i]->m_xPos += 1.0f;
            }

            // Position Y
            ImGui::Text("Position Y");
            ImGui::SameLine();
            if (ImGui::Button("-##PosY")) {
                gWalls[i]->m_yPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gWalls[i]->m_yPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosY")) {
                gWalls[i]->m_yPos += 1.0f;
            }

            // Position Z
            ImGui::Text("Position Z");
            ImGui::SameLine();
            if (ImGui::Button("-##PosZ")) {
                gWalls[i]->m_zPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gWalls[i]->m_zPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosZ")) {
                gWalls[i]->m_zPos += 1.0f;
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            if (ImGui::Button("-##Rotation")) {
                gWalls[i]->m_uRotate -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gWalls[i]->m_uRotate);
            ImGui::SameLine();
            if (ImGui::Button("+##Rotation")) {
                gWalls[i]->m_uRotate += 15.0f;
            }
            // Texture Array Index
            ImGui::InputInt("Texture Array Index", &gWalls[i]->texArrayIndex);

            if (ImGui::Button("Save To Room")) {
                room.RoomWalls.push_back(gWalls[i]);
                roomLoading::saveToJson(room, "roomData.json");
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void floorEditing() {
    ImGui::Begin("Room Floor Editing");

    for (size_t i = 0; i < gFloors.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            // Position X
            ImGui::Text("Position X");
            ImGui::SameLine();
            if (ImGui::Button("-##PosX")) {
                gFloors[i]->m_xPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gFloors[i]->m_xPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosX")) {
                gFloors[i]->m_xPos += 1.0f;
            }

            // Position Y
            ImGui::Text("Position Y");
            ImGui::SameLine();
            if (ImGui::Button("-##PosY")) {
                gFloors[i]->m_yPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gFloors[i]->m_yPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosY")) {
                gFloors[i]->m_yPos += 1.0f;
            }

            // Position Z
            ImGui::Text("Position Z");
            ImGui::SameLine();
            if (ImGui::Button("-##PosZ")) {
                gFloors[i]->m_zPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gFloors[i]->m_zPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosZ")) {
                gFloors[i]->m_zPos += 1.0f;
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            if (ImGui::Button("-##Rotation")) {
                gFloors[i]->m_uRotate -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gFloors[i]->m_uRotate);
            ImGui::SameLine();
            if (ImGui::Button("+##Rotation")) {
                gFloors[i]->m_uRotate += 15.0f;
            }
            // Texture Array Index
            ImGui::InputInt("Texture Array Index", &gFloors[i]->texArrayIndex);

            if (ImGui::Button("Save To Room")) {
                room.RoomFloors.push_back(gFloors[i]);
                roomLoading::saveToJson(room, "roomData.json");
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void ArtifactEditing() {
    ImGui::Begin("Room Artifact Editing");

    for (size_t i = 0; i < gRoomArtifacts.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            // Position X
            ImGui::Text("Position X");
            ImGui::SameLine();
            if (ImGui::Button("-##PosX")) {
                gRoomArtifacts[i]->m_xPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomArtifacts[i]->m_xPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosX")) {
                gRoomArtifacts[i]->m_xPos += 1.0f;
            }

            // Position Y
            ImGui::Text("Position Y");
            ImGui::SameLine();
            if (ImGui::Button("-##PosY")) {
                gRoomArtifacts[i]->m_yPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomArtifacts[i]->m_yPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosY")) {
                gRoomArtifacts[i]->m_yPos += 1.0f;
            }

            // Position Z
            ImGui::Text("Position Z");
            ImGui::SameLine();
            if (ImGui::Button("-##PosZ")) {
                gRoomArtifacts[i]->m_zPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomArtifacts[i]->m_zPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosZ")) {
                gRoomArtifacts[i]->m_zPos += 1.0f;
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            if (ImGui::Button("-##Rotation")) {
                gRoomArtifacts[i]->m_uRotate -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomArtifacts[i]->m_uRotate);
            ImGui::SameLine();
            if (ImGui::Button("+##Rotation")) {
                gRoomArtifacts[i]->m_uRotate += 15.0f;
            }
            // Texture Array Index
            ImGui::InputInt("Texture Array Index", &gRoomArtifacts[i]->texArrayIndex);

            if (ImGui::Button("Save To Room")) {
                room.RoomArtifacts.push_back(gRoomArtifacts[i]);
                roomLoading::saveToJson(room, "roomData.json");
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void DoorEditing() {
    ImGui::Begin("Room Door Editing");

    for (size_t i = 0; i < gRoomDoors.size(); ++i) {
        ImGui::PushID(static_cast<int>(i)); // Ensure unique IDs for each model

        if (ImGui::TreeNode(("Model " + std::to_string(i)).c_str())) {
            // Position X
            ImGui::Text("Position X");
            ImGui::SameLine();
            if (ImGui::Button("-##PosX")) {
                gRoomDoors[i]->m_xPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomDoors[i]->m_xPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosX")) {
                gRoomDoors[i]->m_xPos += 1.0f;
            }

            // Position Y
            ImGui::Text("Position Y");
            ImGui::SameLine();
            if (ImGui::Button("-##PosY")) {
                gRoomDoors[i]->m_yPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomDoors[i]->m_yPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosY")) {
                gRoomDoors[i]->m_yPos += 1.0f;
            }

            // Position Z
            ImGui::Text("Position Z");
            ImGui::SameLine();
            if (ImGui::Button("-##PosZ")) {
                gRoomDoors[i]->m_zPos -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomDoors[i]->m_zPos);
            ImGui::SameLine();
            if (ImGui::Button("+##PosZ")) {
                gRoomDoors[i]->m_zPos += 1.0f;
            }

            ImGui::Text("Rotation");
            ImGui::SameLine();
            if (ImGui::Button("-##Rotation")) {
                gRoomDoors[i]->m_uRotate -= 1.0f;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f", gRoomDoors[i]->m_uRotate);
            ImGui::SameLine();
            if (ImGui::Button("+##Rotation")) {
                gRoomDoors[i]->m_uRotate += 15.0f;
            }
            // Texture Array Index
            ImGui::InputInt("Texture Array Index", &gRoomDoors[i]->texArrayIndex);

            if (ImGui::Button("Save To Room")) {
                room.RoomDoors.push_back(gRoomDoors[i]);
                roomLoading::saveToJson(room, "roomData.json");
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

#pragma endregion
#pragma region User Input
bool db = false;
bool db2 = false;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Pass the input event to ImGui first
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    // Handle backspace key
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        // Process backspace key event
        io.KeysDown[GLFW_KEY_BACKSPACE] = true;
    }
    else if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE) {
        io.KeysDown[GLFW_KEY_BACKSPACE] = false;
    }

    // Your existing key handling logic
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (!db2) {
            gLocalPlayer.m_PlayerCamera.enabled = false;
            glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            db = true;
        }
        else if (db2) {
            db = false;
            gLocalPlayer.m_PlayerCamera.enabled = true;
            glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
            glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    else if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        if (!db) {
            showDocumentedEntityWindow = true;
            db = true;
        }
        else if (db) {
            showDocumentedEntityWindow = false;
            db = false;
            gLocalPlayer.m_PlayerCamera.enabled = true;
            glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
            glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
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
    xpos = xpos * 0.15;
    ypos = ypos * 0.15;
    gLocalPlayer.m_PlayerCamera.MouseLook(xpos, ypos);

    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_W) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveForward(gSpeed * gApp.deltaTime);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_S) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveBackward(gSpeed * gApp.deltaTime);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_A) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveLeft(gSpeed * gApp.deltaTime);
    }
    if (glfwGetKey(gApp.mGraphicsApplicationWindow, GLFW_KEY_D) == GLFW_PRESS) {
        gLocalPlayer.m_PlayerCamera.MoveRight(gSpeed * gApp.deltaTime);
    }

    glfwSetKeyCallback(gApp.mGraphicsApplicationWindow, KeyCallback);
}

void handleWallsAndFloorsCollision(objects::baseHitbox& collider, objects::baseEntity& ent, bool CamCollision = false) {
    for (auto& hitboxes : gWallsAndFloors) {
        hitboxes->updateBounds(0.12f);
        bool collision = collider.CheckCollision(collider, *hitboxes, gGameNegativeHitboxes);

        if (collision) {
            if (CamCollision)
            {
                gCamHitbox.ResolveCollision(gCamHitbox, *hitboxes, gGameNegativeHitboxes);
                UpdateCameraPosition(gCamHitbox, gLocalPlayer.m_PlayerCamera); // Update the camera position
            }
            else {
                collider.ResolveCollision(collider, *hitboxes, gGameNegativeHitboxes);
                ent.updatedEntityPosition();
            }
        }
    }
}

void handleArtifactCollection() {
    for (auto& artifact : gArtifacts) {
        bool artifactClicked = artifact->CheckMouseClick(gLocalPlayer.m_PlayerCamera, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2, gApp.mScreenWidth, gApp.mScreenHeight, perspective, gLocalPlayer.m_PlayerCamera.GetViewMatrix());
        if (artifactClicked) {
            artifact->onCollectFunction();
        }
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Call ImGui's mouse button callback
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (!gDocumentingCreature) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            handleArtifactCollection();

            bool isHoveringOver1 = gCockroachHitbox1.CheckMouseClick(gLocalPlayer.m_PlayerCamera, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2, gApp.mScreenWidth, gApp.mScreenHeight, perspective, gLocalPlayer.m_PlayerCamera.GetViewMatrix());
            if (isHoveringOver1) {
                if (!documentedCreatures[0]) {
                    gDocumentCreature = 'C';
                    showDocumentEntityWindow = true; // Show the window
                }
                else {
                    std::cout << "Creature Documentation Manager: Creature[0] already documented \n";
                }
            }
            cockroachEntity1.updateEntity();
        }
    }
}
#pragma endregion
#pragma region Draw and Perspective
void PreDraw() {
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
    glClearColor(0.0f, 1.f, 1.f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    // Update the cockroach entity's position
    cockroachEntity1.updatedActionScript();

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

    perspective = glm::perspective(glm::radians(45.0f), (float)gApp.mScreenWidth / (float)gApp.mScreenHeight, 0.1f, 50.0f);
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

    UpdateCameraHitbox(gLocalPlayer.m_PlayerCamera, gCamHitbox);
    gCockroachHitbox1.updateBounds(0.12f); // Apply offset to the hitbox bounds

    handleWallsAndFloorsCollision(gCamHitbox, gLocalPlayer, true);
    handleWallsAndFloorsCollision(gCockroachHitbox1, cockroachEntity1, false);

    bool isColliding1 = gCockroachHitbox1.CheckCollision(gCockroachHitbox1, gCamHitbox, gGameNegativeHitboxes);

    if (isColliding1) {
        gCamHitbox.ResolveCollision(gCamHitbox, gCockroachHitbox1, gGameNegativeHitboxes); // Resolve the collision
        UpdateCameraPosition(gCamHitbox, gLocalPlayer.m_PlayerCamera); // Update the camera position

        if (cockroachEntity1.attacking) {
            cockroachEntity1.attacking = false;
            gLocalPlayer.playerCurrentHealth -= 10;
        }
    }

    glfwSetMouseButtonCallback(gApp.mGraphicsApplicationWindow, MouseButtonCallback);
    cockroachEntity1.updateEntity();
}

void RenderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ShowPlayerProperties();
    ShowDocumentEntityProperty();
    showDocumentedCreatures();
    wallEditing();
    floorEditing();
    ArtifactEditing();
    DoorEditing();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    RenderImGui();

    glUseProgram(0);
}
#pragma endregion
#pragma region Main Functions
void MainLoop()
{
    glfwSetCursorPos(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
    glfwSetInputMode(gApp.mGraphicsApplicationWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double lastFrame = glfwGetTime();

    while (!gApp.quitApplication)
    {
        gApp.calculateDeltaTime();
        Input();
        PreDraw();
        Draw();
        glfwSwapBuffers(gApp.mGraphicsApplicationWindow);
        glfwPollEvents();
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

void LoadRoomsIntoGame() {
    roomLoading::loadFromJson(room, "roomData.json");
    for (auto& wall : room.RoomWalls) {
        wall->loadFromOBJ(wall->OBJFilePath);

        std::cout << "Wall Position: " << wall->m_xPos << ", " << wall->m_yPos << ", " << wall->m_zPos << "\n";

        VertexSpecification(wall, wall->texArrayIndex);
        gGameHitboxes.push_back(wall);
        gWalls.push_back(wall);
        gWallsAndFloors.push_back(wall);
    }
    for (auto& floor : room.RoomFloors) {
        floor->loadFromOBJ(floor->OBJFilePath);

        std::cout << "Floor Position: " << floor->m_xPos << ", " << floor->m_yPos << ", " << floor->m_zPos << "\n";

        VertexSpecification(floor, floor->texArrayIndex);
        gGameHitboxes.push_back(floor);
        gFloors.push_back(floor);
        gWallsAndFloors.push_back(floor);
    }
    for (auto& artifact : room.RoomArtifacts) {
        if(!artifact->OBJFilePath.empty())
        {
            artifact->loadFromOBJ(artifact->OBJFilePath);
        }
        else {
            artifact->m_vertexData = gPreMade.m_cubeVertexData;
            artifact->m_indexBufferData = gPreMade.m_cubeIndexBufferData;
        }
        std::cout << "Artifact Position: " << artifact->m_xPos << ", " << artifact->m_yPos << ", " << artifact->m_zPos << "\n";

        VertexSpecification(artifact, artifact->texArrayIndex);
        gGameHitboxes.push_back(artifact);
        gRoomArtifacts.push_back(artifact);
        gArtifacts.push_back(artifact);
        gWallsAndFloors.push_back(artifact);
    }
    for (auto& door : room.RoomDoors) {
        if (!door->OBJFilePath.empty())
        {
            door->loadFromOBJ(door->OBJFilePath);
        }
        else {
            door->m_vertexData = gPreMade.m_cubeVertexData;
            door->m_indexBufferData = gPreMade.m_cubeIndexBufferData;
        }
        std::cout << "Door Position: " << door->m_xPos << ", " << door->m_yPos << ", " << door->m_zPos << "\n";

        VertexSpecification(door, door->texArrayIndex);
        door->gameDoors = &gGameDoors;
        door->PlayerCamera = &gLocalPlayer.m_PlayerCamera;
        gGameHitboxes.push_back(door);
        gRoomDoors.push_back(door);
        gGameDoors.push_back(door);
        gArtifacts.push_back(door);
        gWallsAndFloors.push_back(door);
    }
}

void clientActions() {
    InitializeProgram(&gApp);
    loadTextureArray2D(gTexturePaths, 4, &gTexArray);

    l_Door1.m_vertexData = gPreMade.m_cubeVertexData;
    l_Door1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;
    l_Door1.DoorIndex = 0;
    l_Door1.gameDoors = &gGameDoors;
    l_Door1.PlayerCamera = &gLocalPlayer.m_PlayerCamera;
    l_Door1.m_xPos += 5;

    l_Door2.m_vertexData = gPreMade.m_cubeVertexData;
    l_Door2.m_indexBufferData = gPreMade.m_cubeIndexBufferData;
    l_Door2.DoorIndex = 1;
    l_Door2.gameDoors = &gGameDoors;
    l_Door2.PlayerCamera = &gLocalPlayer.m_PlayerCamera;
    l_Door2.m_xPos -= 5;
    l_Door2.m_uRotate = 180;

    gMesh1.m_vertexData = gPreMade.m_cubeVertexData;
    gMesh1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;

    gCockroachHitbox1.m_vertexData = gPreMade.m_cubeVertexData;
    gCockroachHitbox1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;

    testArtifact.m_vertexData = gPreMade.m_cubeVertexData;
    testArtifact.m_indexBufferData = gPreMade.m_cubeIndexBufferData;
    testArtifact.m_yPos += 3;

    oxygenArtifact1.m_vertexData = gPreMade.m_cubeVertexData;
    oxygenArtifact1.m_indexBufferData = gPreMade.m_cubeIndexBufferData;
    oxygenArtifact1.m_yPos += 3;
    oxygenArtifact1.m_xPos += 3;

    gNuclearCore.loadFromOBJ("assets/models/NuclearCoreTest.obj");
    gNuclearCore.m_yPos += 5;
    gNuclearCore.m_xPos -= 5;

    /*
    gWall1.loadFromOBJ("assets/models/WallPrototypeTrust.obj");
    gWall1.m_xPos = -13.0f;
    gWall1.m_uRotate = 90;
    gWall2.loadFromOBJ("assets/models/WallPrototypeTrust.obj");
    gWall2.m_xPos = -14.0f;

    gWall3.loadFromOBJ("assets/models/WallPrototypeTrust.obj");
    gWall3.m_xPos += 5;
    gWall4.loadFromOBJ("assets/models/WallPrototypeTrust.obj");
    gWall4.m_xPos += 5;
    gWall4.m_zPos += 2;
    */
    gFloor1.loadFromOBJ("assets/models/FloorPrototypeTrust.obj");
    gFloor2.loadFromOBJ("assets/models/FloorPrototypeTrust.obj");

    LoadRoomsIntoGame();

    gMesh2.loadFromOBJ("assets/models/wine.obj");

    cockroachEntity1.mEntityModel = &gMesh2;
    cockroachEntity1.mEntityHitbox = &gCockroachHitbox1;
    cockroachEntity1.mEntityApplication = &gApp;
    cockroachEntity1.initializeEntity();

    VertexSpecification(&gMesh1, 1);
    VertexSpecification(&gMesh2, 0);
    VertexSpecification(&gCockroachHitbox1, 0);
    //VertexSpecification(&gFloor1, 3);
    //VertexSpecification(&gFloor2, 3);
    //VertexSpecification(&gWall3, 3);
    //VertexSpecification(&gWall4, 3);
    VertexSpecification(&testArtifact, 2);
    VertexSpecification(&oxygenArtifact1, 3);
    VertexSpecification(&gNuclearCore, 1);
    VertexSpecification(&l_Door1, 3);
    VertexSpecification(&l_Door2, 0);

    gGameObjects.push_back(&gMesh1);
    gGameObjects.push_back(&gMesh2);
    gGameHitboxes.push_back(&gCockroachHitbox1);
    gGameHitboxes.push_back(&gFloor1);
    gGameHitboxes.push_back(&gFloor2);
    gGameHitboxes.push_back(&gWall3);
    gGameHitboxes.push_back(&gWall4);
    gGameHitboxes.push_back(&testArtifact);
    gGameHitboxes.push_back(&oxygenArtifact1);
    gGameHitboxes.push_back(&gNuclearCore);
    gGameHitboxes.push_back(&l_Door1);
    gGameHitboxes.push_back(&l_Door2);

    gWallsAndFloors.push_back(&gFloor1);
    gWallsAndFloors.push_back(&gFloor2);
    gWallsAndFloors.push_back(&gWall3);
    gWallsAndFloors.push_back(&gWall4);
    gWallsAndFloors.push_back(&testArtifact);
    gWallsAndFloors.push_back(&oxygenArtifact1);
    gWallsAndFloors.push_back(&gNuclearCore);
    gWallsAndFloors.push_back(&l_Door1);
    gWallsAndFloors.push_back(&l_Door2);

    gRoomArtifacts.push_back(&oxygenArtifact1);
    gRoomDoors.push_back(&l_Door1);

    gArtifacts.push_back(&testArtifact);
    gArtifacts.push_back(&oxygenArtifact1);
    gArtifacts.push_back(&gNuclearCore);
    gArtifacts.push_back(&l_Door1);
    gArtifacts.push_back(&l_Door2);

    gGameDoors.push_back(&l_Door1);
    gGameDoors.push_back(&l_Door2);

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();
}

int main(int argc, char* args[]) {
    clientActions();

    return 0;
}
#pragma endregion