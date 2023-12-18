#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <GLM.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // Add a constructor and methods to set up the mesh in OpenGL
};

class OBJLoader
{
public:
    static Mesh Load(const std::string& filePath);
};

// Usage:
// Mesh myMesh = OBJLoader::Load("path/to/your.obj");

