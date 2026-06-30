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
	phase_ = Phase::Approach;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
}

void Enemy::Update()
{
	switch (phase_)
	{
	case Phase::Approach:
	default:
		UpdateApproach();
		break;
	case Phase::Leave:
		UpdateLeave();
		break;
	}

	UpdateWorldTransform(worldTransform_);

}

void Enemy::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);
}

void Enemy::UpdateApproach()
{
	const Vector3 velocity = {0.0f, 0.0f, -0.1f};
	worldTransform_.translation_ += velocity;

	if (worldTransform_.translation_.z < 0.0f)
	{
		phase_ = Phase::Leave;
	}
}

void Enemy::UpdateLeave()
{
	const Vector3 velocity = {0.0f, 0.0f, 0.1f};
	worldTransform_.translation_ += velocity;
}

const char* Enemy::GetPhaseName() const
{
	switch (phase_)
	{
	case Phase::Approach:
		return "Approach";
	case Phase::Leave:
		return "Leave";
	}

	return "Unknown";
}
