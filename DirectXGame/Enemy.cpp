#include "Enemy.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

Enemy::~Enemy()
{
	for (EnemyBullet* bullet : bullets_)
	{
		delete bullet;
	}
}

void Enemy::Initialize(Model* model, const Vector3& position)
{
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("white1x1.png");
	phase_ = Phase::Approach;
	ApproachPhaseInitialize();

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	UpdateWorldTransform(worldTransform_);
}

void Enemy::Update()
{
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead())
		{
			delete bullet;
			return true;
		}

		return false;
	});

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

	for (EnemyBullet* bullet : bullets_)
	{
		bullet->Update();
	}
}

void Enemy::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera, textureHandle_);

	for (EnemyBullet* bullet : bullets_)
	{
		bullet->Draw(camera);
	}
}

void Enemy::Fire()
{
	const float kBulletSpeed = 1.0f;
	const Vector3 velocity = {0.0f, 0.0f, -kBulletSpeed};

	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransform_.translation_, velocity);

	bullets_.push_back(newBullet);
}

void Enemy::ApproachPhaseInitialize()
{
	fireTimer_ = kFireInterval;
}

void Enemy::UpdateApproach()
{
	if (--fireTimer_ <= 0)
	{
		Fire();
		fireTimer_ = kFireInterval;
	}

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
