#pragma once
#include <cassert>
#include <iostream>
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{			
			return {(kd * cd) / dae::PI};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3

			return {};
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			const Vector3& reflect{ v.Reflect(l, n).Normalized()};
			const float cosAlp{ std::max(0.f, v.Dot(reflect, v))};
			if (cosAlp <= 0.f) return { 0.f, 0.f, 0.f };
			const float value{ ks * powf(cosAlp , exp) };
			return {value, value, value};
		}

		static ColorRGB Phong(const ColorRGB& ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			const Vector3& reflect{ v.Reflect(l, n).Normalized()};
			const float cosAlp{ std::max(0.f, v.Dot(reflect, v))};
			if (cosAlp <= 0.f) return { 0.f, 0.f, 0.f };
			const ColorRGB value{ ks * powf(cosAlp , exp) };
			return value;
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			const ColorRGB& one{ 1.f, 1.f, 1.f };
			const float dotVH{ std::max(v.Dot(v,h), 0.f) };
			if (dotVH == 0.f) return f0;

			return { f0 + (one - f0) * ( (1.f - dotVH) * (1.f - dotVH) * (1.f - dotVH) * (1.f - dotVH) * (1.f - dotVH) ) };
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			const float alphSqr{ roughness * roughness };
			const float dotNH{ h.Dot(n, h) };
			const float dotNHAlpha{ (dotNH * dotNH) * (alphSqr - 1.f) + 1.f };
			return { alphSqr / (float(PI) * (dotNHAlpha * dotNHAlpha ) )};
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static const float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			const float dotNV{ std::max(v.Dot(n, v), 0.f) };
			const float kDirect{ ((roughness + 1.f) * (roughness + 1.f)) / 8.f };
			return {dotNV/ (dotNV * (1.f - kDirect) + kDirect)};
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			float dotNL{ std::max(v.Dot(n, l), 0.f) };
			float kDirect{ std::powf(roughness + 1.f, 2) / 8.f };
			return { (dotNL / (dotNL * (1.f - kDirect) + kDirect)) * GeometryFunction_SchlickGGX(n, v, roughness)};
		}

	}
}