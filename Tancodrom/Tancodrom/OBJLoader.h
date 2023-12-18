#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <GLM.hpp>

#include "Mesh.h"

//struct Vertex
//{
//    // position
//    glm::vec3 Position;
//    // normal
//    glm::vec3 Normal;
//    // texCoords
//    glm::vec2 TexCoords;
//};

class OBJLoader
{
public:
    static Mesh Load(const std::string& filePath);
};

// Usage:
// Mesh myMesh = OBJLoader::Load("path/to/your.obj");

