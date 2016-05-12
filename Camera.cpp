#include "Camera.h"
#define DEG_TO_RAD 0.0174532
#define MIN_FOV 1.0
#define MAX_FOV 60.0
#define MIN_PITCH -89.0
#define MAX_PITCH 89.0

Camera::Camera(const Eigen::Vector3f& pos0, const Eigen::Vector3f& up0, const float& fov0):
pos(pos0),
up(up0),
yaw(-90.0),
fov(fov0),
pitch(0.0),
speed(5.0)
{
    updateDirection();
}

void Camera::processKeyboard(CameraMovement key, const float& dt)
{
    float scale = speed * dt;
    if (key == FORWARD) {
        pos += dir * scale;
        
    } else if (key == BACKWARD) {
        pos -= dir * scale;
        
    } else if (key == LEFT) {
        pos -= right * scale;
        
    } else if (key == RIGHT) {
        pos += right * scale;
    
    } else if (key == UP) {
        pos += up * scale;
    
    } else if (key == DOWN) {
        pos -= up * scale;
    }
}

void Camera::processMouse(const float& dx, const float& dy)
{
    yaw += dx;
    pitch += dy;
    
    if (pitch < MIN_PITCH) pitch = MIN_PITCH;
    else if (pitch > MAX_PITCH) pitch = MAX_PITCH;
    
    updateDirection();
}

void Camera::processScroll(const float& scroll)
{
    fov -= scroll;
    
    if (fov < MIN_FOV) fov = MIN_FOV;
    else if (fov > MAX_FOV) fov = MAX_FOV;
}

void Camera::updateDirection()
{
    dir.x() = cos(yaw * DEG_TO_RAD) * cos(pitch * DEG_TO_RAD);
    dir.y() = sin(pitch * DEG_TO_RAD);
    dir.z() = sin(yaw * DEG_TO_RAD) * cos(pitch * DEG_TO_RAD);
    dir.normalize();
    right = dir.cross(up).normalized();
}