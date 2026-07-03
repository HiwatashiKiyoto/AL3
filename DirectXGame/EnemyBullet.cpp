#include "EnemyBullet.h"

#include "WorldTransformUpdate.h"

#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

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

	// Y-axis angle
	worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);

	// Length in horizontal direction
	Vector3 velocityXZ = velocity_;
	velocityXZ.y = 0.0f;
	const float velocityXZLength = Length(velocityXZ);

	// X-axis angle
	worldTransform_.rotation_.x = std::atan2(-velocity_.y, velocityXZLength);

	UpdateWorldTransform(worldTransform_);
}

void EnemyBullet::Update()
{
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
