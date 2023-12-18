#include "TankModel.h"

TankModel::TankModel(const std::string& path)
{
    loadModel(path);
}

void TankModel::Draw(Shader shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void TankModel::loadModel(const std::string& path)
{
    Mesh loadedMesh = OBJLoader::Load(path);
    Mesh processedMesh = processMesh(loadedMesh);
    meshes.push_back(processedMesh);
}

Mesh TankModel::processMesh(const Mesh& loadedMesh)
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