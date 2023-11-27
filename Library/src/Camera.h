#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float ar{ 1};
		float nearPlane{ 1 };
		float farPlane{ 100 };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void SetAspectRatio(float value) { ar = value; }

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix			

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = Matrix{ {right, 0}, {up, 0},
				{forward, 0}, {origin, 1} };

			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);

			
			

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W3

			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, ar, nearPlane, farPlane);
			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[NULL])
			{
				return;
			}

			float
				scrollSpeed{ 7.f },
				swipeSpeed{ 0.25f },
				rotationSpeed{ 0.5f };

			//Keyboard Input
			//*********************************
#pragma region KeyboardInput
			//camera movement

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * scrollSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * scrollSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * scrollSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * scrollSpeed * deltaTime;
			}



#pragma endregion

#pragma region MouseInput

			//Mouse Input
			//move forward/sideways
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (SDL_BUTTON(1) == mouseState)
			{
				
				origin -= forward * swipeSpeed * deltaTime * float(mouseY);
				totalYaw -= float(mouseX) * deltaTime * rotationSpeed;
				//forward = Matrix::CreateRotationY(totalYaw).TransformVector(Vector3::UnitZ);

			}
			
			//move up and down
			if ((SDL_BUTTON(1) | SDL_BUTTON(3)) == mouseState)
			{
				origin -= up * swipeSpeed * deltaTime * float(mouseY);
			}
			
			//Rotation
			if (SDL_BUTTON(3) == mouseState)
			{
				totalPitch += float(mouseY) * deltaTime * rotationSpeed;
				totalYaw   -= float(mouseX) * deltaTime * rotationSpeed;
				//forward = Matrix::CreateRotationY(totalYaw).TransformVector(Vector3::UnitZ);
			}


#pragma endregion
			const Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
