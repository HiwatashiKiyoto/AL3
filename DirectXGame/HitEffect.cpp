#define NOMINMAX
#include "HitEffect.h"
#include "WorldTransformConfig.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

namespace
{
float EaseOut(float start, float end, float t)
{
	float easedT = 1.0f - (1.0f - t) * (1.0f - t);
	return MathUtility::Lerp(start, end, easedT);
}

float GetEllipseRotation(uint32_t index)
{
	static const std::array<float, 4> kRotations = {
	    0.0f,
	    std::numbers::pi_v<float> / 2.0f,
	    std::numbers::pi_v<float> / 4.0f,
	    -std::numbers::pi_v<float> / 4.0f,
	};
	return kRotations[index];
}
}

HitEffect* HitEffect::Create(const Vector3& position)
{
	HitEffect* instance = new HitEffect();
	assert(instance);
	instance->Initialize(position);
	return instance;
}

void HitEffect::Initialize(const Vector3& position)
{
	position_ = position;
	position_.z += kEffectFrontOffset;
	counter_ = 0.0f;
	state_ = State::kSpread;
	color_ = {1.0f, 0.95f, 0.75f, 1.0f};

	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position_;

	for (WorldTransform& worldTransform : ellipseWorldTransforms_)
	{
		worldTransform.Initialize();
		worldTransform.translation_ = position_;
	}

	for (uint32_t i = 0; i < kNumEllipses; ++i)
	{
		ellipseRotations_[i] = GetEllipseRotation(i);
	}

	objectColor_.Initialize();
	objectColor_.SetColor(color_);
	UpdateTransforms(0.0f);
}

void HitEffect::Update()
{
	if (state_ == State::kDead)
	{
		return;
	}

	counter_ += 1.0f / 60.0f;

	switch (state_)
	{
	case State::kSpread:
	{
		float t = std::clamp(counter_ / kSpreadDuration, 0.0f, 1.0f);
		UpdateTransforms(t);
		if (counter_ >= kSpreadDuration)
		{
			ChangeState(State::kFade);
		}
		break;
	}
	case State::kFade:
	{
		float t = std::clamp(counter_ / kFadeDuration, 0.0f, 1.0f);
		color_.w = std::clamp(1.0f - t, 0.0f, 1.0f);
		objectColor_.SetColor(color_);
		UpdateTransforms(1.0f);
		if (counter_ >= kFadeDuration)
		{
			ChangeState(State::kDead);
		}
		break;
	}
	case State::kDead:
	default:
		break;
	}
}

void HitEffect::Draw()
{
	if (state_ == State::kDead || !model_ || !camera_)
	{
		return;
	}

	Model::PreDraw();
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);
	for (const WorldTransform& worldTransform : ellipseWorldTransforms_)
	{
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	Model::PostDraw();
}

void HitEffect::ChangeState(State state)
{
	state_ = state;
	counter_ = 0.0f;
}

void HitEffect::UpdateTransforms(float spreadT)
{
	float circleScale = EaseOut(kCircleStartScale, kCircleEndScale, spreadT);
	circleWorldTransform_.translation_ = position_;
	circleWorldTransform_.scale_ = {circleScale, circleScale, 1.0f};
	WorldTransformConfig(circleWorldTransform_);

	float ellipseScale = EaseOut(kEllipseStartScale, kEllipseEndScale, spreadT);
	for (uint32_t i = 0; i < kNumEllipses; ++i)
	{
		ellipseWorldTransforms_[i].translation_ = position_;
		ellipseWorldTransforms_[i].scale_ = {kEllipseWidth * ellipseScale, kEllipseLength * ellipseScale, 1.0f};
		ellipseWorldTransforms_[i].rotation_ = {0.0f, 0.0f, ellipseRotations_[i]};
		WorldTransformConfig(ellipseWorldTransforms_[i]);
	}
}
