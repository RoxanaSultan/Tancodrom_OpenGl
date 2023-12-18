#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Shader.h"
#include "Mesh.h"
#include "OBJLoader.h"

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

class TankModel
{
public:
    TankModel(const std::string& path);
    void Draw(Shader shader);

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path);

    Mesh processMesh(const Mesh& loadedMesh);
};