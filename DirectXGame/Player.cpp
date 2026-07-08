#include "Player.h"

#include "Enemy.h"
#include "LockOn.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 result = MakeIdentityMatrix();
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;

	return result;
}

bool IsGamepadButtonTriggered(const XINPUT_STATE& current, const XINPUT_STATE& previous, WORD button)
{
	return (current.Gamepad.wButtons & button) != 0 && (previous.Gamepad.wButtons & button) == 0;
}
}

Player::~Player()
{
	for (PlayerBullet* bullet : bullets_)
	{
		delete bullet;
	}
}

void Player::Initialize(Model* model, Model* bulletModel, uint32_t textureHandle)
{
	assert(model);
	assert(bulletModel);
	(void)textureHandle;

	model_ = model;
	bulletModel_ = bulletModel;
	input_ = Input::GetInstance();

	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, 50.0f};

	worldTransform3DReticle_.Initialize();
	worldTransform3DReticle_.scale_ = {0.8f, 0.8f, 0.8f};
}

void Player::Update(const Camera& camera)
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

	XINPUT_STATE joyState{};
	if (input_->GetJoystickState(0, joyState))
	{
		move.x += static_cast<float>(joyState.Gamepad.sThumbLX) / static_cast<float>(SHRT_MAX) * kCharacterSpeed;
		move.y += static_cast<float>(joyState.Gamepad.sThumbLY) / static_cast<float>(SHRT_MAX) * kCharacterSpeed;
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
	Update3DReticle();
	Update2DReticle(camera);

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

void Player::DrawUI()
{
}

Vector3 Player::GetWorldPosition() const
{
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

Vector3 Player::Get3DReticleWorldPosition() const
{
	Vector3 worldPos;

	worldPos.x = worldTransform3DReticle_.matWorld_.m[3][0];
	worldPos.y = worldTransform3DReticle_.matWorld_.m[3][1];
	worldPos.z = worldTransform3DReticle_.matWorld_.m[3][2];

	return worldPos;
}

void Player::SetParent(const WorldTransform* parent)
{
	worldTransform_.parent_ = parent;
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
	XINPUT_STATE joyState{};
	XINPUT_STATE joyStatePre{};
	const bool isGamepadShot =
	    input_->GetJoystickState(0, joyState) && input_->GetJoystickStatePrevious(0, joyStatePre) &&
	    IsGamepadButtonTriggered(joyState, joyStatePre, XINPUT_GAMEPAD_RIGHT_SHOULDER);

	if (input_->TriggerKey(DIK_SPACE) || isGamepadShot)
	{
		const float kBulletSpeed = 1.0f;
		Vector3 velocity = {};
		if (lockOn_ != nullptr && lockOn_->ExistTarget())
		{
			velocity = lockOn_->GetTarget()->GetWorldPosition() - GetWorldPosition();
		}
		else
		{
			velocity = Get3DReticleWorldPosition() - GetWorldPosition();
		}
		Normalize(velocity);
		velocity *= kBulletSpeed;

		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(bulletModel_, GetWorldPosition(), velocity);

		bullets_.push_back(newBullet);
	}
}

void Player::Update3DReticle()
{
	const float kDistancePlayerTo3DReticle = 30.0f;
	Vector3 offset = {0.0f, 0.0f, 1.0f};
	offset = TransformNormal(offset, worldTransform_.matWorld_);
	Normalize(offset);
	offset *= kDistancePlayerTo3DReticle;

	worldTransform3DReticle_.rotation_ = worldTransform_.rotation_;
	worldTransform3DReticle_.translation_ = GetWorldPosition() + offset;
	UpdateWorldTransform(worldTransform3DReticle_);
}

void Player::Update2DReticle(const Camera& camera)
{
	const Matrix4x4 matViewport =
	    MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight), 0.0f, 1.0f);
	const Matrix4x4 matViewProjectionViewport = camera.matView * camera.matProjection * matViewport;

	Vector3 positionPlayer = GetWorldPosition();
	positionPlayer = TransformCoord(positionPlayer, matViewProjectionViewport);
	position2DReticle_ = {positionPlayer.x, positionPlayer.y};

	if (!std::isfinite(position2DReticle_.x) || !std::isfinite(position2DReticle_.y))
	{
		position2DReticle_ = {static_cast<float>(WinApp::kWindowWidth) / 2.0f, static_cast<float>(WinApp::kWindowHeight) / 2.0f};
	}

	const float kDistancePlayerTo2DReticle = 160.0f;
	position2DReticle_.x += std::sin(worldTransform_.rotation_.y) * kDistancePlayerTo2DReticle;
	position2DReticle_.y -= std::cos(worldTransform_.rotation_.y) * kDistancePlayerTo2DReticle;
	position2DReticle_.y += 90.0f;

	const float kReticleHalfSize = 90.0f;
	position2DReticle_.x = std::clamp(position2DReticle_.x, kReticleHalfSize, static_cast<float>(WinApp::kWindowWidth) - kReticleHalfSize);
	position2DReticle_.y = std::clamp(position2DReticle_.y, kReticleHalfSize, static_cast<float>(WinApp::kWindowHeight) - kReticleHalfSize);

#ifdef USE_IMGUI
	ImDrawList* drawList = ImGui::GetForegroundDrawList(ImGui::GetMainViewport());
	const ImVec2 center(position2DReticle_.x, position2DReticle_.y);
	const ImU32 color = IM_COL32(96, 96, 96, 220);
	const float radius = 34.0f;
	const float gap = 8.0f;
	const float lineLength = 22.0f;

	drawList->AddCircle(center, radius, color, 48, 5.0f);
	drawList->AddCircle(center, 7.0f, color, 32, 3.0f);
	drawList->AddLine(ImVec2(center.x - radius - lineLength, center.y), ImVec2(center.x - gap, center.y), color, 5.0f);
	drawList->AddLine(ImVec2(center.x + gap, center.y), ImVec2(center.x + radius + lineLength, center.y), color, 5.0f);
	drawList->AddLine(ImVec2(center.x, center.y - radius - lineLength), ImVec2(center.x, center.y - gap), color, 5.0f);
	drawList->AddLine(ImVec2(center.x, center.y + gap), ImVec2(center.x, center.y + radius + lineLength), color, 5.0f);
#endif
}
