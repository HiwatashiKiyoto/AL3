#define NOMINMAX
#include "Enemy.h"
#include "WorldTransformConfig.h"
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

	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;

	WorldTransformConfig(worldTransform_);
}

void Enemy::Update()
{
	walkTimer_ += 1.0f / 60.0f;

	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	float t = (param + 1.0f) / 2.0f;
	float degrees = MathUtility::Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);
	worldTransform_.rotation_.x = DegreesToRadians(degrees);

	WorldTransformConfig(worldTransform_);
}

void Enemy::Draw(const Camera& camera)
{
	Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	Model::PostDraw();
}
