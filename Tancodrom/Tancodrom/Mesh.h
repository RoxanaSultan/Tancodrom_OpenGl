#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Shader.h"

struct Vertex 
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
};

struct Texture 
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh 
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void Draw(Shader shader);

private:
    unsigned int VBO, EBO;

    void setupMesh();
};
