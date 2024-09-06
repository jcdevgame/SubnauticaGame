#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <Pipeline.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace objects {
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
        float m_uScaleZ = 1.0f;

        std::vector<GLfloat> m_vertexData;
        std::vector<GLuint> m_indexBufferData;

        unsigned int textureID;
        int texArrayIndex;

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
        float step = 0.0000000005f;

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
            mEntityHitbox->updateBounds(0.12);
            mEntityHitbox->setHitboxPosition(mEntityModel->m_xPos, mEntityModel->m_yPos, mEntityModel->m_zPos);
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
}
#endif
