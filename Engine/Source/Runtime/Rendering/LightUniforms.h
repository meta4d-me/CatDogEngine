#pragma once

#include <bgfx/bgfx.h>
#include "cassert"
#include "vector"
#include "Lights.h"

namespace engine
{

namespace {
	constexpr uint16_t MAX_PUNCTUAL_LIGHT_COUNT = 16;
	constexpr uint16_t MAX_DIRECTIONAL_LIGHT_COUNT = 16;
	constexpr uint16_t MAX_AREAL_LIGHT_COUNT = 16;
}

constexpr uint16_t MyCeil(const float num) {
	const uint16_t inum = (uint16_t)num;
	if (num == (float)inum) {
		return inum;
	}
	return inum + 1;
}

struct PointLightUniform {
	static constexpr uint16_t POINT_LIGHT_STRIDE = MyCeil(sizeof(PointLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = POINT_LIGHT_STRIDE * MAX_PUNCTUAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_pointLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<PointLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_PUNCTUAL_LIGHT_COUNT && "Point light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
		}m_lights[MAX_PUNCTUAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct SpotLightUniform {
	static constexpr uint16_t SPOT_LIGHT_STRIDE = MyCeil(sizeof(SpotLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = SPOT_LIGHT_STRIDE * MAX_PUNCTUAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_spotLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}
	void Update(const std::vector<SpotLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_PUNCTUAL_LIGHT_COUNT && "Spot light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
			lightUniform.direction = lightSource.direction;
			lightUniform.lightAngleScale = lightSource.lightAngleScale;
			lightUniform.lightAngleOffeset = lightSource.lightAngleOffeset;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
			/*2*/ struct { vec3 direction; float lightAngleScale; };
			/*3*/ struct { float lightAngleOffeset, unused0, unused1, unused2; };
		}m_lights[MAX_PUNCTUAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct DirectionalLightUniform {
	static constexpr uint16_t DIRECTIONAL_LIGHT_STRIDE = MyCeil(sizeof(DirectionalLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = DIRECTIONAL_LIGHT_STRIDE * MAX_DIRECTIONAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_directionalLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<DirectionalLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_DIRECTIONAL_LIGHT_COUNT && "Directional light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.direction = lightSource.direction;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 direction; float unused0; };
		}m_lights[MAX_DIRECTIONAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct SphereLightUniform {
	static constexpr uint16_t SPHERE_LIGHT_STRIDE = MyCeil(sizeof(SphereLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = SPHERE_LIGHT_STRIDE * MAX_AREAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_sphereLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<SphereLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_AREAL_LIGHT_COUNT && "Sphere light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
			lightUniform.radius = lightSource.radius;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
			/*2*/ struct { float radius, unused0, unused1, unused2; };
		}m_lights[MAX_AREAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct DiskLightUniform {
	static constexpr uint16_t DISK_LIGHT_STRIDE = MyCeil(sizeof(DiskLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = DISK_LIGHT_STRIDE * MAX_AREAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_diskLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<DiskLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_AREAL_LIGHT_COUNT && "Disk light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
			lightUniform.direction = lightSource.direction;
			lightUniform.radius = lightSource.radius;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
			/*2*/ struct { vec3 direction; float radius; };
		}m_lights[MAX_AREAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct RectangleLightUniform {
	static constexpr uint16_t RECTANGLE_LIGHT_STRIDE = MyCeil(sizeof(RectangleLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = RECTANGLE_LIGHT_STRIDE * MAX_AREAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_rectangleLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<RectangleLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_AREAL_LIGHT_COUNT && "Rectangle light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
			lightUniform.direction = lightSource.direction;
			lightUniform.width = lightSource.width;
			lightUniform.height = lightSource.height;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
			/*2*/ struct { vec3 direction; float width; };
			/*3*/ struct { vec3 up; float height; };
		}m_lights[MAX_AREAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

struct TubeLightUniform {
	static constexpr uint16_t TUBE_LIGHT_STRIDE = MyCeil(sizeof(TubeLight) / (4.0f * sizeof(float)));
	static constexpr uint16_t VEC4_COUNT = TUBE_LIGHT_STRIDE * MAX_AREAL_LIGHT_COUNT;

	void Init() {
		u_params = bgfx::createUniform("u_tubeLightParams", bgfx::UniformType::Vec4, VEC4_COUNT);
	}

	void Update(const std::vector<TubeLight> &_lights) {
		const size_t realCount = _lights.size();
		assert(realCount <= MAX_AREAL_LIGHT_COUNT && "Tube light count overflow.");
		for (uint16_t lightIndex = 0; lightIndex < realCount; ++lightIndex) {
			auto &lightUniform = m_lights[lightIndex];
			const auto &lightSource = _lights[lightIndex];

			lightUniform.color = lightSource.color;
			lightUniform.intensity = lightSource.intensity;
			lightUniform.position = lightSource.position;
			lightUniform.range = lightSource.range;
			lightUniform.direction = lightSource.direction;
			lightUniform.radius = lightSource.radius;
			lightUniform.width = lightSource.width;
		}
	}

	void Submit() {
		bgfx::setUniform(u_params, m_lightParams, VEC4_COUNT);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			/*0*/ struct { vec3 color; float intensity; };
			/*1*/ struct { vec3 position; float range; };
			/*2*/ struct { vec3 direction; float radius; };
			/*3*/ struct { float width, unused0, unused1, unused2; };
		}m_lights[MAX_AREAL_LIGHT_COUNT];
		float m_lightParams[4 * VEC4_COUNT];
	};

	bgfx::UniformHandle u_params;
};

}
