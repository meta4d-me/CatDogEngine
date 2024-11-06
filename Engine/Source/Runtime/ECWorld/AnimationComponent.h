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

enum class AnimationClip
{
	Idle,
	Walking,
	Running,

	Blend,
	Switch,
	Count,
};

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
	float& GetDuration() { return m_duration; }
	float GetDuration() const { return m_duration; }

	void SetTicksPerSecond(float ticksPerSecond) { m_ticksPerSecond = ticksPerSecond; }
	float& GetTicksPerSecond() { return m_ticksPerSecond; }
	float GetTicksPerSecond() const { return m_ticksPerSecond; }

	void SetBoneMatricesUniform(uint16_t uniform) { m_boneMatricesUniform = uniform; }
	uint16_t& GetBoneMatrixsUniform() { return m_boneMatricesUniform; }
	uint16_t GetBoneMatrixsUniform() const { return m_boneMatricesUniform; }

	void SetAnimationPlayTime(float time) { m_animationPlayTime = time; }
	float& GetAnimationPlayTime() { return m_animationPlayTime; }
	float GetAnimationPlayTime() const { return m_animationPlayTime; }

	void SetPlayBackSpeed(float time) { m_playBackSpeed = time; }
	float& GetPlayBackSpeed() { return m_playBackSpeed; }
	float GetPlayBackSpeed() const { return m_playBackSpeed; }

	void SetAnimationClip(AnimationClip crtClip) { m_clip = crtClip; }
	AnimationClip& GetAnimationClip() { return m_clip; }
	const AnimationClip& GeAnimationClip() const { return m_clip; }

	void SetBlendFactor(float factor) { m_blendFactor = factor; }
	float& GetBlendFactor() { return m_blendFactor; }
	float GetBlendFactor() const { return m_blendFactor; }

	bool& IsPlaying() { return m_playAnimation; }

private:
	AnimationClip m_clip = AnimationClip::Idle;
	const cd::Animation* m_pAnimation = nullptr;
	const cd::Track* m_pTrack = nullptr;

	float m_blendFactor;
	float m_playBackSpeed;
	bool m_playAnimation;

	float m_animationPlayTime;
	float m_duration;
	float m_ticksPerSecond;
	uint16_t m_boneMatricesUniform;
	std::vector<cd::Matrix4x4> m_boneMatrices;
};

}