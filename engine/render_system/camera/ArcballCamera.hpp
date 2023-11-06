#pragma once

#include "PerspectiveCamera.hpp"

#include <SDL2/SDL_events.h>

namespace MFA
{
    class ArcballCamera : public PerspectiveCamera
    {
    public:

        explicit ArcballCamera(glm::vec3 target = {});

        ~ArcballCamera();

        void Update(float dtSec) override;

    protected:

        void UpdateMousePosition();

        void OnSDL_Event(SDL_Event* event);

        void CalculateViewMat() override;

        MFA_VARIABLE1(movementSpeed, float, 10.0f)
        MFA_VARIABLE1(rotationSpeed, float, 0.4f)
        MFA_VARIABLE1(movementEnabled, bool, true)
        MFA_VARIABLE1(scrollSpeed, float, 1.0f)
        MFA_VARIABLE1(minDistance, float, 1.0f)
        MFA_VARIABLE1(maxDistance, float, 20.0f)

        float _mouseX = 0.0f;
        float _mouseY = 0.0f;
        float _mouseRelX = 0.0f;        // Mouse relative motion
        float _mouseRelY = 0.0f;

        bool _leftMouseDown = false;
        
        glm::vec3 _target{};
    
    };
}
