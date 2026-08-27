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
constexpr float kDirectionEpsilon = 0.0001f;

Vector3 SlerpDirection(const Vector3& from, const Vector3& to, float t)
{
	const float dot = std::clamp(Dot(from, to), -1.0f, 1.0f);

	if (dot > 0.9995f)
	{
		Vector3 result = from + (to - from) * t;
		Normalize(result);
		return result;
	}

	if (dot < -0.9995f)
	{
		const Vector3 reference = std::abs(from.y) < 0.9f ? Vector3{0.0f, 1.0f, 0.0f} : Vector3{1.0f, 0.0f, 0.0f};
		Vector3 perpendicular = Cross(from, reference);
		Normalize(perpendicular);
		const float angle = 3.141592654f * t;
		return from * std::cos(angle) + perpendicular * std::sin(angle);
	}

	const float angle = std::acos(dot);
	const float sinAngle = std::sin(angle);
	const float fromWeight = std::sin((1.0f - t) * angle) / sinAngle;
	const float toWeight = std::sin(t * angle) / sinAngle;
	return from * fromWeight + to * toWeight;
}
}

void EnemyBullet::Initialize(Model* model, uint32_t textureHandle, const Vector3& position, const Vector3& velocity)
{
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;
	velocity_ = velocity;

	worldTransform_.Initialize();
	// Enemy shots now use the same fish-bone mesh as the player. Keep its
	// proportions intact and distinguish it with the shared red texture.
	worldTransform_.scale_ = {0.62f, 0.62f, 0.62f};
	worldTransform_.rotation_.z = -20.0f * 3.141592654f / 180.0f;
	worldTransform_.translation_ = position;
	UpdateRotation();
	UpdateWorldTransform(worldTransform_);
}

void EnemyBullet::Update()
{
	const float bulletSpeed = Length(velocity_);
	if (player_ != nullptr && bulletSpeed > kDirectionEpsilon)
	{
		Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();
		if (Length(toPlayer) > kDirectionEpsilon)
		{
			Normalize(toPlayer);
			Vector3 currentDirection = velocity_;
			Normalize(currentDirection);
			velocity_ = SlerpDirection(currentDirection, toPlayer, kHomingStrength) * bulletSpeed;
		}
	}

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

void EnemyBullet::OnCollision()
{
	isDead_ = true;
}

void EnemyBullet::UpdateRotation()
{
	const float horizontalLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	worldTransform_.rotation_.x = -std::atan2(velocity_.y, horizontalLength);
	worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);
}
