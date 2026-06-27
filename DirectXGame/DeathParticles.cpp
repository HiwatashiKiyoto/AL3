#define NOMINMAX
#include "DeathParticles.h"
#include "WorldTransformConfig.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	model_ = model;
	camera_ = camera;
	counter_ = 0.0f;
	isFinished_ = false;
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	for (WorldTransform& worldTransform : worldTransforms_)
	{
		worldTransform.Initialize();
		worldTransform.translation_ = position;
		worldTransform.scale_ = {kParticleScale, kParticleScale, kParticleScale};
		WorldTransformConfig(worldTransform);
	}

	objectColor_.Initialize();
	objectColor_.SetColor(color_);
}

void DeathParticles::Update()
{
	if (isFinished_)
	{
		return;
	}

	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration)
	{
		counter_ = kDuration;
		isFinished_ = true;
	}

	color_.w = std::clamp(1.0f - counter_ / kDuration, 0.0f, 1.0f);
	objectColor_.SetColor(color_);

	for (uint32_t i = 0; i < kNumParticles; ++i)
	{
		float angle = kAngleUnit * static_cast<float>(i);
		Vector3 velocity = {};
		velocity.x = std::cos(angle) * kSpeed;
		velocity.y = std::sin(angle) * kSpeed;
		velocity.z = 0.0f;

		worldTransforms_[i].translation_.x += velocity.x;
		worldTransforms_[i].translation_.y += velocity.y;
		worldTransforms_[i].translation_.z += velocity.z;
		WorldTransformConfig(worldTransforms_[i]);
	}
}

void DeathParticles::Draw()
{
	if (isFinished_)
	{
		return;
	}

	Model::PreDraw();
	for (const WorldTransform& worldTransform : worldTransforms_)
	{
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	Model::PostDraw();
}
