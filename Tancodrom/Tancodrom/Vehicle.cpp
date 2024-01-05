#include "Vehicle.h"

Vehicle::Vehicle(Model model, const int width, const int height, const glm::vec3& position)
{
    vehicleModel = model;
	startPosition = position;
	Set(width, height, position);
}

void Vehicle::Set(const int width, const int height, const glm::vec3 & position)
{
    this->isPerspective = true;
    this->yaw = YAW;

    this->FoVy = FOV;
    this->width = width;
    this->height = height;
    this->zNear = zNEAR;
    this->zFar = zFAR;

    this->worldUp = glm::vec3(0, 1, 0);//???
    this->position = position;

    lastX = width / 2.0f;
    lastY = height / 2.0f;

    UpdateVehicleVectors();
}

const glm::vec3 Vehicle::GetPosition() const
{
	return position;
}

const glm::mat4 Vehicle::GetProjectionMatrix() const
{
    glm::mat4 Proj = glm::mat4(1);
    if (isPerspective)
    {
        float aspectRatio = ((float)(width)) / height;
        Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
    } else
    {
        float scaleFactor = 2000.f;
        Proj = glm::ortho<float>(
            -width / scaleFactor, width / scaleFactor,
            -height / scaleFactor, height / scaleFactor, -zFar, zFar);
    }
    return Proj;
}

void Vehicle::ProcessKeyboard(VehicleMovementType direction, float deltaTime)
{
    float velocity = (float)(vehicleSpeedFactor * deltaTime);
    switch (direction)
    {
        case VehicleMovementType::V_FORWARD:
            position += forward * velocity;
            break;
        case VehicleMovementType::V_BACKWARD:
            position -= forward * velocity;
            break;
        case VehicleMovementType::V_LEFT:
            position -= right * velocity;
            break;
        case VehicleMovementType::V_RIGHT:
            position += right * velocity;
            break;
        case VehicleMovementType::V_ROTATE_RIGHT:
            rotation += 0.05f * velocity;
            break;
        case VehicleMovementType::V_ROTATE_LEFT:
            rotation -= 0.05f * velocity;
            break;
    }
}

Model& Vehicle::getVehicleModel()
{
    return vehicleModel;
}

float Vehicle::getRotation()
{
    return rotation;
}

void Vehicle::UpdateVehicleVectors()
{
    // Calculate the new forward vector
    this->forward.x = cos(glm::radians(yaw));
    this->forward.z = sin(glm::radians(yaw));
    this->forward = glm::normalize(this->forward);
    // Also re-calculate the Right and Up vector

    //See if right is needed

    right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
}
