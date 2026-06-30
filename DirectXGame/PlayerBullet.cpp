#include "PlayerBullet.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void PlayerBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("black.png");
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.8f, 0.8f, 0.8f};
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
}

void PlayerBullet::Update()
{
	worldTransform_.translation_ += velocity_;

	if (--deathTimer_ <= 0)
	{
		isDead_ = true;
	}

	UpdateWorldTransform(worldTransform_);
}

void PlayerBullet::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}
