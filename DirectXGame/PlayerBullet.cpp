#include "PlayerBullet.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;

void PlayerBullet::Initialize(Model* model, const Vector3& position)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("black.png");

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.8f, 0.8f, 0.8f};
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
}

void PlayerBullet::Update()
{
	UpdateWorldTransform(worldTransform_);
}

void PlayerBullet::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}
