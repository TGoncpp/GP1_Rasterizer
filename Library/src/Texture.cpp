#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
		if (loadedSurface == NULL)
		{
			printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		}

		return new Texture(loadedSurface);

	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		const float uNormal{ std::ranges::clamp(uv.x, 0.f, 1.f) };
		const float vNormal{ std::ranges::clamp(uv.y, 0.f, 1.f) };

		const Uint32 u{ Uint32 (uNormal * m_pSurface->w) };
		const Uint32 v{ Uint32 (vNormal * m_pSurface->h) };

		//const Uint32 totPxl{ Uint32(m_pSurface->w * m_pSurface->h) };

		Uint32 pxl{ ( u  + (v * m_pSurface->w )) };

		Uint8 r{}, b{}, g{};

		SDL_GetRGB(m_pSurfacePixels[pxl], m_pSurface->format, &r, &b, &g);

		return { ColorRGB{(float)r, (float)g, (float)b} };
	}
}