#include "ArcballCamera.hpp"

#include "LogicalDevice.hpp"
#include "UI.hpp"
#include "BedrockLog.hpp"
#include "BedrockMath.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	ArcballCamera::ArcballCamera(glm::vec3 target)
	{
		_target = std::move(target);
		LogicalDevice::Instance->SDL_EventSignal.Register([&](SDL_Event* event)->void{OnSDL_Event(event);});
	}

	//-------------------------------------------------------------------------------------------------

	ArcballCamera::~ArcballCamera() = default;

	//-------------------------------------------------------------------------------------------------

	void ArcballCamera::Update(float const dtSec)
	{
		UpdateMousePosition();

		if ((std::abs(_mouseRelX) > glm::epsilon<float>() || std::abs(_mouseRelY) > glm::epsilon<float>()) && _leftMouseDown == true)
		{
			auto const camVec = _position - _target;

			auto const rotDt = _rotationSpeed * dtSec;

			float const eulerX = rotDt * _mouseRelX;
			float const eulerY = -rotDt * _mouseRelY;

			auto const forward = glm::normalize(camVec);
			auto const biTan = glm::normalize(glm::cross(forward, Math::UpVec3));

			auto const up = glm::normalize(glm::cross(forward, biTan));

			auto const rotX = glm::rotate(glm::identity<glm::quat>(), eulerX, up);
			auto const rotY = glm::rotate(glm::identity<glm::quat>(), eulerY, biTan);

			auto const newPosition = (rotX * rotY * camVec) + _target;

			if (std::abs(glm::dot(glm::normalize(newPosition - _target), Math::UpVec3)) < 0.99f)
			{
				_position = newPosition;
				SetViewDirty();
			}

		}
	}

	//-------------------------------------------------------------------------------------------------

	void ArcballCamera::UpdateMousePosition()
	{
		int mouseNewX = 0;
		int mouseNewY = 0;
		SDL_GetMouseState(&mouseNewX, &mouseNewY);

		_mouseRelX = mouseNewX - _mouseX;
		_mouseRelY = mouseNewY - _mouseY;

		_mouseX = mouseNewX;
		_mouseY = mouseNewY;

		auto const uiHasFocus = UI::Instance != nullptr ? UI::Instance->HasFocus() : false;

		if (_leftMouseDown == true && uiHasFocus == false)
		{
			auto const surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();
			auto const screenWidth = surfaceCapabilities.currentExtent.width;
			auto const screenHeight = surfaceCapabilities.currentExtent.height;

			bool mousePositionNeedsWarping = false;
			if (_mouseX < static_cast<float>(screenWidth) * 0.010f) {
				_mouseX = static_cast<float>(screenWidth) * 0.010f + screenWidth * 0.5f;
				mousePositionNeedsWarping = true;
			}
			if (_mouseX > static_cast<float>(screenWidth) * 0.990f) {
				_mouseX = static_cast<float>(screenWidth) * 0.990f - screenWidth * 0.5f;
				mousePositionNeedsWarping = true;
			}
			if (_mouseY < static_cast<float>(screenHeight) * 0.010f) {
				_mouseY = static_cast<float>(screenHeight) * 0.010f + screenHeight * 0.5f;
				mousePositionNeedsWarping = true;
			}
			if (_mouseY > static_cast<float>(screenHeight) * 0.990f) {
				_mouseY = static_cast<float>(screenHeight) * 0.990f - screenHeight * 0.5f;
				mousePositionNeedsWarping = true;
			}
			if (mousePositionNeedsWarping) {
				SDL_WarpMouseInWindow(LogicalDevice::Instance->GetWindow(), _mouseX, _mouseY);
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	void ArcballCamera::OnSDL_Event(SDL_Event* event)
	{
		if (UI::Instance != nullptr && UI::Instance->HasFocus() == true)
		{
			_leftMouseDown = false;
		}
		else if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
		{
			auto const modifier = event->type == SDL_MOUSEBUTTONDOWN ? true : false;
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				_leftMouseDown = modifier;
			}
		}
		else if (event->type == SDL_MOUSEWHEEL)
		{
			if (std::abs(event->wheel.y) > 0) // scroll up
			{
				// Put code for handling "scroll up" here!
				auto const camVec = _target - _position;
				auto const camMag = glm::length(camVec);
				auto const camDir = camVec / camMag;
				auto const deltaVec = camDir * std::abs(event->wheel.preciseY) * (event->wheel.preciseY > 0.0f ? 1.0f : -1.0f);
				auto const deltaMag = glm::length(deltaVec + _position - _target);
				if (deltaMag >= _minDistance && deltaMag <= _maxDistance)
				{
					_position += deltaVec;
					_isViewDirty = true;
				}
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	void ArcballCamera::CalculateViewMat()
	{
		auto const camVec = _target - _position;

		_forward = glm::normalize(camVec);
		_right = glm::normalize(glm::cross(_forward, Math::UpVec3));
		_up = glm::normalize(glm::cross(_forward, _right));

		_viewMat = glm::lookAt(_position, _target, _up);
	}

	//-------------------------------------------------------------------------------------------------

}
