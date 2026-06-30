#include "Player.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

Player::~Player()
{
	delete bullet_;
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

	Attack();

	if (bullet_)
	{
		bullet_->Update();
	}

	UpdateWorldTransform(worldTransform_);
}

void Player::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);

	if (bullet_)
	{
		bullet_->Draw(camera);
	}
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
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, worldTransform_.translation_);

		delete bullet_;
		bullet_ = newBullet;
	}
}
