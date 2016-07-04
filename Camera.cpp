#include "Camera.h"
#define MIN_FOV 1.0
#define MAX_FOV 60.0
#define MIN_PITCH -89.0
#define MAX_PITCH 89.0

Camera::Camera(const Eigen::Vector3f& pos0, const Eigen::Vector3f& up0, const float& fov0,
               const float& near0, const float& far0):
pos(pos0),
up(up0),
yaw(-90.0),
fov(fov0),
aspect(1.0),
near(near0),
far(far0),
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

glm::mat4 Camera::projectionMatrix(float x, float y)
{
    aspect = x/y;
    return glm::perspective(fov, aspect, near, far);
}

glm::mat4 Camera::viewMatrix()
{
    
    Eigen::Vector3f center = pos + dir;
    return glm::lookAt(glm::vec3(pos.x(), pos.y(), pos.z()),
                       glm::vec3(center.x(), center.y(), center.z()),
                       glm::vec3(up.x(), up.y(), up.z()));
}

void Camera::computeFrustum(std::vector<Eigen::Vector3f>& frustum, Eigen::Vector3f& center)
{
    // compute frustum vertices
    float hNear = tan(fov / 2) * near;
    float wNear = hNear * aspect;
    float hFar = tan(fov / 2) * far;
    float wFar = hFar * aspect;
    
    Eigen::Vector3f nc = pos + dir * near;
    Eigen::Vector3f fc = pos + dir * far;
    
    frustum.resize(8);
    frustum[0] = nc + (up * hNear) - (right * wNear);
    frustum[1] = nc + (up * hNear) + (right * wNear);
    frustum[2] = nc - (up * hNear) - (right * wNear);
    frustum[3] = nc - (up * hNear) + (right * wNear);
    frustum[4] = fc + (up * hFar) - (right * wFar);
    frustum[5] = fc + (up * hFar) + (right * wFar);
    frustum[6] = fc - (up * hFar) - (right * wFar);
    frustum[7] = fc - (up * hFar) + (right * wFar);
    
    // compute frustum center
    center = frustum[0];
    for (int i = 1; i < 8; i++) {
        center += frustum[i];
    }
    
    center /= 8.0;
}

void Camera::updateDirection()
{
    dir.x() = cos(yaw * DEG_TO_RAD) * cos(pitch * DEG_TO_RAD);
    dir.y() = sin(pitch * DEG_TO_RAD);
    dir.z() = sin(yaw * DEG_TO_RAD) * cos(pitch * DEG_TO_RAD);
    dir.normalize();
    right = dir.cross(up).normalized();
}
