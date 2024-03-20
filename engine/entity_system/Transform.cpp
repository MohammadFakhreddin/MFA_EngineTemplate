#include "Transform.hpp"

#include "BedrockMath.hpp"

namespace MFA
{
    
	//-------------------------------------------------------------------------------------------------

    Transform::Transform() = default;
	
	//-------------------------------------------------------------------------------------------------

	void Transform::SetEulerAngles(glm::vec3 const & eulerAngles)
	{
		_isDirty |= _rotation.SetEulerAngles(eulerAngles);
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::SetQuaternion(glm::quat const & quaternion)
	{
		_isDirty |= _rotation.SetQuaternion(quaternion);
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const& Transform::GetMatrix()
	{
		if (_isDirty == true)
		{
			_transform = _extraTransform * Math::Translate(_position) * _rotation.GetMatrix() * Math::Scale(_scale);
			_isDirty = false;
		}
		return _transform;
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::SetDirty()
	{
		_isDirty = true;
	}

	//-------------------------------------------------------------------------------------------------

}