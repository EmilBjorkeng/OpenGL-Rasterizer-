#ifndef __CAMERA_H__
#define __CAMERA_H__

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    float fov = 90.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Camera() = default;

    // Constructor with position and quaternion
    Camera(const glm::vec3& initPosition, const glm::quat& initRotation)
        : position(initPosition), rotation(initRotation) {}

    // Constructor with position and Euler angles (pitch, yaw, roll)
    Camera(const glm::vec3& initPosition, const glm::vec3& eulerAngles)
        : position(initPosition), rotation(glm::quat(glm::radians(eulerAngles))) {}

    glm::vec3 Forward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
    glm::vec3 Right()   const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
    glm::vec3 Up()      const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

    glm::mat4 GetViewMatrix() const {
        glm::mat4 rotMat = glm::toMat4(glm::inverse(rotation));
        glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);
        return rotMat * transMat;
    }
};

#endif
