#include "PlayerBullet.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
constexpr float kBaseVisualScale = 0.70f;
constexpr float kSpawnVisualScale = 1.05f;
constexpr int32_t kSpawnPopFrames = 12;
constexpr int32_t kTrailSampleInterval = 2;
constexpr float kReadabilityRoll = 20.0f * 3.141592654f / 180.0f;
}

void PlayerBullet::Initialize(Model* model, uint32_t textureHandle, const Vector3& position, const Vector3& velocity)
{
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {kSpawnVisualScale, kSpawnVisualScale, kSpawnVisualScale};
	const float horizontalLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	worldTransform_.rotation_.x = -std::atan2(velocity_.y, horizontalLength);
	worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);
	worldTransform_.rotation_.z = kReadabilityRoll;
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
	outlineTransform_.Initialize();
	outlineTransform_.rotation_ = worldTransform_.rotation_;
	outlineTransform_.translation_ = position;
	outlineTransform_.scale_ = {kSpawnVisualScale * 1.12f, kSpawnVisualScale * 1.12f, kSpawnVisualScale * 1.12f};
	UpdateWorldTransform(outlineTransform_);

	bodyColor_.Initialize();
	bodyColor_.SetColor({1.0f, 0.34f, 0.04f, 1.0f});
	outlineColor_.Initialize();
	outlineColor_.SetColor({0.015f, 0.025f, 0.055f, 1.0f});
	trailColor_.Initialize();
	trailColor_.SetColor({1.0f, 0.20f, 0.015f, 0.38f});

	for (size_t i = 0; i < trailTransforms_.size(); ++i)
	{
		WorldTransform& trail = trailTransforms_[i];
		trail.Initialize();
		const float ratio = 1.0f - static_cast<float>(i) / static_cast<float>(trailTransforms_.size());
		const float trailScale = 0.18f * ratio;
		trail.scale_ = {trailScale, trailScale, trailScale};
		trail.rotation_ = worldTransform_.rotation_;
		trail.translation_ = position;
		UpdateWorldTransform(trail);
	}
}

void PlayerBullet::Update()
{
	++ageFrames_;
	worldTransform_.translation_ += velocity_;

	// Start at a very readable size, then settle to the 2x display scale in 0.2 seconds.
	const float popT = std::clamp(static_cast<float>(ageFrames_) / static_cast<float>(kSpawnPopFrames), 0.0f, 1.0f);
	const float smoothT = popT * popT * (3.0f - 2.0f * popT);
	const float visualScale = kSpawnVisualScale + (kBaseVisualScale - kSpawnVisualScale) * smoothT;
	worldTransform_.scale_ = {visualScale, visualScale, visualScale};
	outlineTransform_.rotation_ = worldTransform_.rotation_;
	outlineTransform_.translation_ = worldTransform_.translation_;
	outlineTransform_.scale_ = {visualScale * 1.12f, visualScale * 1.12f, visualScale * 1.12f};
	UpdateWorldTransform(outlineTransform_);

	// Sample every two frames: five afterimages persist for about 0.15 seconds at 60 fps.
	if (ageFrames_ % kTrailSampleInterval == 0)
	{
		for (size_t i = trailTransforms_.size() - 1; i > 0; --i)
		{
			trailTransforms_[i].translation_ = trailTransforms_[i - 1].translation_;
		}
		trailTransforms_[0].translation_ = worldTransform_.translation_ - velocity_;
	}

	for (WorldTransform& trail : trailTransforms_)
	{
		trail.rotation_ = worldTransform_.rotation_;
		UpdateWorldTransform(trail);
	}

	if (--deathTimer_ <= 0)
	{
		isDead_ = true;
	}

	UpdateWorldTransform(worldTransform_);
}

void PlayerBullet::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_, &bodyColor_);
}

void PlayerBullet::DrawTrail(const Camera& camera)
{
	for (auto it = trailTransforms_.rbegin(); it != trailTransforms_.rend(); ++it)
	{
		model_->Draw(*it, camera, textureHandle_, &trailColor_);
	}
}

void PlayerBullet::DrawOutline(const Camera& camera)
{
	model_->Draw(outlineTransform_, camera, textureHandle_, &outlineColor_);
}

Vector3 PlayerBullet::GetWorldPosition() const
{
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void PlayerBullet::OnCollision()
{
	isDead_ = true;
}
