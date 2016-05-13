#ifndef CAMERA_H
#define CAMERA_H

#include "Types.h"

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // constructor
    Camera(const Eigen::Vector3f& pos0 = Eigen::Vector3f(0.0, 10.0, 0.0),
           const Eigen::Vector3f& up0 = Eigen::Vector3f(0.0, 1.0, 0.0),
           const float& fov0 = 45.0);
    
    // process keyboard
    void processKeyboard(CameraMovement key, const float& dt);
    
    // process mouse
    void processMouse(const float& dx, const float& dy);
    
    // process scroll
    void processScroll(const float& scroll);
    
    // member variables
    Eigen::Vector3f pos;
    Eigen::Vector3f dir;
    Eigen::Vector3f up;
    float fov;

private:
    // update
    void updateDirection();
    
    // member variables
    Eigen::Vector3f right;
    float yaw;
    float pitch;
    float speed;
};

#endif