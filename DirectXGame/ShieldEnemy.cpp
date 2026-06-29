#include "ShieldEnemy.h"

#include "GameScene.h"
#include "Player.h"
#include "WorldTransformConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

void ShieldEnemy::Initialize(Model* model, Camera* camera, const Vector3& position)
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
	isFacingRight_ = false;
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
	guardTimer_ = 0.0f;
	deathTimer_ = 0.0f;

	WorldTransformConfig(worldTransform_);
}

void ShieldEnemy::Update()
{
	if (isDead_) {
		return;
	}

	UpdateBehaviorTransition();

	switch (behavior_) {
	case Behavior::kWalk:
		BehaviorWalkUpdate();
		break;
	case Behavior::kGuard:
		BehaviorGuardUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	case Behavior::kUnknown:
	default:
		break;
	}

	WorldTransformConfig(worldTransform_);
}

void ShieldEnemy::Draw(const Camera& camera)
{
	if (isDead_ || model_ == nullptr) {
		return;
	}

	Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	Model::PostDraw();
}

Vector3 ShieldEnemy::GetWorldPosition() const
{
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB ShieldEnemy::GetAABB() const
{
	const Vector3 worldPos = GetWorldPosition();

	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void ShieldEnemy::OnCollision(Player* player)
{
	if (behavior_ == Behavior::kDeath || behavior_ == Behavior::kGuard || isDead_) {
		return;
	}

	if (player && player->IsAttack()) {
		const Vector3 enemyPosition = GetWorldPosition();
		const Vector3 effectPos = {
		    enemyPosition.x,
		    enemyPosition.y + kHeight / 2.0f,
		    enemyPosition.z,
		};
		const Vector3 guardEffectPos = {
		    enemyPosition.x,
		    enemyPosition.y + kHeight * 0.85f,
		    enemyPosition.z,
		};

		if (IsGuardingFrontAttack(player)) {
			behaviorRequest_ = Behavior::kGuard;
			player->RequestKnockback();
			if (gameScene_) {
				gameScene_->CreateGuardEffect(guardEffectPos);
			}
			return;
		}

		behaviorRequest_ = Behavior::kDeath;
		if (gameScene_) {
			gameScene_->CreateHitEffect(effectPos);
		}
	}
}

void ShieldEnemy::UpdateBehaviorTransition()
{
	if (behaviorRequest_ == Behavior::kUnknown) {
		return;
	}

	behavior_ = behaviorRequest_;
	behaviorRequest_ = Behavior::kUnknown;

	switch (behavior_) {
	case Behavior::kWalk:
		BehaviorWalkInitialize();
		break;
	case Behavior::kGuard:
		BehaviorGuardInitialize();
		break;
	case Behavior::kDeath:
		BehaviorDeathInitialize();
		break;
	case Behavior::kUnknown:
	default:
		break;
	}
}

void ShieldEnemy::BehaviorWalkInitialize()
{
	isCollisionDisabled_ = false;
	velocity_ = isFacingRight_ ? Vector3{kWalkSpeed, 0.0f, 0.0f} : Vector3{-kWalkSpeed, 0.0f, 0.0f};
}

void ShieldEnemy::BehaviorWalkUpdate()
{
	walkTimer_ += 1.0f / 60.0f;
	if (walkTimer_ >= 1.0f) {
		walkTimer_ -= 1.0f;
	}

	const float angleT = (std::sin(walkTimer_ * std::numbers::pi_v<float> * 2.0f) + 1.0f) / 2.0f;
	const float angleDegree = -20.0f + (20.0f - -20.0f) * angleT;
	worldTransform_.rotation_.x = angleDegree * std::numbers::pi_v<float> / 180.0f;

	worldTransform_.translation_.x += velocity_.x;
}

void ShieldEnemy::BehaviorGuardInitialize()
{
	isCollisionDisabled_ = true;
	velocity_ = {};
	guardTimer_ = 0.0f;
}

void ShieldEnemy::BehaviorGuardUpdate()
{
	guardTimer_ += 1.0f / 60.0f;

	const float t = std::clamp(guardTimer_ / kGuardMotionTime, 0.0f, 1.0f);
	worldTransform_.rotation_.z = std::sin(t * std::numbers::pi_v<float> * 2.0f) * 0.2f;

	if (guardTimer_ >= kGuardMotionTime) {
		worldTransform_.rotation_.z = 0.0f;
		behaviorRequest_ = Behavior::kWalk;
	}
}

void ShieldEnemy::BehaviorDeathInitialize()
{
	isCollisionDisabled_ = true;
	deathTimer_ = 0.0f;
	velocity_ = {};
}

void ShieldEnemy::BehaviorDeathUpdate()
{
	deathTimer_ += 1.0f / 60.0f;
	worldTransform_.translation_.y += kDeathFloatSpeed;
	worldTransform_.rotation_.y += kDeathSpinSpeedY * 1.0f / 60.0f;
	worldTransform_.rotation_.x += kDeathSpinSpeedX * 1.0f / 60.0f;

	const float t = std::clamp(deathTimer_ / kDeathMotionTime, 0.0f, 1.0f);
	const float scale = kModelScale * (1.0f - t);
	worldTransform_.scale_ = {scale, scale, scale};

	if (deathTimer_ >= kDeathMotionTime) {
		isDead_ = true;
	}
}

bool ShieldEnemy::IsGuardingFrontAttack(const Player* player) const
{
	if (player == nullptr) {
		return false;
	}

	const bool playerIsLeft = player->GetWorldPosition().x < GetWorldPosition().x;

	if (playerIsLeft) {
		return player->IsFacingRight() && !isFacingRight_;
	}

	return !player->IsFacingRight() && isFacingRight_;
}
