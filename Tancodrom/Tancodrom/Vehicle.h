#pragma once

#include <GL/glew.h>
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <glfw3.h>

#include "Model.h"

enum VehicleMovementType
{
    V_UNKNOWN,
    V_FORWARD,
    V_BACKWARD,
    V_LEFT,
    V_RIGHT,
    V_ROTATE_LEFT,
    V_ROTATE_RIGHT
};

class Vehicle
{
private:
    const float zNEAR = 0.1f;
    const float zFAR = 500.f;
    const float YAW = -90.0f;
    const float FOV = 45.0f;
    glm::vec3 startPosition;
    Model vehicleModel;

public:
    Vehicle() = default;
    Vehicle(Model model, const int width, const int height, const glm::vec3& position);

    Vehicle& operator=(const Vehicle& othervehicle);

    void Set(const int width, const int height, const glm::vec3& position);

    const glm::vec3 GetPosition() const;

    //const glm::mat4 GetViewMatrix() const;

    const glm::mat4 GetProjectionMatrix() const;

    void ProcessKeyboard(VehicleMovementType direction, float deltaTime);

    Model& getVehicleModel();

    float getRotation();

private:
    void UpdateVehicleVectors();

protected:
    const float vehicleSpeedFactor = 2.5f;

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
    float rotation;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;

    float lastX = 0.f, lastY = 0.f;
};

