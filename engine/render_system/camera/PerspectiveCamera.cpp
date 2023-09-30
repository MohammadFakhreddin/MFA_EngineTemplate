#include "PerspectiveCamera.hpp"

#include "LogicalDevice.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	PerspectiveCamera::PerspectiveCamera()
	{
		_resizeListener = LogicalDevice::Instance->ResizeEventSignal1.Register([this]()
		{
			SetProjectionDirty();
		});
	}

	//-------------------------------------------------------------------------------------------------

	PerspectiveCamera::~PerspectiveCamera()
	{
		LogicalDevice::Instance->ResizeEventSignal1.UnRegister(_resizeListener);
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 PerspectiveCamera::GetViewProjection()
	{
		if (_isViewDirty)
		{
			CalculateViewMat();
		}
		
		if (_isProjectionDirty)
		{
			CalculateProjMat();
		}
		
		if (_isViewDirty == true || _isProjectionDirty == true)
		{
			_viewProjMat = _projMat * _viewMat;
		}

		if (_isViewDirty == true)
		{
			auto rotationT = glm::transpose(_rotation.GetMatrix());
			_forward = glm::normalize(rotationT * Math::ForwardVec4W0);
			_right = glm::normalize(rotationT * Math::RightVec4W0);
			_up = glm::normalize(rotationT * Math::UpVec4W0);
		}

		_isProjectionDirty = false;
		_isViewDirty = false;

		return _viewProjMat;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetProjectionDirty()
	{
		_isProjectionDirty = true;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetViewDirty()
	{
		_isViewDirty = true;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::CalculateViewMat()
	{
		auto const translation = glm::translate(glm::identity<glm::mat4>(), _position);
		auto const viewMat4 = _rotation.GetMatrix() * translation;
		_viewMat = viewMat4;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::CalculateProjMat()
	{
		auto const extent = LogicalDevice::Instance->GetSurfaceCapabilities().currentExtent;
		float const aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

		Math::PerspectiveProjection(_projMat, aspectRatio, _fovDeg, _nearPlane, _farPlane);
	}

	//-------------------------------------------------------------------------------------------------

	bool PerspectiveCamera::IsDirty() const
	{
		return _isViewDirty == true || _isProjectionDirty == true;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & PerspectiveCamera::GetForward() const
	{
		return _forward;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & PerspectiveCamera::GetRight() const
	{
		return _right;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & PerspectiveCamera::GetUp() const
	{
		return _up;
	}

	//-------------------------------------------------------------------------------------------------

}
