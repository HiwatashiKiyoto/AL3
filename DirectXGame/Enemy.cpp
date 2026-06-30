#include "Enemy.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void Enemy::Initialize(Model* model, const Vector3& position)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("white1x1.png");

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
}

void Enemy::Update()
{
	const Vector3 velocity = {0.0f, 0.0f, -0.1f};
	worldTransform_.translation_ += velocity;

	UpdateWorldTransform(worldTransform_);

#ifdef USE_IMGUI
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Position", &worldTransform_.translation_.x, 0.01f);
	ImGui::End();
#endif
}

void Enemy::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}
