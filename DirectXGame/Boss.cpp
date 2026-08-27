#include "Boss.h"

#include "GameAudio.h"
#include "EnemyBullet.h"
#include "GameScene.h"
#include "Player.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
Vector4 MakeGamingColor(int32_t frame)
{
	const float phase = static_cast<float>(frame) * 0.045f;
	const float pulse = 0.84f + std::sin(phase * 2.3f) * 0.16f;
	const auto channel = [phase, pulse](float offset) {
		return (0.18f + (std::sin(phase + offset) * 0.5f + 0.5f) * 0.82f) * pulse;
	};
	return {
	    channel(0.0f),
	    channel(2.094395102f),
	    channel(4.188790205f),
	    1.0f};
}
}

void Boss::Initialize(
    Model* model, Model* bulletModel, Player* player, GameScene* gameScene,
    uint32_t bulletTextureHandle, const Vector3& position)
{
	assert(model);
	assert(bulletModel);
	assert(player);
	assert(gameScene);
	model_ = model;
	bulletModel_ = bulletModel;
	bulletTextureHandle_ = bulletTextureHandle;
	player_ = player;
	gameScene_ = gameScene;
	basePosition_ = position;
	health_ = kMaxHealth;
	battleTimer_ = 0;
	fireTimer_ = kFireInterval;
	colorTimer_ = 0;
	presentationScale_ = 0.18f;
	gamingColor_.Initialize();
	UpdateGamingMaterial();

	worldTransform_.Initialize();
	worldTransform_.translation_ = basePosition_;
	worldTransform_.scale_ = {presentationScale_, presentationScale_, presentationScale_};

	modelTransform_.Initialize();
	modelTransform_.scale_ = {1.05f, 1.05f, 1.05f};
	modelTransform_.rotation_.y = 3.141592654f;
	// Recenter the authored OBJ around its measured geometry center.
	constexpr Vector3 kModelCenter = {1.38256f, 4.1077125f, -0.637657f};
	const Matrix4x4 orientation =
	    MakeScaleMatrix(modelTransform_.scale_) * MakeRotateYMatrix(modelTransform_.rotation_.y);
	const Vector3 transformedCenter = TransformNormal(kModelCenter, orientation);
	modelTransform_.translation_ = {-transformedCenter.x, -transformedCenter.y, -transformedCenter.z};
	modelTransform_.parent_ = &worldTransform_;
	UpdateTransforms();
}

void Boss::Update(bool battleActive)
{
	if (IsDead())
	{
		return;
	}
	++colorTimer_;
	UpdateGamingMaterial();
	if (battleActive)
	{
		++battleTimer_;
		worldTransform_.translation_.x = basePosition_.x + std::sin(static_cast<float>(battleTimer_) * 0.018f) * 7.0f;
		worldTransform_.translation_.y = basePosition_.y + std::sin(static_cast<float>(battleTimer_) * 0.031f) * 1.6f;
		modelTransform_.rotation_.z = std::sin(static_cast<float>(battleTimer_) * 0.025f) * 0.06f;
		if (--fireTimer_ <= 0)
		{
			Fire();
			fireTimer_ = kFireInterval;
		}
	}
	UpdateTransforms();
}

void Boss::Draw(const Camera& camera)
{
	if (!IsDead())
	{
		model_->Draw(modelTransform_, camera, &gamingColor_);
	}
}

void Boss::OnCollision()
{
	if (health_ > 0)
	{
		GameAudio::GetInstance()->PlaySe(GameAudio::Se::Hit);
		--health_;
	}
}

void Boss::SetPresentationScale(float scale)
{
	presentationScale_ = (std::max)(scale, 0.0f);
	worldTransform_.scale_ = {presentationScale_, presentationScale_, presentationScale_};
	UpdateTransforms();
}

Vector3 Boss::GetWorldPosition() const
{
	return {
	    worldTransform_.matWorld_.m[3][0],
	    worldTransform_.matWorld_.m[3][1],
	    worldTransform_.matWorld_.m[3][2]};
}

void Boss::Fire()
{
	GameAudio::GetInstance()->PlaySe(GameAudio::Se::EnemyShot);
	Vector3 velocity = player_->GetWorldPosition() - GetWorldPosition();
	if (Length(velocity) < 0.0001f)
	{
		return;
	}
	Normalize(velocity);
	velocity *= 0.62f;
	EnemyBullet* bullet = new EnemyBullet();
	bullet->Initialize(bulletModel_, bulletTextureHandle_, GetWorldPosition(), velocity);
	// Do not set a homing target: boss shots keep the direction they had at fire time.
	gameScene_->AddEnemyBullet(bullet);
}

void Boss::UpdateTransforms()
{
	UpdateWorldTransform(worldTransform_);
	UpdateWorldTransform(modelTransform_);
}

void Boss::UpdateGamingMaterial()
{
	const Vector4 color = MakeGamingColor(colorTimer_);
	gamingColor_.SetColor(color);

	// The engine's OBJ path can preserve a material color more strongly than
	// ObjectColor. Update both so an untextured white boss is visibly RGB-tinted.
	for (const auto& mesh : model_->GetMeshes())
	{
		Material* material = mesh->GetMaterial();
		if (material == nullptr)
		{
			continue;
		}
		material->ambient_ = {color.x, color.y, color.z};
		material->diffuse_ = {color.x, color.y, color.z};
		material->specular_ = {0.85f, 0.85f, 0.85f};
		material->Update();
	}
}
