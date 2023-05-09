#pragma once

#include "Light.h"

#include <cassert>
#include <vector>

namespace engine
{

class RenderContext;

namespace
{

constexpr uint16_t MAX_LIGHT_COUNT = 16;

}

constexpr uint16_t ConstexprCeil(const float num) {
	const uint16_t inum = (uint16_t)num;
	if (num == (float)inum) {
		return inum;
	}
	return inum + 1;
}

class LightUniform final {
public:
	static constexpr uint16_t LIGHT_STRIDE = ConstexprCeil(sizeof(U_Light) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = LIGHT_STRIDE * MAX_LIGHT_COUNT;
	static_assert(VEC4_COUNT == LIGHT_LENGTH && "Different light parameters length between CPU and GPU.");

public:
	LightUniform() = delete;
	explicit LightUniform(RenderContext *pRenderContext);
	LightUniform(const LightUniform &) = delete;
	LightUniform &operator=(const LightUniform &) = delete;
	LightUniform(LightUniform &&) = delete;
	LightUniform &operator=(LightUniform &&) = delete;

	void Update(const std::vector<U_Light> &lights);
	void Submit(const uint16_t lightNum);
	void Submit();

private:
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

	RenderContext *m_pRenderContext = nullptr;
	uint16_t m_lightCount = 0;
};

} // namespace engine
