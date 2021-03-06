#pragma once
/*
#define CALCULATE_MODEL_MATRIX()        m_ModelMatrix       = glm::mat4(1.0f); \
                                        m_ModelMatrix       = glm::translate(m_ModelMatrix, transform.position); \
                                        m_ModelMatrix       = glm::rotate(m_ModelMatrix, (float)glm::radians(transform.rotation.x), glm::vec3(1, 0, 0)); \
                                        m_ModelMatrix       = glm::rotate(m_ModelMatrix, (float)glm::radians(transform.rotation.y), glm::vec3(0, 1, 0)); \
                                        m_ModelMatrix       = glm::rotate(m_ModelMatrix, (float)glm::radians(transform.rotation.z), glm::vec3(0, 0, 1)); \
                                        m_ModelMatrix       = glm::scale(m_ModelMatrix, transform.scale);
*/

// Imgui
#include <imgui.h>
#include <imgui_internal.h>
// ImGuizmo
#include <ImGuizmo/ImGuizmo.h>
// GLM
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Transform component
struct Transform
{
    glm::vec3   position = glm::vec3(0.0f);
    glm::vec3   rotation = glm::vec3(0.0f);
    glm::vec3   scale = glm::vec3(1.0f);
    glm::mat4   transformMatrix = glm::mat4(1.0f);

    Transform(glm::vec3 pos = glm::vec3(0, 0, 0), glm::vec3 rot = glm::vec3(0, 0, 0), glm::vec3 scale = glm::vec3(1, 1, 1))
        : position(pos), rotation(rot), scale(scale)
    {
        transformMatrix = glm::translate(transformMatrix, position);
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.x), glm::vec3(1, 0, 0));
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.y), glm::vec3(0, 1, 0));
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.z), glm::vec3(0, 0, 1));
        transformMatrix = glm::scale(transformMatrix, scale);
    }

    Transform AttachGuizmo(ImGuizmo::OPERATION operation, const glm::mat4& view, const glm::mat4& projection)
    {
        // static glm::mat4 transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::translate(transformMatrix, position);
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.x), glm::vec3(1, 0, 0));
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.y), glm::vec3(0, 1, 0));
        transformMatrix = glm::rotate(transformMatrix, (float)glm::radians(rotation.z), glm::vec3(0, 0, 1));
        transformMatrix = glm::scale(transformMatrix, scale);

        // Transform transform(position, rotation, scale);
        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operation, ImGuizmo::MODE::WORLD, &(this->transformMatrix[0][0]));
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(&(this->transformMatrix[0][0]), matrixTranslation, matrixRotation, matrixScale);
        this->position = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
        this->rotation = glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
        this->scale = glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]);
        // this->transformMatrix = transformMatrix;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Transform& transform)
    {
        stream << "Transform\n" << "|\n|---> Position : [" << transform.position.x << ", " << transform.position.z << ", " << transform.position.x << "]\n" <<
        "|\n|---> Rotation : [" << transform.rotation.x << ", " << transform.rotation.y << ", " << transform.rotation.z << "]\n" <<
        "|\n|---> Scale : [" << transform.scale.x << ", " << transform.scale.y << ", " << transform.scale.z << "]" << std::endl;

        return stream;
    }
};

#define ATTACH_GUIZMO(x, operation) x = x.AttachGuizmo(operation, renderer.GetViewMatrix(), renderer.GetProjectionMatrix());
