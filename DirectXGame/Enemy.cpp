#define NOMINMAX
#include "Enemy.h"
#include "GameScene.h"
#include "Player.h"
#include "WorldTransformConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

namespace
{
float DegreesToRadians(float degrees)
{
	return degrees * std::numbers::pi_v<float> / 180.0f;
}
}

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransform_.scale_ = {kModelScale, kModelScale, kModelScale};

	behavior_ = Behavior::kWalk;
	behaviorRequest_ = Behavior::kUnknown;
	isDead_ = false;
	isCollisionDisabled_ = false;
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
	deathTimer_ = 0.0f;

	WorldTransformConfig(worldTransform_);
}

void Enemy::Update()
{
	UpdateBehaviorTransition();

	switch (behavior_)
	{
	case Behavior::kWalk:
	default:
		BehaviorWalkUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	}

	WorldTransformConfig(worldTransform_);
}

void Enemy::UpdateBehaviorTransition()
{
	if (behaviorRequest_ == Behavior::kUnknown)
	{
		return;
	}

	behavior_ = behaviorRequest_;

	switch (behavior_)
	{
	case Behavior::kWalk:
	default:
		BehaviorWalkInitialize();
		break;
	case Behavior::kDeath:
		BehaviorDeathInitialize();
		break;
	}

	behaviorRequest_ = Behavior::kUnknown;
}

void Enemy::BehaviorWalkInitialize()
{
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
	worldTransform_.scale_ = {kModelScale, kModelScale, kModelScale};
}

void Enemy::BehaviorWalkUpdate()
{
	walkTimer_ += 1.0f / 60.0f;

	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	float t = (param + 1.0f) / 2.0f;
	float degrees = MathUtility::Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);
	worldTransform_.rotation_.x = DegreesToRadians(degrees);
}

void Enemy::BehaviorDeathInitialize()
{
	isCollisionDisabled_ = true;
	deathTimer_ = 0.0f;
	velocity_ = {};
}

void Enemy::BehaviorDeathUpdate()
{
	deathTimer_ += 1.0f / 60.0f;

	worldTransform_.translation_.y += kDeathFloatSpeed;
	worldTransform_.rotation_.y += kDeathSpinSpeedY * (1.0f / 60.0f);
	worldTransform_.rotation_.x += kDeathSpinSpeedX * (1.0f / 60.0f);

	float t = std::clamp(deathTimer_ / kDeathMotionTime, 0.0f, 1.0f);
	float scale = MathUtility::Lerp(kModelScale, 0.1f, t);
	worldTransform_.scale_ = {scale, scale, scale};

	if (deathTimer_ >= kDeathMotionTime)
	{
		isDead_ = true;
	}
}

void Enemy::Draw(const Camera& camera)
{
	Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	Model::PostDraw();
}

Vector3 Enemy::GetWorldPosition() const
{
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Enemy::GetAABB() const
{
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight, worldPos.z + kWidth / 2.0f};
	return aabb;
}

void Enemy::OnCollision(const Player* player)
{
	if (behavior_ == Behavior::kDeath)
	{
		return;
	}

	if (player && player->IsAttack())
	{
		behaviorRequest_ = Behavior::kDeath;
		if (gameScene_)
		{
			Vector3 enemyPosition = GetWorldPosition();
			Vector3 effectPosition = {
			    enemyPosition.x,
			    enemyPosition.y + kHeight / 2.0f,
			    enemyPosition.z};
			gameScene_->CreateHitEffect(effectPosition);
		}
	}
}
