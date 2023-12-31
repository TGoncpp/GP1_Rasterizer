//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"
#include "BRDFs.h"
#include <iostream>
#include <limits>

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer       = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer        = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels  = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[m_Width * m_Height];
	ResetDepthBuffer();

	//Initialize Camera
	m_Camera.Initialize(45.f, { .0f, 5.f, 64.f });
	m_Camera.SetAspectRatio(float(m_Width) / m_Height);

	//Init textures
	m_pTexture              = Texture::LoadFromFile("Resources/tuktuk.png");
	m_pTextureNormalMap     = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pTextureGlossines     = Texture::LoadFromFile("Resources/vehicle_gloss.png");
	m_pTextureSpecular      = Texture::LoadFromFile("Resources/vehicle_specular.png");
	m_pTextureVehicle       = Texture::LoadFromFile("Resources/vehicle_diffuse.png");

	//Init model
	m_Meshes_world.push_back( Mesh{} );
	Utils::ParseOBJ("Resources/vehicle.obj", m_Meshes_world[0].vertices, m_Meshes_world[0].indices);
	m_Meshes_world[0].primitiveTopology          = PrimitiveTopology::TriangleList;
	m_Meshes_world[0].worldMatrix                = Matrix::CreateTranslation({ 0.f, 0.f, 50.f });

}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
	delete m_pTextureNormalMap;
	delete m_pTextureGlossines;
	delete m_pTextureSpecular;
	delete m_pTextureVehicle;
}

void Renderer::Update(Timer* pTimer)
{
	if (m_Rotating)
	{
		//50 is offset position off model to world
		const float rotationSpeed{ pTimer->GetElapsed() };
		m_AngleOfModel += rotationSpeed;
		m_Meshes_world[0].worldMatrix = Matrix::CreateRotationY(m_AngleOfModel) * Matrix::CreateTranslation(0, 0, 50.f);
	}

	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//RENDER LOGIC
	//IntroRender();
	//Render_W1_1();
	//Render_W1_2();
	//Render_W1_3();
	//Render_W1_4();
	//Render_W1_5();
	//Render_W2_1();
	//Render_W2_2();
	//Render_W3_1();
	Render_W4_1();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
	
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void dae::Renderer::ToggleRotation()
{
	m_Rotating = !m_Rotating;
}

void dae::Renderer::SwitchLightMode()
{
	const int amountOfModes{ 4 };
	m_LightMode = static_cast<LightingMode>((int(m_LightMode) + 1) % amountOfModes);
	std::cout << "LightMode: oa/diffuse/specular/combined " << int(m_LightMode)  << std::endl;
}

void dae::Renderer::ToggleNormal()
{
	m_UseNormalMap = !m_UseNormalMap;
}


void Renderer::IntroRender()const
{
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ gradient, gradient, gradient };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::Render_W1_1()const
{
	ColorRGB finalColor{  };
	std::vector<Vector3> vertices_ndc{  {  0.f, .5f, 1.f },
										{ .5f, -.5f, 1.f },
										{-.5f, -.5f, 1.f }
	
	};
	
	std::vector<Vector2> vertices_Screen{};
	vertices_Screen.reserve(3);
	VertectTransformToScreen(vertices_ndc, vertices_Screen);


	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pxlScr{ px + 0.5f, py + 0.5f };
			
			/*if (px > m_Width/2 && py > m_Height/2)
			{ 
				std::cout << "het\n";
			}*/

			if ((Vector2::Cross(vertices_Screen[1] - vertices_Screen[0], pxlScr - vertices_Screen[0]) < 0.0f) 
			|| ( Vector2::Cross(vertices_Screen[2] - vertices_Screen[1], pxlScr - vertices_Screen[1]) < 0.0f) 
			|| ( Vector2::Cross(vertices_Screen[0] - vertices_Screen[2], pxlScr - vertices_Screen[2]) < 0.0f) )
				{ finalColor = {}; }
			else	
				finalColor = {1.f, 1.f, 1.f};

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::Render_W1_2()
{
	ColorRGB finalColor{  };

	//World Space
	std::vector<Vector3> vertices_world{ {  0.f, 2.f,  0.f },
										{ 1.f, .0f, 0.f },
										{ -1.f, .0f, 0.f }

	};

	//Apply vieuw matrix to get Vieuw Space
	std::vector<Vertex> vertices_Vieuw{};
	vertices_Vieuw.reserve(3);

	for (Vector3& vertc : vertices_world)
	{
		vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc) });
	}

	//Apply Projection Matrix to get NDC Space
	std::vector<Vertex> vertices_NDC{};
	vertices_NDC.reserve(3);
	VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

	//NDC to rasterSpace
	std::vector<Vector2> vertices_Screen{};
	VertectTransformToScreen(vertices_NDC, vertices_Screen);

	//check for every pxl if in triangle 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pxlScr{ px + 0.5f, py + 0.5f };

			////debug lines
			//if (px > m_Width/2 && py > m_Height/2.5)
			//{
			//	std::cout << "het\n";
			//}

			if ((   Vector2::Cross(vertices_Screen[1] - vertices_Screen[0], pxlScr - vertices_Screen[0]) < 0.0f)
				|| (Vector2::Cross(vertices_Screen[2] - vertices_Screen[1], pxlScr - vertices_Screen[1]) < 0.0f)
				|| (Vector2::Cross(vertices_Screen[0] - vertices_Screen[2], pxlScr - vertices_Screen[2]) < 0.0f))
			{
				finalColor = {};
			}
			else
				finalColor = { 1.f, 1.f, 1.f };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::Render_W1_3()
{
	ColorRGB finalColor{  };

	//World Space
	std::vector<Vertex> vertices_world{ {{  0.f, 4.f,  2.f }, {1.0f, 0.0f, 0.0f} },
										{{ 3.f, -2.0f, 2.f }, {0.0f, 1.0f, 0.0f} },
										{{ -3.f, -2.0f, 2.f }, {0.0f, 0.0f, 1.0f} }

	};

	//Apply vieuw matrix to get Vieuw Space
	std::vector<Vertex> vertices_Vieuw{};
	vertices_Vieuw.reserve(3);

	for (Vertex& vertc : vertices_world)
	{
		vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc.position) });
	}

	//Apply Projection Matrix to get NDC Space
	std::vector<Vertex> vertices_NDC{};
	vertices_NDC.reserve(3);
	VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

	//NDC to rasterSpace
	std::vector<Vector2> vertices_Screen{};
	VertectTransformToScreen(vertices_NDC, vertices_Screen);

	//check for every pxl if in triangle 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pxlScr{ px + 0.5f, py + 0.5f };

			////debug lines
			//if (px > m_Width / 2 && py > m_Height / 2.5)
			//{
			//	std::cout << "het\n";
			//}

			const float W2 = Vector2::Cross( pxlScr - vertices_Screen[0], vertices_Screen[1] - vertices_Screen[0]);
			const float W0 = Vector2::Cross( pxlScr - vertices_Screen[1], vertices_Screen[2] - vertices_Screen[1]);
			const float W1 = Vector2::Cross( pxlScr - vertices_Screen[2], vertices_Screen[0] - vertices_Screen[2]);

			if (   W0 > 0.0f ||  W1 > 0.0f ||  W2 > 0.0f)
			{
				finalColor = {};
			}
			else
			{
				const float W = Vector2::Cross(vertices_Screen[1] - vertices_Screen[2], vertices_Screen[0] - vertices_Screen[2]);
				float total = W0 / W + W1 / W + W2 / W;

				finalColor = { W0/W, W1/W , W2/W  };
				if (abs(1 - total) >= 0.001f)
					float total = W0 / W + W1 / W + W2 / W;
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::Render_W1_4()
{
	//SDL_Rect rect = SDL_Rect{ 0, 0, m_Width, m_Height };
	//SDL_FillRect(m_pBackBuffer, &rect, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	ColorRGB finalColor{  };

	//World Space
	std::vector<Vertex> vertices_world{
			//Triangle 0
			{{  0.f,  2.f,  0.f },  {1.0f, 0.0f, 0.0f} },
			{{ 1.5f, -1.0f, 0.f },  {1.0f, 0.0f, 0.0f} },
			{{ -1.5f, -1.0f, 0.f }, {1.0f, 0.0f, 0.0f} },

			//Triangle 1
			{{  0.f, 4.f,  2.f },  {1.0f, 0.0f, 0.0f} },
			{{ 3.f, -2.0f, 2.f },  {0.0f, 1.0f, 0.0f} },
			{{ -3.f, -2.0f, 2.f }, {0.0f, 0.0f, 1.0f} }

	};

	//Apply vieuw matrix to get Vieuw Space
	std::vector<Vertex> vertices_Vieuw{};
	vertices_Vieuw.reserve(3);

	for (Vertex& vertc : vertices_world)
	{
		vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc.position) });
	}

	//Apply Projection Matrix to get NDC Space
	std::vector<Vertex> vertices_NDC{};
	vertices_NDC.reserve(3);
	VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

	//NDC to rasterSpace
	std::vector<Vector2> vector2_Screen{};
	VertectTransformToScreen(vertices_NDC, vector2_Screen);

	int pxl{-1};
	//check for every pxl if in triangle 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			pxl++;
			Vector2 pxlScr{ px + 0.5f, py + 0.5f };

			////debug lines
			//if (px > m_Width / 2 && py > m_Height / 2.5)
			//{
			//	std::cout << "het\n";
			//}

			//check for every triangle
			for (size_t indc{0}; indc < vector2_Screen.size(); indc += 3)
			{
				const float W2 = Vector2::Cross(pxlScr - vector2_Screen[indc + 0], vector2_Screen[indc + 1] - vector2_Screen[indc + 0]);
				const float W0 = Vector2::Cross(pxlScr - vector2_Screen[indc + 1], vector2_Screen[indc + 2] - vector2_Screen[indc + 1]);
				const float W1 = Vector2::Cross(pxlScr - vector2_Screen[indc + 2], vector2_Screen[indc + 0] - vector2_Screen[indc + 2]);

				if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
				{
					const float W = Vector2::Cross(vector2_Screen[1] - vector2_Screen[2], vector2_Screen[0] - vector2_Screen[2]);

					//Get avg depth DepthCheck
					const float depth0 = W0 / W * vertices_NDC[indc + 0].position.z;
					const float depth1 = W1 / W * vertices_NDC[indc + 1].position.z;
					const float depth2 = W2 / W * vertices_NDC[indc + 2].position.z;
					const float avgDepth{ (depth0 + depth1 + depth2) / 3.f };

					//if pxl is closer to camera, give color of triangleVertex
					if (m_pDepthBufferPixels[pxl] > avgDepth)
					{
						finalColor = {
							W0 / W * vertices_world[indc + 0].color.r  +  W1 / W * vertices_world[indc + 1].color.r  +  W2 / W * vertices_world[indc + 2].color.r,
							W0 / W * vertices_world[indc + 0].color.g  +  W1 / W * vertices_world[indc + 1].color.g  +  W2 / W * vertices_world[indc + 2].color.g,
							W0 / W * vertices_world[indc + 0].color.b  +  W1 / W * vertices_world[indc + 1].color.b  +  W2 / W * vertices_world[indc + 2].color.b

						};
						m_pDepthBufferPixels[pxl] = avgDepth;
					}

				}
				
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));

			
			//reset for next pxl
			finalColor = {};
		}
	}
			ResetDepthBuffer();
}

void Renderer::Render_W1_5()
{
	//SDL_Rect rect = SDL_Rect{ 0, 0, m_Width, m_Height };
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	ColorRGB finalColor{  };

	//World Space
	std::vector<Vertex> vertices_world{
		//Triangle 0
		{{  0.f,  2.f,  0.f },  {1.0f, 0.0f, 0.0f} },
		{{ 1.5f, -1.0f, 0.f },  {1.0f, 0.0f, 0.0f} },
		{{ -1.5f, -1.0f, 0.f }, {1.0f, 0.0f, 0.0f} },

		//Triangle 1
		{{  0.f, 4.f,  2.f },  {1.0f, 0.0f, 0.0f} },
		{{ 3.f, -2.0f, 2.f },  {0.0f, 1.0f, 0.0f} },
		{{ -3.f, -2.0f, 2.f }, {0.0f, 0.0f, 1.0f} }

	};

	//Apply vieuw matrix to get Vieuw Space
	std::vector<Vertex> vertices_Vieuw{};
	vertices_Vieuw.reserve(3);

	for (Vertex& vertc : vertices_world)
	{
		vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc.position) });
	}

	//Apply Projection Matrix to get NDC Space
	std::vector<Vertex> vertices_NDC{};
	vertices_NDC.reserve(3);
	VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

	//NDC to rasterSpace
	std::vector<Vector2> vector2_Screen{};
	VertectTransformToScreen(vertices_NDC, vector2_Screen);

	int pxl{ -1 };
	//check for every pxl if in triangle 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			pxl++;
			Vector2 pxlScr{ px + 0.5f, py + 0.5f };

			//check for every triangle
			for (size_t indc{ 0 }; indc < vector2_Screen.size(); indc += 3)
			{
				//check if in bounds
				if (!IsInBoundingBox(pxlScr, indc, vector2_Screen))
					continue;		

				//get area between vector  vertix i ->pixel and vertix i -> vertix i+1
				const float W2 = Vector2::Cross(pxlScr - vector2_Screen[indc + 0], vector2_Screen[indc + 1] - vector2_Screen[indc + 0]);
				const float W0 = Vector2::Cross(pxlScr - vector2_Screen[indc + 1], vector2_Screen[indc + 2] - vector2_Screen[indc + 1]);
				const float W1 = Vector2::Cross(pxlScr - vector2_Screen[indc + 2], vector2_Screen[indc + 0] - vector2_Screen[indc + 2]);

				if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
				{
					//const float W = W0 + W1 + W2;
					const float W = Vector2::Cross(vector2_Screen[indc + 1] - vector2_Screen[indc + 2], vector2_Screen[indc + 0] - vector2_Screen[indc + 2]);

					//Get avg depth DepthCheck
					const float depth0 = W0 / W * vertices_NDC[indc + 0].position.z;
					const float depth1 = W1 / W * vertices_NDC[indc + 1].position.z;
					const float depth2 = W2 / W * vertices_NDC[indc + 2].position.z;
					const float avgDepth{ (depth0 + depth1 + depth2) / 3.f };

					//if pxl is closer to camera, give color of triangleVertex
					if (m_pDepthBufferPixels[pxl] > avgDepth)
					{
						finalColor = {
							W0 / W * vertices_world[indc + 0].color.r + W1 / W * vertices_world[indc + 1].color.r + W2 / W * vertices_world[indc + 2].color.r,
							W0 / W * vertices_world[indc + 0].color.g + W1 / W * vertices_world[indc + 1].color.g + W2 / W * vertices_world[indc + 2].color.g,
							W0 / W * vertices_world[indc + 0].color.b + W1 / W * vertices_world[indc + 1].color.b + W2 / W * vertices_world[indc + 2].color.b

						};
						m_pDepthBufferPixels[pxl] = avgDepth;
					}

				}

			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));


			//reset for next pxl
			finalColor = {};
		}
	}
	ResetDepthBuffer();
}

void dae::Renderer::Render_W2_1()
{

	//World Space
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{{-3,  3, -2}, {0.0f, 1.f, 1.f}, {0   , 0   }},
				Vertex{{ 0,  3, -2}, {0.0f, 1.f, 1.f}, {0.5f, 0   }},
				Vertex{{ 3,  3, -2}, {0.0f, 1.f, 1.f}, {1   , 0   }},
				Vertex{{-3,  0, -2}, {0.0f, 1.f, 1.f}, {0   , 0.5f}},
				Vertex{{ 0,  0, -2}, {0.0f, 1.f, 1.f}, {0.5f, 0.5f}},
				Vertex{{ 3,  0, -2}, {0.0f, 1.f, 1.f}, {1   , 0.5f}},
				Vertex{{-3, -3, -2}, {0.0f, 1.f, 1.f}, {0   , 1   }},
				Vertex{{ 0, -3, -2}, {0.0f, 1.f, 1.f}, {0.5f, 1   }},
				Vertex{{ 3, -3, -2}, {0.0f, 1.f, 1.f}, {1   , 1   }}
			},
			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},
			PrimitiveTopology::TriangleStrip
		},
		Mesh
		{
			{
				Vertex{{-5,  3, 3}, {1.0f, 1.f, 0.f}, {0   , 0   }},
				Vertex{{ 0,  3, 3}, {1.0f, 1.f, 0.f}, {0.5f, 0   }},
				Vertex{{ 1,  3, 3}, {1.0f, 1.f, 0.f}, {1   , 0   }},
				Vertex{{-5,  0, 3}, {1.0f, 1.f, 0.f}, {0   , 0.5f}},
				Vertex{{ 0,  0, 3}, {1.0f, 1.f, 0.f}, {0.5f, 0.5f}},
				Vertex{{ 1,  0, 3}, {1.0f, 1.f, 0.f}, {1   , 0.5f}},
				Vertex{{-5, -6, 3}, {1.0f, 1.f, 0.f}, {0   , 1   }},
				Vertex{{ 0, -6, 3}, {1.0f, 1.f, 0.f}, {0.5f, 1   }},
				Vertex{{ 1, -6, 3}, {1.0f, 1.f, 0.f}, {1   , 1   }}
			},
			{
				3, 0, 1,    1, 4, 3,    4, 1, 2,
				2, 5, 4,    6, 3, 4,    4, 7, 6,
				7, 4, 5,    5, 8, 7
			},
			PrimitiveTopology::TriangleList
		}
	};

	ColorRGB finalColor{ colors::Negative };
	
	//////////////////////////////////////////////////////////////////////////////////
	//Check every Mesh
	/////////////////////////////////////////////////////////////////////////////////
	for (Mesh& mesh : meshes_world)
	{
		bool isColored{ false };//-> get value from depthBuffer if pixel is already colored  

		/////////////////////////////////////////////////////////////////////////////
		//Apply vieuw matrix to get Vieuw Space
		std::vector<Vertex> vertices_Vieuw{};
		vertices_Vieuw.reserve(3);

		for (Vertex& vertc : mesh.vertices)
			vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc.position) });
		

		/////////////////////////////////////////////////////////////////////////////
		//Apply Projection Matrix to get NDC Space
		std::vector<Vertex> vertices_NDC{};
		vertices_NDC.reserve(3);
		VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

		////////////////////////////////////////////////////////////////////////////
		//NDC to rasterSpace
		std::vector<Vector2> vector2_Screen{};
		VertectTransformToScreen(vertices_NDC, vector2_Screen);

		int pxl{ -1 };
		//////////////////////////////////////////////////////////////////////////
		//check for every pxl if in Mesh

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				//pixel position and index
				pxl++;
				Vector2 pxlScr{ px + 0.5f, py + 0.5f };

				//////debug lines
				//if (px > 300  && py > 400 && mesh.primitiveTopology == PrimitiveTopology::TriangleList)
				//{
				//	std::cout << "het\n";
				//}

				//////////////////////////////////////////////////////////////////////////////
				//Check if pixel is in Mesh 
				//Checked by the type off mesh inserted
				// return color off the Vertex where current pixel is closest to (by weight)
				//////////////////////////////////////////////////////////////////////////////
				if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
				{

					//check for every triangle by its indices
					for (size_t indc{ 0 }; indc < mesh.indices.size(); indc += 3)
					{
						//check if in bounds -> check bounds for mesh ->works but its slower
						if (!IsInBoundingBox(pxlScr, indc, vector2_Screen, mesh))
							continue;

						//get area between vector  vertix i ->pixel and vertix i -> vertix i+1
						const float W2 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 0]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 0]]);
						const float W0 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 1]], vector2_Screen[mesh.indices[indc + 2]] - vector2_Screen[mesh.indices[indc + 1]]);
						const float W1 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]);

						if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
						{
							//const float W = Vector2::Cross(vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]);
							const float W = W0 + W1 + W2;

							//Get avg depth DepthCheck
							const float depth0 = W0 / W * vertices_NDC[mesh.indices[indc + 0]].position.z;
							const float depth1 = W1 / W * vertices_NDC[mesh.indices[indc + 1]].position.z;
							const float depth2 = W2 / W * vertices_NDC[mesh.indices[indc + 2]].position.z;
							const float avgDepth{ (depth0 + depth1 + depth2) / 3.f };

							////if pxl is closer to camera, give color of triangleVertex
							if (m_pDepthBufferPixels[pxl] > avgDepth)
							{
								const float zInterpolated{ 1.0f / (
									  ( (W0 / W) / vertices_NDC[mesh.indices[indc + 0]].position.z)
									+ ( (W1 / W) / vertices_NDC[mesh.indices[indc + 1]].position.z)
									+ ( (W2 / W) / vertices_NDC[mesh.indices[indc + 2]].position.z)
										) };
								
								//value by uv
								Vector2 uv{(
									  mesh.vertices[mesh.indices[indc + 0]].uv * (W0 / W) /vertices_NDC[mesh.indices[indc + 0]].position.z 
									+ mesh.vertices[mesh.indices[indc + 1]].uv * (W1 / W) /vertices_NDC[mesh.indices[indc + 1]].position.z 
									+ mesh.vertices[mesh.indices[indc + 2]].uv * (W2 / W) /vertices_NDC[mesh.indices[indc + 2]].position.z 
									  ) * zInterpolated
								};
								
								finalColor = m_pTextureNormalMap->Sample(uv);			
								m_pDepthBufferPixels[pxl] = avgDepth;
							}
							
						}
						
					}//end for each triangle
				
				}//end if mesh is triangleList

				else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
				{

					//check for every indice
					for (size_t indc{ 0 }; indc < mesh.indices.size() - 2; indc++)
					{
						////check if in bounds -> check bounds for mesh ->works but its slower
						//if (!IsInBoundingBox(pxlScr, indc, vector2_Screen, mesh))
						//	continue;

						//get area between vector  vertix i ->pixel and vertix i -> vertix i+1

						float W2 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 0]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 0]]);
						float W0 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 1]], vector2_Screen[mesh.indices[indc + 2]] - vector2_Screen[mesh.indices[indc + 1]]);
						float W1 = Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]);

						if (indc % 2 != 0)
						{
							W0 = -W0;
							W1 = -W1;
							W2 = -W2;
						}
						//if pixel in triangle
						if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
						{
							const float sign{ (indc % 2 == 0) ? 1.f : -1.f };
							//const float W = sign * Vector2::Cross(vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]);
							const float W = (W0 + W1 + W2);

							//Get avg depth DepthCheck
							const float depth0 = W0 / W * vertices_NDC[mesh.indices[indc + 0]].position.z;
							const float depth1 = W1 / W * vertices_NDC[mesh.indices[indc + 1]].position.z;
							const float depth2 = W2 / W * vertices_NDC[mesh.indices[indc + 2]].position.z;
							const float avgDepth{ (depth0 + depth1 + depth2) / 3.f };

							////if pxl is closer to camera, give color of triangleVertex
							if (m_pDepthBufferPixels[pxl] > avgDepth)
							{
								const float zInterpolated{ 1.0f / (
									  ( (W0 / W) / vertices_NDC[mesh.indices[indc + 0]].position.z)
									+ ( (W1 / W) / vertices_NDC[mesh.indices[indc + 1]].position.z)
									+ ( (W2 / W) / vertices_NDC[mesh.indices[indc + 2]].position.z)
										) };

								//value by uv
								Vector2 uv{ (
									  mesh.vertices[mesh.indices[indc + 0]].uv * (W0 / W) / vertices_NDC[mesh.indices[indc + 0]].position.z
									+ mesh.vertices[mesh.indices[indc + 1]].uv * (W1 / W) / vertices_NDC[mesh.indices[indc + 1]].position.z
									+ mesh.vertices[mesh.indices[indc + 2]].uv * (W2 / W) / vertices_NDC[mesh.indices[indc + 2]].position.z
									  ) * zInterpolated
								};

								finalColor = m_pTexture->Sample(uv);
								m_pDepthBufferPixels[pxl] = avgDepth;
							}

						}
						else
							continue;
						
					
					}//end for each pixel
				
				}//end if mesh is triangleStrip

				/////////////////////////////////////////////////////////////////////////////
				//Update Color in Buffer for current mesh
				/////////////////////////////////////////////////////////////////////////////
				finalColor.MaxToOne();

				//if already colored and no closer hit from current pixel -> keep previous pixel 
				if (finalColor == colors::Negative /*&& m_pDepthBufferPixels[pxl] < std::numeric_limits<float>::max() - 1.0f*/)
					isColored = true;
				else
					isColored = false;

				if (!isColored )
				{
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}

				finalColor = colors::Negative;
			
			}//end for py
		}//end for px
	}//end for each Mesh
	ResetDepthBuffer();
}

void dae::Renderer::Render_W2_2()
{

	//World Space
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{{-2,  2, -3}, {0.0f, 1.f, 1.f}, {0   , 0   }},
				Vertex{{ 0,  2, -3}, {0.0f, 1.f, 1.f}, {0.5f, 0   }},
				Vertex{{ 2,  2, -3}, {0.0f, 1.f, 1.f}, {1   , 0   }},
				Vertex{{-2,  0, -3}, {0.0f, 1.f, 1.f}, {0   , 0.5f}},
				Vertex{{ 0,  0, -3}, {0.0f, 1.f, 1.f}, {0.5f, 0.5f}},
				Vertex{{ 2,  0, -3}, {0.0f, 1.f, 1.f}, {1   , 0.5f}},
				Vertex{{-2, -2, -3}, {0.0f, 1.f, 1.f}, {0   , 1   }},
				Vertex{{ 0, -2, -3}, {0.0f, 1.f, 1.f}, {0.5f, 1   }},
				Vertex{{ 2, -2, -3}, {0.0f, 1.f, 1.f}, {1   , 1   }}
			},
			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},
			PrimitiveTopology::TriangleStrip
		},
		Mesh
		{
			{
				Vertex{{-3,  3, -2}, {0.0f, 1.f, 1.f}, {0   , 0   }},
				Vertex{{ 0,  3, -2}, {0.0f, 1.f, 1.f}, {0.5f, 0   }},
				Vertex{{ 3,  3, -2}, {0.0f, 1.f, 1.f}, {1   , 0   }},
				Vertex{{-3,  0, -2}, {0.0f, 1.f, 1.f}, {0   , 0.5f}},
				Vertex{{ 0,  0, -2}, {0.0f, 1.f, 1.f}, {0.5f, 0.5f}},
				Vertex{{ 3,  0, -2}, {0.0f, 1.f, 1.f}, {1   , 0.5f}},
				Vertex{{-3, -3, -2}, {0.0f, 1.f, 1.f}, {0   , 1   }},
				Vertex{{ 0, -3, -2}, {0.0f, 1.f, 1.f}, {0.5f, 1   }},
				Vertex{{ 3, -3, -2}, {0.0f, 1.f, 1.f}, {1   , 1   }}
			},
			{
				3, 0, 1,    1, 4, 3,    4, 1, 2,
				2, 5, 4,    6, 3, 4,    4, 7, 6,
				7, 4, 5,    5, 8, 7
			},
			PrimitiveTopology::TriangleList
		}
	};

	ColorRGB finalColor{ colors::Negative };
	
	//////////////////////////////////////////////////////////////////////////////////
	//Check every Mesh
	/////////////////////////////////////////////////////////////////////////////////
	for (Mesh& mesh : meshes_world)
	{
		bool isColored{ false };//-> get value from depthBuffer if pixel is already colored  

		/////////////////////////////////////////////////////////////////////////////
		//Apply vieuw matrix to get Vieuw Space
		std::vector<Vertex> vertices_Vieuw{};
		vertices_Vieuw.reserve(mesh.vertices.size());

		for (Vertex& vertc : mesh.vertices)
			vertices_Vieuw.push_back(Vertex{ m_Camera.viewMatrix.TransformPoint(vertc.position) });


		/////////////////////////////////////////////////////////////////////////////
		//Apply Projection Matrix to get NDC Space
		std::vector<Vertex> vertices_NDC{};
		vertices_NDC.reserve(mesh.vertices.size());
		VertexTransformationFunction(vertices_Vieuw, vertices_NDC);

		////////////////////////////////////////////////////////////////////////////
		//NDC to rasterSpace
		std::vector<Vector2> vector2_Screen{};
		vector2_Screen.reserve(mesh.vertices.size());
		VertectTransformToScreen(vertices_NDC, vector2_Screen);


		//////////////////////////////////////////////////////////////////////////
		//loop through every triangle of current mesh
		int increment{};
		int sizeReducer{};
		bool invertEven{};
		switch(mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
			increment   = 3;
			sizeReducer = 0;
			invertEven  = false;
			break;
		case PrimitiveTopology::TriangleStrip:
			increment   = 1;
			sizeReducer = 2;
			invertEven  = true;
			break;
		default:
			std::cout << "invallid triangle type\n";
			break;
		}
		

		for (size_t indc{ 0 }; indc < mesh.indices.size() - sizeReducer; indc += increment)
		{
			//check bounds off current mesh
			int left  {int ( std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x) )};
			int top   {int ( std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y)) };
			int right {int ( std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x)) };
			int bottom{int ( std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y)) };

			left   = Clamp(left, 0, m_Width);
			top    = Clamp(top, 0, m_Height);
			right  = Clamp(right, 0, m_Width);
			bottom = Clamp(bottom, 0, m_Height);


			//Check for every pxl if in current triangle
			for (int px{left}; px < right; ++px)
			{
				for (int py{top}; py < bottom; ++py)
				{
					//pixel position and index
					int pxl{ px + py * m_Width };
					Vector2 pxlScr{ px + 0.5f, py + 0.5f };

					//invert Weights when using triangleStrips
					int inverter{ (invertEven && indc % 2 != 0)? -1 : 1};

					const float W =  inverter*Vector2::Cross(vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 2]]);
					const float W2 = inverter*Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 0]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 0]])/W;
					const float W0 = inverter*Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 1]], vector2_Screen[mesh.indices[indc + 2]] - vector2_Screen[mesh.indices[indc + 1]])/W;
					const float W1 = inverter*Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]])/W;

					if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
					{
						const float zInterpolated{ 1.0f / (
							  ( (W0 ) / vertices_NDC[mesh.indices[indc + 0]].position.z)
							+ ( (W1 ) / vertices_NDC[mesh.indices[indc + 1]].position.z)
							+ ( (W2 ) / vertices_NDC[mesh.indices[indc + 2]].position.z)
								) };

						if (m_pDepthBufferPixels[pxl] > zInterpolated)
						{
							Vector2 uv{(
								  mesh.vertices[mesh.indices[indc + 0]].uv * (W0 ) /vertices_NDC[mesh.indices[indc + 0]].position.z 
								+ mesh.vertices[mesh.indices[indc + 1]].uv * (W1 ) /vertices_NDC[mesh.indices[indc + 1]].position.z 
								+ mesh.vertices[mesh.indices[indc + 2]].uv * (W2 ) /vertices_NDC[mesh.indices[indc + 2]].position.z 
								  ) * zInterpolated
							};
							
							finalColor = m_pTexture->Sample(uv);			
							m_pDepthBufferPixels[pxl] = zInterpolated;
						}						
					}//end if pxl in triangle

					/////////////////////////////////////////////////////////////////////////////
					//Update Color in Buffer for current mesh
					/////////////////////////////////////////////////////////////////////////////
					finalColor.MaxToOne();

					if (finalColor == colors::Negative)
						isColored = true;
					else
						isColored = false;

					if (!isColored )
					{
						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				
					//reset for next pxl
					finalColor = colors::Negative;
		
				}//end for py
			
			}//end for px
		
		}//end for each triangle

	
	}//end for each Mesh
	ResetDepthBuffer();
}

void dae::Renderer::Render_W3_1()
{
	////World Space
	//std::vector<Mesh> m_Meshes_world
	//{
	//	Mesh
	//	{
	//		{
	//			Vertex{{-2,  2, -1.0f}, {0.0f, 1.f, 1.f}, {0   , 0   }},
	//			Vertex{{ 0,  2, -1.0f}, {0.0f, 1.f, 1.f}, {0.5f, 0   }},
	//			Vertex{{ 2,  2, -1.0f}, {0.0f, 1.f, 1.f}, {1   , 0   }},
	//			Vertex{{-2,  0, -1.0f}, {0.0f, 1.f, 1.f}, {0   , 0.5f}},
	//			Vertex{{ 0,  0, -1.0f}, {0.0f, 1.f, 1.f}, {0.5f, 0.5f}},
	//			Vertex{{ 2,  0, -1.0f}, {0.0f, 1.f, 1.f}, {1   , 0.5f}},
	//			Vertex{{-2, -2, -1.0f}, {0.0f, 1.f, 1.f}, {0   , 1   }},
	//			Vertex{{ 0, -2, -1.0f}, {0.0f, 1.f, 1.f}, {0.5f, 1   }},
	//			Vertex{{ 2, -2, -1.0f}, {0.0f, 1.f, 1.f}, {1   , 1   }}
	//		},
	//		{
	//			3, 0, 4, 1, 5, 2,
	//			2, 6,
	//			6, 3, 7, 4, 8, 5
	//		},
	//		PrimitiveTopology::TriangleStrip,
	//
	//	},
	//	Mesh
	//	{
	//		{
	//			Vertex{{-3,  3, 80}, {0.0f, 1.f, 1.f}, {0   , 0   }},
	//			Vertex{{ 0,  3, 80}, {0.0f, 1.f, 1.f}, {0.5f, 0   }},
	//			Vertex{{ 3,  3, 80}, {0.0f, 1.f, 1.f}, {1   , 0   }},
	//			Vertex{{-3,  0, 80}, {0.0f, 1.f, 1.f}, {0   , 0.5f}},
	//			Vertex{{ 0,  0, 80}, {0.0f, 1.f, 1.f}, {0.5f, 0.5f}},
	//			Vertex{{ 3,  0, 80}, {0.0f, 1.f, 1.f}, {1   , 0.5f}},
	//			Vertex{{-3, -3, 80}, {0.0f, 1.f, 1.f}, {0   , 1   }},
	//			Vertex{{ 0, -3, 80}, {0.0f, 1.f, 1.f}, {0.5f, 1   }},
	//			Vertex{{ 3, -3, 80}, {0.0f, 1.f, 1.f}, {1   , 1   }}
	//		},
	//		{
	//			3, 0, 1,    1, 4, 3,    4, 1, 2,
	//			2, 5, 4,    6, 3, 4,    4, 7, 6,
	//			7, 4, 5,    5, 8, 7
	//		},
	//		PrimitiveTopology::TriangleList
	//	}
	//};
	
	ColorRGB finalColor{ colors::Negative };

	//////////////////////////////////////////////////////////////////////////////////
	//Check every Mesh
	/////////////////////////////////////////////////////////////////////////////////
	for (Mesh& mesh : m_Meshes_world)
	{
		bool isColored{ false };//-> get value from depthBuffer if pixel is already colored  
		
		//World to NDCSpace
		std::vector<Vector4> vertices_NDC{};
		vertices_NDC.reserve(mesh.vertices.size());
		ViewProjectionToNDC(mesh.vertices, vertices_NDC);

		
		//NDC to RasterSpace
		std::vector<Vector2> vector2_Screen{};
		vector2_Screen.reserve(mesh.vertices.size());
		VertectTransformToScreen(vertices_NDC, vector2_Screen);

		//////////////////////////////////////////////////////////////////////////
		//loop through every triangle of current mesh
		
		//check triangleType and adjust loop variables 
		int increment{};
		int sizeReducer{};
		bool invertEven{};
		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
			increment = 3;
			sizeReducer = 0;
			invertEven = false;
			break;
		case PrimitiveTopology::TriangleStrip:
			increment = 1;
			sizeReducer = 2;
			invertEven = true;
			break;
		default:
			std::cout << "invallid triangle type\n";
			break;
		}

		for (size_t indc{ 0 }; indc < mesh.indices.size() - sizeReducer; indc += increment)
		{
			//check if triangle is in frustom
			if (vertices_NDC[mesh.indices[indc + 0]].x < -1.0f || vertices_NDC[mesh.indices[indc + 0]].x > 1.0f || vertices_NDC[mesh.indices[indc + 1]].x < -1.0f || vertices_NDC[mesh.indices[indc + 1]].x > 1.0f || vertices_NDC[mesh.indices[indc + 2]].x < -1.0f || vertices_NDC[mesh.indices[indc + 2]].x > 1.0f ||
				vertices_NDC[mesh.indices[indc + 0]].y < -1.0f || vertices_NDC[mesh.indices[indc + 0]].y > 1.0f || vertices_NDC[mesh.indices[indc + 1]].y < -1.0f || vertices_NDC[mesh.indices[indc + 1]].y > 1.0f || vertices_NDC[mesh.indices[indc + 2]].y < -1.0f || vertices_NDC[mesh.indices[indc + 2]].y > 1.0f ||
				vertices_NDC[mesh.indices[indc + 0]].z <    0  || vertices_NDC[mesh.indices[indc + 0]].z > 1.0f || vertices_NDC[mesh.indices[indc + 1]].z <  0    || vertices_NDC[mesh.indices[indc + 1]].z > 1.0f || vertices_NDC[mesh.indices[indc + 2]].z < 0     || vertices_NDC[mesh.indices[indc + 2]].z > 1.0f   )
				continue;
				


			//check bounds off current mesh
			const int left  { Clamp(int(std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x) - 1), 0, m_Width) };
			const int top   { Clamp(int(std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y) - 1), 0, m_Height) };
			const int right { Clamp(int(std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x) + 1), 0, m_Width) };
			const int bottom{ Clamp(int(std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y) + 1), 0, m_Height) };

			//Check for every pxl if in current triangle
			for (int px{ left }; px < right; ++px)
			{
				for (int py{ top }; py < bottom; ++py)
				{
					//pixel position and index
					int pxl{ px + py * m_Width };
					Vector2 pxlScr{ px + 0.5f, py + 0.5f };

					//invert Weights when using triangleStrips
					int inverter{ (invertEven && indc % 2 != 0) ? -1 : 1 };
					const float W  = inverter * Vector2::Cross(vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 2]]);
					const float W2 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 0]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 0]]) / W;
					const float W0 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 1]], vector2_Screen[mesh.indices[indc + 2]] - vector2_Screen[mesh.indices[indc + 1]]) / W;
					const float W1 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]) / W;

					//if pxl in current triangle, check depth
					if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
					{
						const float zBufferValue
						{ -1.0f / (
							  ((W0) / vertices_NDC[mesh.indices[indc + 0]].z)
							+ ((W1) / vertices_NDC[mesh.indices[indc + 1]].z)
							+ ((W2) / vertices_NDC[mesh.indices[indc + 2]].z)
						) };

						//if pixel not in frustum, check next pxl
						if (zBufferValue < 0.0f || zBufferValue > 1.0f)
							continue;

						//Compare with DepthBuffer
						if (m_pDepthBufferPixels[pxl] > zBufferValue)
						{
							const float zInterpolated
							{ 1.0f / (
								  ((W0) / vertices_NDC[mesh.indices[indc + 0]].w)
								+ ((W1) / vertices_NDC[mesh.indices[indc + 1]].w)
								+ ((W2) / vertices_NDC[mesh.indices[indc + 2]].w)
							) };

							//Color pixel depending on cameraMode							
							float v = Remap(zBufferValue, 0.995f, 1.f);
							
							m_pDepthBufferPixels[pxl] = zBufferValue;
						}
					}//end if pxl in triangle

					/////////////////////////////////////////////////////////////////////////////
					//Update Color in Buffer for current mesh
					/////////////////////////////////////////////////////////////////////////////
					finalColor.MaxToOne();

					if (finalColor == colors::Negative)
						isColored = true;
					else
						isColored = false;

					if (!isColored)
					{
						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}

					//reset for next pxl
					finalColor = colors::Negative;

				}//end for py

			}//end for px

		}//end for each triangle

	}//end for each Mesh
	ResetDepthBuffer();
}

void dae::Renderer::Render_W4_1()
{
	
	ColorRGB finalColor{ colors::Negative };

	//////////////////////////////////////////////////////////////////////////////////
	//Check every Mesh
	/////////////////////////////////////////////////////////////////////////////////
	for (Mesh& mesh : m_Meshes_world)
	{
		bool isColored{ false };//-> get value from depthBuffer if pixel is already colored  

		//World to NDCSpace
		std::vector<Vertex_Out> vertices_NDC{};
		vertices_NDC.reserve(mesh.vertices.size());
		ViewProjectionToNDC(mesh, vertices_NDC);

		//NDC to RasterSpace
		std::vector<Vector2> vector2_Screen{};
		vector2_Screen.reserve(mesh.vertices.size());
		VertectTransformToScreen(vertices_NDC, vector2_Screen);

		//////////////////////////////////////////////////////////////////////////
		//loop through every triangle of current mesh

		//check triangleType and adjust loop variables 
		int increment{};
		int sizeReducer{};
		bool invertEven{};
		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
			increment = 3;
			sizeReducer = 0;
			invertEven = false;
			break;
		case PrimitiveTopology::TriangleStrip:
			increment = 1;
			sizeReducer = 2;
			invertEven = true;
			break;
		default:
			std::cout << "invallid triangle type\n";
			break;
		}

		for (size_t indc{ 0 }; indc < mesh.indices.size() - sizeReducer; indc += increment)
		{
			//check if triangle is in frustom
			if (vertices_NDC[mesh.indices[indc + 0]].position.x < -1.0f || vertices_NDC[mesh.indices[indc + 0]].position.x > 1.0f || vertices_NDC[mesh.indices[indc + 1]].position.x < -1.0f || vertices_NDC[mesh.indices[indc + 1]].position.x > 1.0f || vertices_NDC[mesh.indices[indc + 2]].position.x < -1.0f || vertices_NDC[mesh.indices[indc + 2]].position.x > 1.0f ||
				vertices_NDC[mesh.indices[indc + 0]].position.y < -1.0f || vertices_NDC[mesh.indices[indc + 0]].position.y > 1.0f || vertices_NDC[mesh.indices[indc + 1]].position.y < -1.0f || vertices_NDC[mesh.indices[indc + 1]].position.y > 1.0f || vertices_NDC[mesh.indices[indc + 2]].position.y < -1.0f || vertices_NDC[mesh.indices[indc + 2]].position.y > 1.0f ||
				vertices_NDC[mesh.indices[indc + 0]].position.z < 0     || vertices_NDC[mesh.indices[indc + 0]].position.z > 1.0f || vertices_NDC[mesh.indices[indc + 1]].position.z < 0     || vertices_NDC[mesh.indices[indc + 1]].position.z > 1.0f || vertices_NDC[mesh.indices[indc + 2]].position.z < 0     || vertices_NDC[mesh.indices[indc + 2]].position.z > 1.0f)
				continue;
			

#pragma region BoundingBox
			//check bounds off current triangle
			const int left{ Clamp(int(std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x) - 1), 0, m_Width) };
			const int top{ Clamp(int(std::min(std::min(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y) - 1), 0, m_Height) };
			const int right{ Clamp(int(std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].x, vector2_Screen[mesh.indices[indc + 1]].x), vector2_Screen[mesh.indices[indc + 2]].x) + 1), 0, m_Width) };
			const int bottom{ Clamp(int(std::max(std::max(vector2_Screen[mesh.indices[indc + 0]].y, vector2_Screen[mesh.indices[indc + 1]].y), vector2_Screen[mesh.indices[indc + 2]].y) + 1), 0, m_Height) };
#pragma endregion BoundingBox calulations

			//Calculate area off current triangle and check if it is a line;
			int inverter{ (invertEven && indc % 2 != 0) ? -1 : 1 };//invert Weights when using triangleStrips
			const float W  = inverter * Vector2::Cross(vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 2]]);
			if (W <= 0.0001f && W >= -0.0001f)continue;

			//Check for every pxl off the boundingBox if in current triangle
			for (int px{ left }; px < right; ++px)
			{
				for (int py{ top }; py < bottom; ++py)
				{
					//pixel position and index
					int pxl{ px + py * m_Width };
					Vector2 pxlScr{ px + 0.5f, py + 0.5f };

					//Calculate total area off current triangle and the weight off every corner
					const float W2 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 0]], vector2_Screen[mesh.indices[indc + 1]] - vector2_Screen[mesh.indices[indc + 0]]) / W;
					const float W0 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 1]], vector2_Screen[mesh.indices[indc + 2]] - vector2_Screen[mesh.indices[indc + 1]]) / W;
					const float W1 = inverter * Vector2::Cross(pxlScr - vector2_Screen[mesh.indices[indc + 2]], vector2_Screen[mesh.indices[indc + 0]] - vector2_Screen[mesh.indices[indc + 2]]) / W;

					//if pxl in current triangle, check depth
					if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
					{
						const float zBufferValue
						{ -1.0f / (
							  ((W0) / vertices_NDC[mesh.indices[indc + 0]].position.w)
							+ ((W1) / vertices_NDC[mesh.indices[indc + 1]].position.w)
							+ ((W2) / vertices_NDC[mesh.indices[indc + 2]].position.w)
						) };

						//Compare with DepthBuffer
						if (m_pDepthBufferPixels[pxl] > zBufferValue)
						{
							const float zInterpolated
							{ 1.0f / (
								  ((W0) / vertices_NDC[mesh.indices[indc + 0]].position.w)
								+ ((W1) / vertices_NDC[mesh.indices[indc + 1]].position.w)
								+ ((W2) / vertices_NDC[mesh.indices[indc + 2]].position.w)
							) };

							m_pDepthBufferPixels[pxl] = zBufferValue;

							//-----------------------------------------------------------------------------


							//Interpolate vertex for shading
							////////////////////////////////////////////////////////////////
#pragma region Interpolation 
							Vertex_Out interpolatedVertex{};
							interpolatedVertex.uv = { (
									  vertices_NDC[mesh.indices[indc + 0]].uv * (W0) / vertices_NDC[mesh.indices[indc + 0]].position.w
									+ vertices_NDC[mesh.indices[indc + 1]].uv * (W1) / vertices_NDC[mesh.indices[indc + 1]].position.w
									+ vertices_NDC[mesh.indices[indc + 2]].uv * (W2) / vertices_NDC[mesh.indices[indc + 2]].position.w
									  ) * zInterpolated
								};

							interpolatedVertex.normal = { (
									  vertices_NDC[mesh.indices[indc + 0]].normal * (W0) / vertices_NDC[mesh.indices[indc + 0]].position.w
									+ vertices_NDC[mesh.indices[indc + 1]].normal * (W1) / vertices_NDC[mesh.indices[indc + 1]].position.w
									+ vertices_NDC[mesh.indices[indc + 2]].normal * (W2) / vertices_NDC[mesh.indices[indc + 2]].position.w
									  ) * zInterpolated
								};
							interpolatedVertex.normal.Normalize();

							interpolatedVertex.tangent = { (
									  vertices_NDC[mesh.indices[indc + 0]].tangent * (W0) / vertices_NDC[mesh.indices[indc + 0]].position.w
									+ vertices_NDC[mesh.indices[indc + 1]].tangent * (W1) / vertices_NDC[mesh.indices[indc + 1]].position.w
									+ vertices_NDC[mesh.indices[indc + 2]].tangent * (W2) / vertices_NDC[mesh.indices[indc + 2]].position.w
									  ) * zInterpolated
								};
							interpolatedVertex.tangent.Normalize();

							interpolatedVertex.viewDirection = { (
									  vertices_NDC[mesh.indices[indc + 0]].viewDirection* (W0) / vertices_NDC[mesh.indices[indc + 0]].position.w
									+ vertices_NDC[mesh.indices[indc + 1]].viewDirection * (W1) / vertices_NDC[mesh.indices[indc + 1]].position.w
									+ vertices_NDC[mesh.indices[indc + 2]].viewDirection * (W2) / vertices_NDC[mesh.indices[indc + 2]].position.w
									  ) * zInterpolated
							};
							interpolatedVertex.viewDirection.Normalize();
#pragma endregion Interpolatin 
							

							finalColor = ShadePxl(interpolatedVertex);
						}
					}//end if pxl in triangle

					/////////////////////////////////////////////////////////////////////////////
					//Update Color in Buffer for current mesh
					/////////////////////////////////////////////////////////////////////////////
					finalColor.MaxToOne();

					if (finalColor == colors::Negative)
						isColored = true;
					else
						isColored = false;

					if (!isColored)
					{
						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}

					//reset for next pxl
					finalColor = colors::Negative;

				}//end for py

			}//end for px

		}//end for each triangle

	}//end for each Mesh
	ResetDepthBuffer();
}





void dae::Renderer::ResetDepthBuffer()
{
	for (int i{}; i < (m_Width * m_Height); ++i)
	{
		m_pDepthBufferPixels[i] = std::numeric_limits<float>::max();
	}
}

void dae::Renderer::ResetColorBuffer()
{
	for (int i{}; i < (m_Width * m_Height); ++i)
	{
		m_pBackBufferPixels[i] = SDL_MapRGB(m_pBackBuffer->format,
			static_cast<uint8_t>(0),
			static_cast<uint8_t>(0),
			static_cast<uint8_t>(0));
	}
}

bool dae::Renderer::IsInBoundingBox(const Vector2& pxlScr, size_t indc, const std::vector<Vector2>& vector2_Screen)
{

	Vector2 topLeft{ std::min(std::min(vector2_Screen[indc].x, vector2_Screen[indc + 1].x), vector2_Screen[indc + 2].x),
								 std::min(std::min(vector2_Screen[indc].y, vector2_Screen[indc + 1].y), vector2_Screen[indc + 2].y) };
	Vector2 bottomRight{ std::max(std::max(vector2_Screen[indc].x, vector2_Screen[indc + 1].x), vector2_Screen[indc + 2].x),
						 std::max(std::max(vector2_Screen[indc].y, vector2_Screen[indc + 1].y), vector2_Screen[indc + 2].y) };

	if ((pxlScr.x < topLeft.x || pxlScr.x > bottomRight.x) ||
		(pxlScr.y < topLeft.y || pxlScr.y > bottomRight.y))
		return false;
	else
		return true;

}

bool dae::Renderer::IsInBoundingBox(const Vector2& pxlScr, size_t index, const std::vector<Vector2>& vector2_Screen, const Mesh& mesh)
{
	Vector2 topLeft{ std::min(std::min(vector2_Screen [mesh.indices[index]] .x, vector2_Screen[mesh.indices[index + 1]].x), vector2_Screen[mesh.indices[index + 2]].x),
									 std::min(std::min(vector2_Screen[mesh.indices[index ]].y, vector2_Screen[mesh.indices[index + 1]].y), vector2_Screen[mesh.indices[index + 2]].y) };
	Vector2 bottomRight{ std::max(std::max(vector2_Screen[mesh.indices[index ]].x, vector2_Screen[mesh.indices[index + 1]].x), vector2_Screen[mesh.indices[index + 2]].x),
						 std::max(std::max(vector2_Screen[mesh.indices[index ]].y, vector2_Screen[mesh.indices[index + 1]].y), vector2_Screen[mesh.indices[index + 2]].y) };

	if ((pxlScr.x < topLeft.x || pxlScr.x > bottomRight.x) ||
		(pxlScr.y < topLeft.y || pxlScr.y > bottomRight.y))
		return false;
	else
		return true;

}


void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
	float ar{ float(m_Width) / m_Height };
	float fov{ m_Camera.fov };
	vertices_out.resize(vertices_in.size());

	for (int i{}; i < vertices_out.size(); ++i)
	{
		float perspectiveX = vertices_in[i].position.x / vertices_in[i].position.z;
		float perspectiveY = vertices_in[i].position.y / vertices_in[i].position.z;

		vertices_out[i].position.x = perspectiveX / (ar * fov);
		vertices_out[i].position.y = perspectiveY /  fov;
		vertices_out[i].position.z = vertices_in[i].position.z;
	}

}

void dae::Renderer::ViewProjectionToNDC(const std::vector<Vertex>& world, std::vector<Vector4>& NDC)
{
	const Matrix worldToNDC{ m_Camera.viewMatrix * m_Camera.projectionMatrix };
	for (const Vertex& vert : world)
	{
		Vector4 intermediate = worldToNDC.TransformPoint(vert.position.ToPoint4());
		intermediate.x /= intermediate.w;
		intermediate.y /= intermediate.w;
		intermediate.z /= intermediate.w;
		intermediate.w  = intermediate.w;

		NDC.push_back(Vector4{ intermediate });
	}
}

void dae::Renderer::ViewProjectionToNDC(const Mesh& world, std::vector<Vertex_Out>& NDC)
{
	const Matrix modelToNDC{ world.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };
	auto modelSpace{ world.vertices };
	for (int i{}; i < modelSpace.size(); ++i)
	{
		Vector4 intermediate = modelToNDC.TransformPoint(modelSpace[i].position.ToPoint4());
		intermediate.x /= intermediate.w;
		intermediate.y /= intermediate.w;
		intermediate.z /= intermediate.w;

		Vector3 normal{ (world.worldMatrix.TransformVector(modelSpace[i].normal)).Normalized()};
		Vector3 tangent{ (world.worldMatrix.TransformVector(modelSpace[i].tangent)).Normalized()};
		Vector3 viewDir{ (world.worldMatrix.TransformVector(modelSpace[i].position) - m_Camera.origin).Normalized()};
		Vertex_Out ndc{ intermediate, modelSpace[i].color, modelSpace[i].uv, normal, tangent, viewDir } ;

		NDC.emplace_back(ndc);
	}
}

float dae::Renderer::Remap(float v, float min, float max) const
{
	float result{ (v - min) / (max - min) };
	return Clamp(result, 0.f, 1.f);
}


ColorRGB dae::Renderer::ShadePxl(const Vertex_Out& pxl) const
{
	//Calculate observed area, if negative break
	const Vector3 lightDirection = { .577f, -.577f, .577f };


	float
		lightIntensity{ 7.0f },
		cosArea{};
	Vector3
		normalValue{},
		normalVector{};
	ColorRGB
		Ambient{.025f, .025f, .025f},
		color{  };

	//Calculate normalmaps
	if (m_UseNormalMap)
	{
		Vector3 biNormal        { Vector3::Cross(pxl.normal, pxl.tangent) };
		Matrix tangentSpaceAxis = Matrix{ pxl.tangent, biNormal, pxl.normal,Vector3::Zero };
		normalVector            = m_pTextureNormalMap->SampleNormal(pxl.uv);
		normalValue             = tangentSpaceAxis.TransformVector(normalVector);
							
		//Calculate observerd area, return if negative
		cosArea = lightDirection.Dot(-lightDirection, normalValue) ;
		if (cosArea < 0.0f)
			return {};
	
	}
	else
	{
		normalValue = pxl.normal;
		cosArea     = lightDirection.Dot(-lightDirection, pxl.normal) ;
		if (cosArea < 0.0f)
			return {};
	}

	const float shininess            { 25.f };
	const Vector3 ambient			 { 0.25f, 0.25f, 0.25f };
	const float GlossMapValue        { m_pTextureGlossines->SampleFloat(pxl.uv)  * shininess };
	const ColorRGB specularMapValue  { m_pTextureSpecular->Sample(pxl.uv) };
	const ColorRGB diffuseMap        { m_pTextureVehicle->Sample(pxl.uv) };
	const ColorRGB diffuseColor      { BRDF::Lambert(lightIntensity, diffuseMap) };
		
	
	switch (m_LightMode)
	{
	case LightingMode::ObservedArea:
		color = ColorRGB(cosArea, cosArea, cosArea) ;
		break;

	case LightingMode::Diffuse:
		color = diffuseColor* cosArea;
		break;

	case LightingMode::Specular:
		color =  BRDF::Phong(specularMapValue, GlossMapValue, lightDirection, -pxl.viewDirection, normalValue) * cosArea;
		break;

	case LightingMode::Combined:
		color = (diffuseColor + BRDF::Phong(specularMapValue, GlossMapValue, lightDirection, -pxl.viewDirection, normalValue)) * cosArea;
		break;

	default:
		break;
	}

	return color;
}

void Renderer::VertectTransformToScreen(const std::vector<Vector3>& vertices_in, std::vector<Vector2>& vertices_out) const
{
	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector2 out{};
		out.x = ((vertices_in[i].x + 1) / 2.0f) * static_cast<float> (m_Width);
		out.y = ((1 - vertices_in[i].y) / 2.0f) * static_cast<float> (m_Height);
		vertices_out.emplace_back(out);
	}
}

void Renderer::VertectTransformToScreen(const std::vector<Vertex>& vertices_in, std::vector<Vector2>& vertices_out) const
{
	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector2 out{};
		out.x = ((vertices_in[i].position.x + 1) / 2.0f) * static_cast<float> (m_Width);
		out.y = ((1 - vertices_in[i].position.y) / 2.0f) * static_cast<float> (m_Height);
		vertices_out.emplace_back(out);
	}
}

void Renderer::VertectTransformToScreen(const std::vector<Vector4>& vertices_in, std::vector<Vector2>& vertices_out) const
{
	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector2 out{};
		out.x = ((vertices_in[i].x + 1) / 2.0f) * static_cast<float> (m_Width);
		out.y = ((1 - vertices_in[i].y) / 2.0f) * static_cast<float> (m_Height);
		vertices_out.emplace_back(out);
	}
}

void Renderer::VertectTransformToScreen(const std::vector<Vertex_Out>& vertices_in, std::vector<Vector2>& vertices_out) const
{
	for (int i{}; i < vertices_in.size(); ++i)
	{
		Vector2 out{};
		out.x = ((vertices_in[i].position.x + 1) / 2.0f) * static_cast<float> (m_Width);
		out.y = ((1 - vertices_in[i].position.y) / 2.0f) * static_cast<float> (m_Height);
		vertices_out.emplace_back(out);
	}
}
