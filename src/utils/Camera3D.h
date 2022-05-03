#pragma once

// Std. Includes
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW        = -90.0f;
const float PITCH      =  0.0f;
const float SPEED      =  1.5f;
const float SENSITIVTY =  0.50f;
const float ZOOM       =  45.0f;

class Camera3D
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
public:
    // Constructor with vectors
    Camera3D(glm::vec3 position = glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    // Constructor with scalar values
    Camera3D(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
    // Update the camera movement in the world space
    void Update(Window& window, float deltaTime);
    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();
    glm::mat4 GetViewMatrixLH();
    glm::mat4 GetViewMatrixRH();
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);
private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};
