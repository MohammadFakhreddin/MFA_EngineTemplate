#pragma once

#include "BedrockRotation.hpp"
#include "BedrockCommon.hpp"

namespace MFA
{
    class Transform
    {

    public:

        explicit Transform();

        void SetEulerAngles(glm::vec3 const & eulerAngles);

        void SetQuaternion(glm::quat const & quaternion);

        [[nodiscard]]
        glm::mat4 const& GetMatrix();

    private:

        void SetDirty();

        MFA_VARIABLE2(position, glm::vec3, glm::vec3(0.0f, 0.0f, 0.0f), SetDirty);
        MFA_VARIABLE2(rotation, Rotation, Rotation(glm::identity<glm::quat>()), SetDirty);
        MFA_VARIABLE2(scale, glm::vec3, glm::vec3(1.0f, 1.0f, 1.0f), SetDirty);
        MFA_VARIABLE2(extraTransform, glm::mat4, glm::identity<glm::mat4>(), SetDirty);
    	
        bool _isDirty = true;
        glm::mat4 _transform{};

    };
}