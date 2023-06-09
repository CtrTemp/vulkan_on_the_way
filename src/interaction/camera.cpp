#include "interaction/camera.h"

glm::vec3 look_from_pos = glm::vec3(5.0f, 2.0f, 2.0f);
glm::vec3 look_at_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 world_up_dir = glm::vec3(0.0, 0.0, 1.0);

Camera prim_camera(look_from_pos, look_at_pos, world_up_dir);
// Camera prim_camera(glm::vec3(0.0f, -2.0f, 2.0f), glm::radians(45.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// 常规构造
Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldup)
{
    Position = position;
    WorldUp = worldup;
    Forward = glm::normalize(target - position);
    Right = glm::normalize(glm::cross(Forward, WorldUp));
    Up = glm::normalize(glm::cross(Right, Forward));
}

// 欧拉角构造
Camera::Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup)
{
    Position = position;
    WorldUp = worldup;
    Pitch = pitch;
    Yaw = yaw;
    Forward.x = glm::cos(Pitch) * glm::sin(Yaw);
    Forward.y = glm::sin(Pitch);
    Forward.z = glm::cos(Pitch) * glm::cos(Yaw);
    Right = glm::normalize(glm::cross(Forward, WorldUp));
    Up = glm::normalize(glm::cross(Right, Forward));
}

// 获取视图观察矩阵
glm::mat4 Camera::GetViewMatrix(float time)
{
    // std::cout << "time = " << time << std::endl;

    float z_pos = 2 * sin(time);
    float x_pos = 2 * sin(time);

    glm::vec3 new_target = glm::vec3(x_pos, 0, 0);

    // return glm::lookAt(Position, new_target, WorldUp);
    return glm::lookAt(Position, Position + Forward, WorldUp);
}

// 更新摄像机角度
void Camera::UpdataCameraVectors()
{
    // Forward.x = glm::cos(Pitch) * glm::sin(Yaw);
    // Forward.y = glm::sin(Pitch);
    // Forward.z = glm::cos(Pitch) * glm::cos(Yaw);
    Right = glm::normalize(glm::cross(Forward, WorldUp));
    Up = glm::normalize(glm::cross(Right, Forward));
}

// 鼠标移动
void Camera::ProcessMouseMovement(float deltaX, float deltaY)
{
    // Pitch += deltaY * SenceX;
    // Yaw += deltaX * SenceY;
    // if (Pitch > 89.0f)
    //     Pitch = 89.0f;
    // if (Pitch < -89.0f)
    //     Pitch = -89.0f;

    glm::vec3 cur_lookat = Position + Forward;
    glm::vec3 new_lookat = cur_lookat + Right * (deltaX / 300) - Up * (deltaY / 300);
    Forward = glm::normalize(new_lookat - Position);
    UpdataCameraVectors();
}

// 更新摄像机位置
void Camera::UpdataCameraPosition()
{
    // Position += glm::vec3(speedX, speedY,-speedZ) * 0.3f;
    Position += Forward * speedZ * 0.001f + Right * speedX * 0.001f + Up * speedY * 0.001f;
}
