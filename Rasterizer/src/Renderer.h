#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

	private:
		void VertectTransformToScreen(const std::vector<Vector3>& vertices_in, std::vector<Vector2>& vertices_out) const;
		void VertectTransformToScreen(const std::vector<Vertex>& vertices_in, std::vector<Vector2>& vertices_out) const;

		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		Camera m_Camera{};
		Texture* m_pTexture{};
		Texture* m_pTexture1{};


		int m_Width{};
		int m_Height{};


		void IntroRender()const;
		void Render_W1_1()const;
		void Render_W1_2();
		void Render_W1_3();
		void Render_W1_4();
		void Render_W1_5();

		void Render_W2_1();
		void Render_W2_2();

		void ResetDepthBuffer();
		void ResetColorBuffer();
		bool IsInBoundingBox(const Vector2& pxlScr, size_t indc, const std::vector<Vector2>& vector2_Screen);
		bool IsInBoundingBox(const Vector2& pxlScr, size_t indc, const std::vector<Vector2>& vector2_Screen, const Mesh& mesh);
	};
}
