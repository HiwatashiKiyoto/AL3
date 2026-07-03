#include "Player.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

Player::~Player()
{
	for (PlayerBullet* bullet : bullets_)
	{
		delete bullet;
	}
}

void Player::Initialize(Model* model, uint32_t textureHandle)
{
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;
	input_ = Input::GetInstance();

	worldTransform_.Initialize();
}

void Player::Update()
{
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead())
		{
			delete bullet;
			return true;
		}

		return false;
	});

	Rotate();

	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.2f;

	const bool isMoveLeft = input_->PushKey(DIK_LEFT);
	const bool isMoveRight = input_->PushKey(DIK_RIGHT);
	const bool isMoveForward = input_->PushKey(DIK_UP);
	const bool isMoveBack = input_->PushKey(DIK_DOWN);

	if (isMoveLeft)
	{
		move.x -= kCharacterSpeed;
	}
	if (isMoveRight)
	{
		move.x += kCharacterSpeed;
	}

	if (isMoveForward)
	{
		move.y += kCharacterSpeed;
	}
	if (isMoveBack)
	{
		move.y -= kCharacterSpeed;
	}

	worldTransform_.translation_ += move;

	const float kMoveLimitX = 34.0f;
	const float kMoveLimitY = 18.0f;
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, kMoveLimitY);

#ifdef USE_IMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Position", &worldTransform_.translation_.x, 0.01f);
	ImGui::End();
#endif

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, kMoveLimitY);

	UpdateWorldTransform(worldTransform_);

	Attack();

	for (PlayerBullet* bullet : bullets_)
	{
		bullet->Update();
	}
}

void Player::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);

	for (PlayerBullet* bullet : bullets_)
	{
		bullet->Draw(camera);
	}
}

Vector3 Player::GetWorldPosition() const
{
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void Player::OnCollision()
{
}

void Player::Rotate()
{
	const float kRotSpeed = 0.02f;

	if (input_->PushKey(DIK_A))
	{
		worldTransform_.rotation_.y -= kRotSpeed;
	}
	else if (input_->PushKey(DIK_D))
	{
		worldTransform_.rotation_.y += kRotSpeed;
	}
}

void Player::Attack()
{
	if (input_->TriggerKey(DIK_SPACE))
	{
		const float kBulletSpeed = 1.0f;
		Vector3 velocity = {0.0f, 0.0f, kBulletSpeed};
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, worldTransform_.translation_, velocity);

		bullets_.push_back(newBullet);
	}
}
