#include "OBJLoader.h"

MeshLoader OBJLoader::Load(const std::string& filePath)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    MeshLoader mesh;

    std::ifstream file(filePath);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            positions.push_back(vertex);
        } else if (prefix == "vt")
        {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        } else if (prefix == "vn")
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (prefix == "f")
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            char slash;

            for (int i = 0; i < 3; i++)
            {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
                vertexIndices.push_back(vertexIndex[i]);
                uvIndices.push_back(uvIndex[i]);
                normalIndices.push_back(normalIndex[i]);
            }
        }
    }

    // Process and create the mesh data
    // Note: OBJ indices are 1-based, so you need to subtract 1 when indexing your arrays
    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {
        Vertex vertex;
        vertex.position = positions[vertexIndices[i] - 1];
        vertex.texCoords = texCoords[uvIndices[i] - 1];
        vertex.normal = normals[normalIndices[i] - 1];
        mesh.vertices.push_back(vertex);
    }

    return mesh;
}