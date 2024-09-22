#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <Pipeline.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/intersect.hpp>

#include <algorithm>

namespace objects {
    #pragma region Vertex & OBJ Related
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
    #pragma endregion
    #pragma region Objects
        struct baseObject {
            #pragma region Base Object Variables
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
                    float m_uScaleZ = 1.0f;

                    float m_uRotateX = 0.0f;
                    float m_uRotateY = 0.0f;
                    float m_uRotateZ = 0.0f;

                    std::vector<GLfloat> m_vertexData;
                    std::vector<GLuint> m_indexBufferData;

                    unsigned int textureID;
                    int texArrayIndex;

                    std::string OBJFilePath;

            #pragma endregion
            #pragma region Base Object Methods
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
                        OBJFilePath = filename;

                        std::vector<GLuint> indices;
                        std::vector<Vertex> vertices = loadOBJWithAssimp(filename, indices);

                        m_vertexData.clear();
                        for (const auto& vertex : vertices) {
                            m_vertexData.insert(m_vertexData.end(), { vertex.x, vertex.y, vertex.z, vertex.r, vertex.g, vertex.b, vertex.u, vertex.v, vertex.nx, vertex.ny, vertex.nz });
                        }

                        m_indexBufferData = indices;
                    }
            #pragma endregion
        };

        struct baseHitbox : baseObject {
            #pragma region Base Hitbox Variables
            bool visible = true;
            bool negative = false;

            glm::vec3 min;
            glm::vec3 max;
            glm::vec3 mPreviousPos;
            Camera* mCamera; // Add a reference to the camera
            Application mApp;
    #pragma endregion
            #pragma region Base Hitbox Methods
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

                // Apply offset to min and max coordinates
                min -= glm::vec3(offset);
                max += glm::vec3(offset);

                // Define the corners of the AABB
                std::vector<glm::vec3> corners = {
                    min,
                    glm::vec3(min.x, min.y, max.z),
                    glm::vec3(min.x, max.y, min.z),
                    glm::vec3(min.x, max.y, max.z),
                    glm::vec3(max.x, min.y, min.z),
                    glm::vec3(max.x, min.y, max.z),
                    glm::vec3(max.x, max.y, min.z),
                    max
                };

                // Apply rotation to the corners
                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_uRotate), glm::vec3(0.0f, 1.0f, 0.0f));
                for (auto& corner : corners) {
                    glm::vec4 rotatedCorner = rotationMatrix * glm::vec4(corner, 1.0f);
                    corner = glm::vec3(rotatedCorner);
                }

                // Update min and max coordinates based on rotated corners
                min = glm::vec3(std::numeric_limits<float>::max());
                max = glm::vec3(std::numeric_limits<float>::lowest());
                for (const auto& corner : corners) {
                    if (corner.x < min.x) min.x = corner.x;
                    if (corner.y < min.y) min.y = corner.y;
                    if (corner.z < min.z) min.z = corner.z;

                    if (corner.x > max.x) max.x = corner.x;
                    if (corner.y > max.y) max.y = corner.y;
                    if (corner.z > max.z) max.z = corner.z;
                }

                // Apply translation to min and max coordinates
                min += glm::vec3(m_xPos, m_yPos, m_zPos);
                max += glm::vec3(m_xPos, m_yPos, m_zPos);
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

            glm::vec3 CalculateRayDirection(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
                float x = (2.0f * mouseX) / screenWidth - 1.0f;
                float y = 1.0f - (2.0f * mouseY) / screenHeight;
                float z = 1.0f;
                glm::vec3 rayNDC(x, y, z);
                glm::vec4 rayClip(rayNDC.x, rayNDC.y, -1.0f, 1.0f);
                glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
                rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
                glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayEye);
                rayWorld = glm::normalize(rayWorld);

                return rayWorld;
            }

            bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& aabbMin, const glm::vec3& aabbMax) {
                float tmin = (aabbMin.x - rayOrigin.x) / rayDir.x;
                float tmax = (aabbMax.x - rayOrigin.x) / rayDir.x;

                if (tmin > tmax) std::swap(tmin, tmax);

                float tymin = (aabbMin.y - rayOrigin.y) / rayDir.y;
                float tymax = (aabbMax.y - rayOrigin.y) / rayDir.y;

                if (tymin > tymax) std::swap(tymin, tymax);

                if ((tmin > tymax) || (tymin > tmax))
                    return false;

                if (tymin > tmin)
                    tmin = tymin;

                if (tymax < tmax)
                    tmax = tymax;

                float tzmin = (aabbMin.z - rayOrigin.z) / rayDir.z;
                float tzmax = (aabbMax.z - rayOrigin.z) / rayDir.z;

                if (tzmin > tzmax) std::swap(tzmin, tzmax);

                if ((tmin > tzmax) || (tzmin > tmax))
                    return false;

                if (tzmin > tmin)
                    tmin = tzmin;

                if (tzmax < tmax)
                    tmax = tzmax;

                return true;
            }

            bool CheckMouseClick(Camera cam, float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
                glm::vec3 aabbMin = min;
                glm::vec3 aabbMax = max;
                glm::vec3 rayDir = CalculateRayDirection(mouseX, mouseY, screenWidth, screenHeight, projectionMatrix, viewMatrix);
                return RayIntersectsAABB(cam.mEye, rayDir, aabbMin, aabbMax);
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

    #pragma endregion
        };

        #pragma region Camera Hitbox Methods
            void UpdateCameraHitbox(const Camera& camera, baseHitbox& CamHitbox) {
                glm::vec3 camPos = camera.mEye;
                CamHitbox.setHitboxPosition(camPos.x, camPos.y, camPos.z);
                CamHitbox.updateBounds(0.05f); // Update the bounds after setting the position
            }

            void UpdateCameraPosition(const baseHitbox& CamHitbox, Camera& camera) {
                camera.mEye = glm::vec3(CamHitbox.m_xPos, CamHitbox.m_yPos, CamHitbox.m_zPos);
            }
        #pragma endregion

        struct baseEntity : baseObject {
            #pragma region Base Entity Variables
            baseHitbox* mEntityHitbox;
            baseObject* mEntityModel;
            Application* mEntityApplication;

            float time = 0.0f;
            float step = 0.0000000005f;
    #pragma endregion
            #pragma region Base Entity Methods
                    virtual void updatedEntityPosition() {
                        mEntityModel->m_xPos = mEntityHitbox->m_xPos;
                        mEntityModel->m_yPos = mEntityHitbox->m_yPos;
                        mEntityModel->m_zPos = mEntityHitbox->m_zPos;
                    }

                    virtual void singleActionScript() {
                        std::cout << "Single Action Script Called! \n";
                    }

                    virtual void updatedActionScript() {
                        int nothing = 5;
                        nothing += nothing;
                    }

                    void initializeHitbox() {
                        mEntityHitbox->m_vertexData = mEntityModel->m_vertexData;
                        mEntityHitbox->updateBounds(0.12);
                    }

                    void initializeEntity() {
                        mEntityHitbox->setHitboxPosition(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
                        initializeHitbox();
                        singleActionScript();
                    }

                    void updateEntity() {
                        updatedActionScript();
                        mEntityHitbox->setHitboxPosition(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
                        mEntityHitbox->updateBounds(0.12);
                    }
            #pragma endregion
        };

        struct baseModel : baseObject {
            #pragma region Base Model Variables
                    std::vector<baseObject*>modelObjects;
                    std::vector<baseHitbox*>modelHitbox;
            #pragma endregion
            #pragma region Base Model Methods
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
            #pragma endregion
        };

        struct gameArtifact : objects::baseHitbox {
            Camera* PlayerCamera;
            int distRequirment = 1;
            virtual void onCollectFunction() {
                float plrDistance = glm::distance(PlayerCamera->mEye, glm::vec3(m_xPos, m_yPos, m_zPos));
                if (plrDistance <= distRequirment) {
                    std::cout << "Artifact Collected \n";
                }
            }
        };

        struct gameDoor : gameArtifact {
            std::vector<gameDoor*>* gameDoors;

            int DoorIndex;

            void onCollectFunction() override {
                float plrDistance = glm::distance(PlayerCamera->mEye, glm::vec3(m_xPos, m_yPos, m_zPos));
                if (plrDistance <= distRequirment) {

                    std::cout << "Door Opened \n";

                    for (auto& doors : *gameDoors) {
                        if (doors != this && doors->DoorIndex == DoorIndex) {
                            glm::vec3 camNewPos = glm::vec3(doors->m_xPos, doors->m_yPos, doors->m_zPos);
                            glm::vec3 direction = glm::vec3(cos(glm::radians(doors->m_uRotate)), 0, sin(glm::radians(doors->m_uRotate)));
                            camNewPos += direction * 1.5f; // Increase teleport distance

                            std::cout << "Teleporting to: " << camNewPos.x << ", " << camNewPos.y << ", " << camNewPos.z << "\n";
                            PlayerCamera->mEye = camNewPos;
                            camNewPos = glm::vec3(0, 0, 0);
                        }
                    }
                }
            }
        };

        void showHitboxes(std::vector <baseObject*> hitboxes) {
            for (auto& hit : hitboxes) {

            }
        }
    #pragma endregion
}
#endif