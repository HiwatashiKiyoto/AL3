#include "Player.h"

#include <algorithm>
#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation)
{
	Matrix4x4 matScale = MakeScaleMatrix(scale);
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation.z);
	Matrix4x4 matTranslate = MakeTranslateMatrix(translation);

	return matScale * matRotX * matRotY * matRotZ * matTranslate;
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
	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.2f;

	const bool isMoveLeft = input_->PushKey(DIK_LEFT) || input_->PushKey(DIK_A);
	const bool isMoveRight = input_->PushKey(DIK_RIGHT) || input_->PushKey(DIK_D);
	const bool isMoveForward = input_->PushKey(DIK_UP) || input_->PushKey(DIK_W);
	const bool isMoveBack = input_->PushKey(DIK_DOWN) || input_->PushKey(DIK_S);

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
		move.z += kCharacterSpeed;
	}
	if (isMoveBack)
	{
		move.z -= kCharacterSpeed;
	}

	worldTransform_.translation_ += move;

	const float kMoveLimitX = 34.0f;
	const float kMoveLimitZ = 18.0f;
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.z = std::clamp(worldTransform_.translation_.z, -kMoveLimitZ, kMoveLimitZ);

#ifdef USE_IMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Position", &worldTransform_.translation_.x, 0.01f);
	ImGui::End();
#endif

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.z = std::clamp(worldTransform_.translation_.z, -kMoveLimitZ, kMoveLimitZ);

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}
