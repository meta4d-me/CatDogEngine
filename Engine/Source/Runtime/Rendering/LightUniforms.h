#pragma once

#include "Math/VectorDerived.hpp"

// TODO : Remove 3rd party dependencies for bgfx
#include <bgfx/bgfx.h>

#include <cassert>
#include <vector>

namespace engine
{

namespace
{
	constexpr uint16_t MAX_LIGHT_COUNT = 16;
	using vec3 = cd::Vec3f;
}

#include "Shaders/UniformDefines/U_Light.sh"

constexpr uint16_t ConstexprCeil(const float num) {
	const uint16_t inum = (uint16_t)num;
	if (num == (float)inum) {
		return inum;
	}
	return inum + 1;
}

struct LightUniform {
	static constexpr uint16_t LIGHT_STRIDE = ConstexprCeil(sizeof(U_Light) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = LIGHT_STRIDE * MAX_LIGHT_COUNT;

	void Init() {
		static_assert(VEC4_COUNT == LIGHT_LENGTH && "Different light parameters length between CPU and GPU.");
		u_params = bgfx::createUniform("u_lightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Submit(const uint16_t _lightNum) {
		assert(_lightNum <= MAX_LIGHT_COUNT && "Light count overflow;");
		bgfx::setUniform(u_params, m_lightParams, _lightNum * LIGHT_STRIDE);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { float type; vec3 position; };
			/*1*/ struct { float intensity; vec3 color; };
			/*2*/ struct { float range; vec3 direction; };
			/*3*/ struct { float radius; vec3 up; };
			/*4*/ struct { float width, height, lightAngleScale, lightAngleOffeset; };
		}m_light[MAX_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

} // namespace engine
