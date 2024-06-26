#pragma once

#include "BedrockCommon.hpp"
#include "BedrockRotation.hpp"

namespace MFA
{
	class PerspectiveCamera
	{
	public:

		explicit PerspectiveCamera();
		~PerspectiveCamera();

		[[nodiscard]]
		glm::mat4 const & GetViewProjection();

		[[nodiscard]]
		glm::mat4 const & GetView();

		glm::mat4 const & GetProjection();

		[[nodiscard]]
		bool IsDirty() const;

		[[nodiscard]]
		glm::vec3 const & GetForward() const;

		[[nodiscard]]
		glm::vec3 const & GetRight() const;

		[[nodiscard]]
		glm::vec3 const & GetUp() const;

		virtual void Update(float dtSec) {}

	protected:

		void SetProjectionDirty();

		void SetViewDirty();

		virtual void CalculateViewMat();

		void CalculateProjMat();

		MFA_VARIABLE2(fovDeg, float, 40.0f, SetProjectionDirty)

		MFA_VARIABLE2(rotation, Rotation, Rotation { glm::vec3(0.0f, 180.0f, 180.0f) }, SetViewDirty);
		
		MFA_VARIABLE2(position, glm::vec3, {}, SetViewDirty);
		
		MFA_VARIABLE2(farPlane, float, 1000.0f, SetProjectionDirty)

		MFA_VARIABLE2(nearPlane, float, 0.01f, SetProjectionDirty)

		bool _isProjectionDirty = true;

		bool _isViewDirty = true;

		glm::mat4 _viewMat{};
		glm::mat4 _projMat{};
		glm::mat4 _viewProjMat{};

		glm::vec3 _forward{};
		glm::vec3 _right{};
		glm::vec3 _up{};

		int _resizeListener = -1;

	};
}
