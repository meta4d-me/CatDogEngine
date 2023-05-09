#include "LightUniforms.h"

#include "RenderContext.h"

namespace engine
{

LightUniform::LightUniform(RenderContext *pRenderContext)
	: m_pRenderContext(pRenderContext) {
	m_pRenderContext->CreateUniform("u_lightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
};

void LightUniform::Update(const std::vector<U_Light> &lights) {
	m_lightCount = static_cast<uint16_t>(lights.size());
	assert(m_lightCount <= MAX_LIGHT_COUNT && "Light count overflow.");

	for (size_t index = 0; index < m_lightCount; ++index) {
		// I still use this unnamed struct to send data to GPU,
		// because bgfx's uniform must align to vec4/mat3/mat4.
		// If we add some new members to U_Light in the future,
		// which will make U_Light no longer aligned with vec4,
		// the static_assert below may be fail.
		auto &target = m_light[index];
		const U_Light &source = lights[index];

		// The data structure of these two types are perfectly aligned for now.
		static_assert(sizeof(target) == sizeof(source));
		std::memcpy(&target, &source, sizeof(target));
	}
}

void LightUniform::Submit(const uint16_t lightNum) {
	assert(lightNum <= MAX_LIGHT_COUNT && "Light count overflow.");
	m_pRenderContext->FillUniform(StringCrc("u_lightParams"), m_lightParams, lightNum * LIGHT_STRIDE);
}

void LightUniform::Submit() {
	Submit(m_lightCount);
}

} // namespace engine
