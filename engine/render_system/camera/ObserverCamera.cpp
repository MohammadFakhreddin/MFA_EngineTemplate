#include "ObserverCamera.hpp"

#include "LogicalDevice.hpp"
#include "UI.hpp"

namespace MFA
{
	//-------------------------------------------------------------------------------------------------

	ObserverCamera::ObserverCamera()
		: PerspectiveCamera()
	{
		LogicalDevice::Instance->SDL_EventSignal.Register([&](SDL_Event* event)->void
			{
				if (UI::Instance != nullptr && UI::Instance->HasFocus() == true)
				{
					_motionButtons = {};
					_leftMouseDown = false;
				}
				else if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
				{
					auto modifier = event->type == SDL_KEYDOWN ? 1.0f : -1.0f;
					
					if (event->key.keysym.sym == SDLK_w)
					{
						_motionButtons.z += 1.0f * modifier;
					}
					else if (event->key.keysym.sym == SDLK_s)
					{
						_motionButtons.z += -1.0f * modifier;
					}
					else if (event->key.keysym.sym == SDLK_d)
					{
						_motionButtons.x -= 1.0f * modifier;
					}
					else if (event->key.keysym.sym == SDLK_a)
					{
						_motionButtons.x += 1.0f * modifier;
					}
					else if (event->key.keysym.sym == SDLK_q)
					{
						_motionButtons.y -= 1.0f * modifier;
					}
					else if (event->key.keysym.sym == SDLK_e)
					{
						_motionButtons.y += 1.0f * modifier;
					}

					_motionButtons.x = std::clamp(_motionButtons.x, -1.0f, 1.0f);
					_motionButtons.y = std::clamp(_motionButtons.y, -1.0f, 1.0f);
					_motionButtons.z = std::clamp(_motionButtons.z, -1.0f, 1.0f);
				}
				/*else if (event->type == SDL_MOUSEMOTION)
				{
					_mouseRelX = event->motion.x - _mouseX;
					_mouseRelY = event->motion.y - _mouseY;

					_mouseX = event->motion.x;
					_mouseY = event->motion.y;
					
				}*/
				else if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
				{
					auto const modifier = event->type == SDL_MOUSEBUTTONDOWN ? true : false;
					if (event->button.button == SDL_BUTTON_LEFT)
					{
						_leftMouseDown = modifier;
					}
				}
			}
		);

		//if (UI::Instance != nullptr)
		//{
		//	UI::Instance->UpdateSignal.Register([this]()->void
		//	{
		//		UI::Instance->BeginWindow("Observer camera");
		//		ImGui::InputFloat3("Position", &_position[0]);

		//		auto eulerAngles = _rotation.GetEulerAngles();
		//		ImGui::InputFloat3("Euler angles", &eulerAngles[0]);

		//		UI::Instance->EndWindow();
		//	});
		//}
	}

	//-------------------------------------------------------------------------------------------------

	ObserverCamera::~ObserverCamera() = default;

	//-------------------------------------------------------------------------------------------------

	void ObserverCamera::Update(float deltaTimeInSec)
	{
		UpdateMousePosition();

		if (_movementEnabled == false)
		{
			return;
		}

		auto const uiHasFocus = UI::Instance != nullptr ? UI::Instance->HasFocus() : false;

		if (_leftMouseDown && uiHasFocus == false)
		{
			SDL_ShowCursor(SDL_DISABLE);
			if (_mouseRelX != 0.0f || _mouseRelY != 0.0f)
			{
				auto eulerAngles = _rotation.GetEulerAngles();
				auto const rotationDistance = _rotationSpeed * deltaTimeInSec;
				eulerAngles.y = eulerAngles.y + rotationDistance * _mouseRelX;    // Reverse for view mat
				eulerAngles.x = std::clamp(
					eulerAngles.x - rotationDistance * _mouseRelY,
					-90.0f,
					90.0f
				);    // Reverse for view mat
				_rotation.SetEulerAngles(eulerAngles);
				SetViewDirty();
			}
		}
		else
		{
			SDL_ShowCursor(SDL_ENABLE);
		}
		
		auto const motionMag2 = glm::length2(_motionButtons);
		if (motionMag2 > 0.0f)
		{
			auto const motionDirection =  _motionButtons / std::sqrt(motionMag2);
			auto const moveDistance = _movementSpeed * deltaTimeInSec;
			auto const movementVector = motionDirection * moveDistance;

			auto position = _position;
			if (movementVector.z != 0.0f)
			{
				// auto const forward = glm::transpose(_rotation.GetMatrix()) * Math::ForwardVec4W0;
				// position = position + glm::normalize(glm::vec3{ forward }) *  movementVector.z;
				position = position + _forward *  movementVector.z;
			}
			if (movementVector.x != 0.0f)
			{
				// auto const right = glm::transpose(_rotation.GetMatrix()) * Math::RightVec4W0;
				// position = position + glm::normalize(glm::vec3{ right }) * movementVector.x;
				position = position + _right * movementVector.x;
			}
			if (movementVector.y != 0.0f)
			{
				// auto const up = glm::transpose(_rotation.GetMatrix()) * Math::UpVec4W0;
				// position = position + glm::normalize(glm::vec3{ up }) * movementVector.y;
				position = position + _up * movementVector.y;
			}

			_position = position;

			SetViewDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void ObserverCamera::UpdateMousePosition()
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
}
