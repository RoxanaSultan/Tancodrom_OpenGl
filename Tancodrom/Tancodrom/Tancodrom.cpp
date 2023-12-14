#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

enum ECameraMovementType
{
    UNKNOWN,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
private:
    // Default camera values
    const float zNEAR = 0.1f;
    const float zFAR = 500.f;
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float FOV = 45.0f;
    glm::vec3 startPosition;

public:
    Camera(const int width, const int height, const glm::vec3& position)
    {
        startPosition = position;
        Set(width, height, position);
    }

    void Set(const int width, const int height, const glm::vec3& position)
    {
        this->isPerspective = true;
        this->yaw = YAW;
        this->pitch = PITCH;

        this->FoVy = FOV;
        this->width = width;
        this->height = height;
        this->zNear = zNEAR;
        this->zFar = zFAR;

        this->worldUp = glm::vec3(0, 1, 0);
        this->position = position;

        lastX = width / 2.0f;
        lastY = height / 2.0f;
        bFirstMouseMove = true;

        UpdateCameraVectors();
    }

    void Reset(const int width, const int height)
    {
        Set(width, height, startPosition);
    }

    void Reshape(int windowWidth, int windowHeight)
    {
        width = windowWidth;
        height = windowHeight;

        // define the viewport transformation
        glViewport(0, 0, windowWidth, windowHeight);
    }

    const glm::vec3 GetPosition() const
    {
        return position;
    }

    const glm::mat4 GetViewMatrix() const
    {
        // Returns the View Matrix
        return glm::lookAt(position, position + forward, up);
    }

    const glm::mat4 GetProjectionMatrix() const
    {
        glm::mat4 Proj = glm::mat4(1);
        if (isPerspective) {
            float aspectRatio = ((float)(width)) / height;
            Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
        }
        else {
            float scaleFactor = 2000.f;
            Proj = glm::ortho<float>(
                -width / scaleFactor, width / scaleFactor,
                -height / scaleFactor, height / scaleFactor, -zFar, zFar);
        }
        return Proj;
    }

    void ProcessKeyboard(ECameraMovementType direction, float deltaTime)
    {
        float velocity = (float)(cameraSpeedFactor * deltaTime);
        switch (direction) {
        case ECameraMovementType::FORWARD:
            position += forward * velocity;
            break;
        case ECameraMovementType::BACKWARD:
            position -= forward * velocity;
            break;
        case ECameraMovementType::LEFT:
            position -= right * velocity;
            break;
        case ECameraMovementType::RIGHT:
            position += right * velocity;
            break;
        case ECameraMovementType::UP:
            position += up * velocity;
            break;
        case ECameraMovementType::DOWN:
            position -= up * velocity;
            break;
        }
    }

    void MouseControl(float xPos, float yPos)
    {
        if (bFirstMouseMove) {
            lastX = xPos;
            lastY = yPos;
            bFirstMouseMove = false;
        }

        float xChange = xPos - lastX;
        float yChange = lastY - yPos;
        lastX = xPos;
        lastY = yPos;

        if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
            return;
        }
        xChange *= mouseSensitivity;
        yChange *= mouseSensitivity;

        ProcessMouseMovement(xChange, yChange);
    }

    void ProcessMouseScroll(float yOffset)
    {
        if (FoVy >= 1.0f && FoVy <= 90.0f) {
            FoVy -= yOffset;
        }
        if (FoVy <= 1.0f)
            FoVy = 1.0f;
        if (FoVy >= 90.0f)
            FoVy = 90.0f;
    }

private:
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
    {
        yaw += xOffset;
        pitch += yOffset;

        //std::cout << "yaw = " << yaw << std::endl;
        //std::cout << "pitch = " << pitch << std::endl;

        // Avem grijã sã nu ne dãm peste cap
        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        // Se modificã vectorii camerei pe baza unghiurilor Euler
        UpdateCameraVectors();
    }

    void UpdateCameraVectors()
    {
        // Calculate the new forward vector
        this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->forward.y = sin(glm::radians(pitch));
        this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->forward = glm::normalize(this->forward);
        // Also re-calculate the Right and Up vector
        right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = glm::normalize(glm::cross(right, forward));
    }

protected:
    const float cameraSpeedFactor = 2.5f;
    const float mouseSensitivity = 0.1f;

    // Perspective properties
    float zNear;
    float zFar;
    float FoVy;
    int width;
    int height;
    bool isPerspective;

    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;
    float pitch;

    bool bFirstMouseMove = true;
    float lastX = 0.f, lastY = 0.f;
};

class Shader
{
public:
    // constructor generates the shaderStencilTesting on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        Init(vertexPath, fragmentPath);
    }

    ~Shader()
    {
        glDeleteProgram(ID);
    }

    // activate the shaderStencilTesting
    // ------------------------------------------------------------------------
    void Use() const
    {
        glUseProgram(ID);
    }

    unsigned int GetID() const { return ID; }

    // MVP
    unsigned int loc_model_matrix;
    unsigned int loc_view_matrix;
    unsigned int loc_projection_matrix;

    // utility uniform functions
    void SetInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void SetFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void SetVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void SetVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void SetMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    void Init(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shaderStencilTesting
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");
        // shaderStencilTesting Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        CheckCompileErrors(ID, "PROGRAM");

        // 3. delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // utility function for checking shaderStencilTesting compilation/linking errors.
    // ------------------------------------------------------------------------
    void CheckCompileErrors(unsigned int shaderStencilTesting, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shaderStencilTesting, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shaderStencilTesting, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else
        {
            glGetProgramiv(shaderStencilTesting, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shaderStencilTesting, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
private:
    unsigned int ID;
};

Camera* pCamera = nullptr;

int main(int argc, char** argv)
{
    std::string strFullExeFileName = argv[0];
    std::string strExePath;
    const size_t last_slash_idx = strFullExeFileName.rfind('\\');
    if (std::string::npos != last_slash_idx)
    {
        strExePath = strFullExeFileName.substr(0, last_slash_idx);
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tancodrom", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewInit();

    pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 1.0, 3.0));

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
      
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete pCamera;
    glfwTerminate();
    return 0;
}