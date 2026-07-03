#include "EnemyBullet.h"

#include "Player.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
// Linear interpolation
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t)
{
	return v1 + (v2 - v1) * t;
}

// Spherical linear interpolation
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t)
{
	Vector3 start = v1;
	Vector3 end = v2;
	Normalize(start);
	Normalize(end);

	const float dot = std::clamp(Dot(start, end), -1.0f, 1.0f);
	if (dot > 0.999f || dot < -0.999f)
	{
		Vector3 result = Lerp(start, end, t);
		Normalize(result);
		return result;
	}

	const float theta = std::acos(dot);
	const float sinTheta = std::sin(theta);
	const float scaleStart = std::sin((1.0f - t) * theta) / sinTheta;
	const float scaleEnd = std::sin(t * theta) / sinTheta;

	return start * scaleStart + end * scaleEnd;
}
}

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("red1x1.png");
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.scale_.x = 0.5f;
	worldTransform_.scale_.y = 0.5f;
	worldTransform_.scale_.z = 3.0f;
	worldTransform_.translation_ = position;

	UpdateRotation();

	UpdateWorldTransform(worldTransform_);
}

void EnemyBullet::Update()
{
	assert(player_);

	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();
	Normalize(toPlayer);

	const float bulletSpeed = Length(velocity_);
	Normalize(velocity_);
	velocity_ = Slerp(velocity_, toPlayer, kHomingStrength) * bulletSpeed;

	UpdateRotation();

	worldTransform_.translation_ += velocity_;

	if (--deathTimer_ <= 0)
	{
		isDead_ = true;
	}

	UpdateWorldTransform(worldTransform_);
}

void EnemyBullet::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}

Vector3 EnemyBullet::GetWorldPosition() const
{
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void EnemyBullet::UpdateRotation()
{
	// Y-axis angle
	worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);

	// Length in horizontal direction
	Vector3 velocityXZ = velocity_;
	velocityXZ.y = 0.0f;
	const float velocityXZLength = Length(velocityXZ);

	// X-axis angle
	worldTransform_.rotation_.x = std::atan2(-velocity_.y, velocityXZLength);
}
