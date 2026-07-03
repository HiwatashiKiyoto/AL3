#include "EnemyBullet.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("red1x1.png");
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.8f, 0.8f, 0.8f};
	worldTransform_.translation_ = position;
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
