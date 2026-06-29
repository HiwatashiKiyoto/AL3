#include "GuardEffect.h"

#include "WorldTransformConfig.h"
#include <algorithm>
#include <cassert>

using namespace KamataEngine;

Model* GuardEffect::model_ = nullptr;
Camera* GuardEffect::camera_ = nullptr;

GuardEffect* GuardEffect::Create(const Vector3& position)
{
	GuardEffect* instance = new GuardEffect();
	assert(instance);
	instance->Initialize(position);
	return instance;
}

void GuardEffect::Initialize(const Vector3& position)
{
	position_ = position;
	position_.z += kEffectFrontOffset;

	worldTransform_.Initialize();
	objectColor_.Initialize();
	color_ = {0.75f, 0.95f, 1.0f, 1.0f};
	objectColor_.SetColor(color_);
	ChangeState(State::kSpread);
	UpdateTransform(0.0f);
}

void GuardEffect::Update()
{
	if (state_ == State::kDead) {
		return;
	}

	counter_ += 1.0f / 60.0f;

	switch (state_) {
	case State::kSpread:
	{
		const float t = std::clamp(counter_ / kSpreadDuration, 0.0f, 1.0f);
		UpdateTransform(t);
		if (counter_ >= kSpreadDuration) {
			ChangeState(State::kFade);
		}
		break;
	}
	case State::kFade:
	{
		const float t = std::clamp(counter_ / kFadeDuration, 0.0f, 1.0f);
		UpdateTransform(1.0f);
		color_.w = 1.0f - t;
		objectColor_.SetColor(color_);
		if (counter_ >= kFadeDuration) {
			ChangeState(State::kDead);
		}
		break;
	}
	case State::kDead:
		break;
	}
}

void GuardEffect::Draw()
{
	if (state_ == State::kDead || model_ == nullptr || camera_ == nullptr) {
		return;
	}

	Model::PreDraw();
	model_->Draw(worldTransform_, *camera_, &objectColor_);
	Model::PostDraw();
}

void GuardEffect::ChangeState(State state)
{
	state_ = state;
	counter_ = 0.0f;
}

void GuardEffect::UpdateTransform(float scaleT)
{
	const float eased = 1.0f - (1.0f - scaleT) * (1.0f - scaleT);
	const float scale = kStartScale + (kEndScale - kStartScale) * eased;

	worldTransform_.translation_ = position_;
	worldTransform_.scale_ = {scale, scale, scale};
	WorldTransformConfig(worldTransform_);
}
