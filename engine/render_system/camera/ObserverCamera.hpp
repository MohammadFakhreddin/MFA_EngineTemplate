#pragma once
#include "PerspectiveCamera.hpp"

namespace MFA
{
    class ObserverCamera : public PerspectiveCamera
    {
    public:

        explicit ObserverCamera();
        ~ObserverCamera();

        void Update(float deltaTimeInSec);

    protected:

        void UpdateMousePosition();

        MFA_VARIABLE1(movementSpeed, float, 10.0f)
        MFA_VARIABLE1(rotationSpeed, float, 2.5f)
        MFA_VARIABLE1(movementEnabled, bool, true)

        float _mouseX = 0.0f;
        float _mouseY = 0.0f;
        float _mouseRelX = 0.0f;        // Mouse relative motion
        float _mouseRelY = 0.0f;

        bool _leftMouseDown = false;

        glm::vec3 _motionButtons{};
    };
}
