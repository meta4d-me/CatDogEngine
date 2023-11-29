#pragma once

#include "Rendering/Light.h"

#include <cassert>
#include <vector>

namespace engine
{

class RenderContext;

namespace
{

constexpr uint16_t MAX_LIGHT_COUNT = 8;

constexpr uint16_t ConstexprCeil(float x)
{
	// In C++ 23, we can simply use std::ceil as a constexpr function.
	return x == static_cast<float>(static_cast<uint16_t>(x))
		? static_cast<uint16_t>(x)
		: static_cast<uint16_t>(x) + 1;
}

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
	struct LightParameters
	{
		float type; vec3 position;
		float intensity; vec3 color;
		float range; vec3 direction;
		float radius; vec3 up;
		float width, height, lightAngleScale, lightAngleOffeset;
	};

	LightParameters m_lightParameters[MAX_LIGHT_COUNT];
	RenderContext *m_pRenderContext = nullptr;
	uint16_t m_lightCount = 0;
};

} // namespace engine
