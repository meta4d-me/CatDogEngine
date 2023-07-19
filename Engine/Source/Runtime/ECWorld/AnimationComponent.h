#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"

#include <vector>

namespace cd
{

class Animation;
class Track;

}

namespace engine
{

class AnimationComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("AnimationComponent");
		return className;
	}

public:
	AnimationComponent() = default;
	AnimationComponent(const AnimationComponent&) = default;
	AnimationComponent& operator=(const AnimationComponent&) = default;
	AnimationComponent(AnimationComponent&&) = default;
	AnimationComponent& operator=(AnimationComponent&&) = default;
	~AnimationComponent() = default;

	const cd::Animation* GetAnimationData() const { return m_pAnimation; }
	void SetAnimationData(const cd::Animation* pAnimation) { m_pAnimation = pAnimation; }
	
	// TODO : use std::span to present pointer array.
	const cd::Track* GetTrackData() const { return m_pTrack; }
	void SetTrackData(const cd::Track* pTrack) { m_pTrack = pTrack; }

	void SetDuration(float duration) { m_duration = duration; }
	float GetDuration() const { return m_duration; }

	void SetTicksPerSecond(float ticksPerSecond) { m_ticksPerSecond = ticksPerSecond; }
	float GetTicksPerSecond() const { return m_ticksPerSecond; }

	void SetBoneMatricesUniform(uint16_t uniform) { m_boneMatricesUniform = uniform; }
	uint16_t GetBoneMatrixsUniform() const { return m_boneMatricesUniform; }

	void SetBoneMatrices(std::vector<cd::Matrix4x4> boneMatrices) { m_boneMatrices = cd::MoveTemp(boneMatrices); }
	std::vector<cd::Matrix4x4>& GetBoneMatrices() { return m_boneMatrices; }
	const std::vector<cd::Matrix4x4>& GetBoneMatrices() const { return m_boneMatrices; }

private:
	const cd::Animation* m_pAnimation = nullptr;
	const cd::Track* m_pTrack = nullptr;
	
	float m_duration;
	float m_ticksPerSecond;
	uint16_t m_boneMatricesUniform;
	std::vector<cd::Matrix4x4> m_boneMatrices;
};

}