#ifndef EXVERTEXES_H
#define EXVERTEXES_H

#include "vector"
#include "glad/glad.h"

struct ExVertexes
{
  
    const std::vector<GLfloat> m_cubeVertexData{
        // Front face (sidedirt)
        // X       Y       Z       R    G     B      TEXTURE       NORMALS
        -0.25f, -0.25f,  0.25f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, // Bottom-left
         0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, // Bottom-right
        -0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,  0.0f, 0.5f,  0.0f, 0.0f, 1.0f, // Top-left
         0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // Top-right

        // Back face (sidedirt)
        -0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f, // Bottom-left
         0.25f, -0.25f, -0.25f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f, // Bottom-right
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f,  0.0f, 0.0f, -1.0f, // Top-left
         0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  1.0f, 0.5f,  0.0f, 0.0f, -1.0f, // Top-right

        // Left face (sidedirt)
        -0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,  0.5f, 0.0f,  -1.0f, 0.0f, 0.0f, // Bottom-left
        -0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, // Bottom-right
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f,  -1.0f, 0.0f, 0.0f, // Top-left
        -0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,  0.0f, 0.5f,  -1.0f, 0.0f, 0.0f, // Top-right

         // Right face (sidedirt)
         0.25f, -0.25f, -0.25f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // Bottom-left
         0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // Bottom-right
         0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.0f, 0.5f,  1.0f, 0.0f, 0.0f, // Top-left
         0.25f,  0.25f,  0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 0.5f,  1.0f, 0.0f, 0.0f, // Top-right

         // Top face (grass)
        -0.25f,  0.25f,  0.25f,  1.0f, 0.0f, 0.0f,  0.0f, 0.5f,  0.0f, 1.0f, 0.0f, // Bottom-left
         0.25f,  0.25f,  0.25f,  0.0f, 1.0f, 0.0f,  0.5f, 0.5f,  0.0f, 1.0f, 0.0f, // Bottom-right
        -0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f, // Top-left
         0.25f,  0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  0.0f, 1.0f, 0.0f, // Top-right

         // Bottom face (dirt)
        -0.25f, -0.25f,  0.25f,  1.0f, 0.0f, 0.0f,  0.5f, 0.5f,  0.0f, -1.0f, 0.0f, // Bottom-left
         0.25f, -0.25f,  0.25f,  0.0f, 1.0f, 0.0f,  1.0f, 0.5f,  0.0f, -1.0f, 0.0f, // Bottom-right
        -0.25f, -0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  0.5f, 1.0f,  0.0f, -1.0f, 0.0f, // Top-left
         0.25f, -0.25f, -0.25f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f  // Top-right
    };


    const std::vector<GLuint> m_cubeIndexBufferData{
        // Front face
        0, 1, 2, 2, 1, 3,
        // Right face
        4, 5, 6, 6, 5, 7,
        // Back face
        8, 9, 10, 10, 9, 11,
        // Left face
        12, 13, 14, 14, 13, 15,
        // Top face
        16, 17, 18, 18, 17, 19,
        // Bottom face
        20, 21, 22, 22, 21, 23
    }; 
};

#endif