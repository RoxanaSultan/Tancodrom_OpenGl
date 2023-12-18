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

class Model
{
public:
    Model(const std::string& path)
    {
        loadModel(path);
    }

    void Draw(Shader shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(const std::string& path)
    {
        Mesh loadedMesh = OBJLoader::Load(path);
        Mesh processedMesh = processMesh(loadedMesh);
        meshes.push_back(processedMesh);
    }

    Mesh processMesh(const Mesh& loadedMesh)
    {

        Mesh processedMesh;
        // Process vertices
        for (const auto& vertex : loadedMesh.vertices)
        {
            Vertex processedVertex = vertex;
            // Perform any vertex modifications here
            processedMesh.vertices.push_back(processedVertex);
        }

        // Process indices (if needed)
        processedMesh.indices = loadedMesh.indices;

        // Setup the mesh (create VAO, VBO, etc.)
        processedMesh.setupMesh();

        return processedMesh;
    }
};