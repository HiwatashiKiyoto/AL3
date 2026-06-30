#include "Player.h"

#include <cassert>

using namespace KamataEngine;

void Player::Initialize(Model* model, uint32_t textureHandle)
{
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();
}

void Player::Update()
{
	worldTransform_.TransferMatrix();
}

void Player::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}
