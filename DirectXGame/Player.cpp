#include "Player.h"

#include "GameAudio.h"

#include "Enemy.h"
#include "Boss.h"
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
bool IsGamepadButtonTriggered(const XINPUT_STATE& current, const XINPUT_STATE& previous, WORD button)
{
	return (current.Gamepad.wButtons & button) != 0 && (previous.Gamepad.wButtons & button) == 0;
}
}

Player::~Player()
{
	delete sprite2DReticle_;

	for (PlayerBullet* bullet : bullets_)
	{
		delete bullet;
	}
}

void Player::Initialize(Model* model, Model* bulletModel, uint32_t reticleTextureHandle, uint32_t bulletTextureHandle)
{
	assert(model);
	assert(bulletModel);

	model_ = model;
	bulletModel_ = bulletModel;
	bulletTextureHandle_ = bulletTextureHandle;
	input_ = Input::GetInstance();
	health_ = kMaxHealth;
	invincibleTimer_ = 0;
	attackCooldownTimer_ = 0;

	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, 50.0f};
	outlineDarkTransform_.Initialize();
	outlineDarkTransform_.scale_ = {1.38f, 1.38f, 1.38f};
	outlineGlowTransform_.Initialize();
	outlineGlowTransform_.scale_ = {1.20f, 1.20f, 1.20f};
	outlineDarkColor_.Initialize();
	outlineDarkColor_.SetColor({0.015f, 0.035f, 0.09f, 1.0f});
	outlineGlowColor_.Initialize();
	outlineGlowColor_.SetColor({1.0f, 0.28f, 0.02f, 1.0f});

	worldTransform3DReticle_.Initialize();
	worldTransform3DReticle_.scale_ = {0.8f, 0.8f, 0.8f};

	position2DReticle_ = {static_cast<float>(WinApp::kWindowWidth) / 2.0f, static_cast<float>(WinApp::kWindowHeight) / 2.0f};
	sprite2DReticle_ = Sprite::Create(reticleTextureHandle, position2DReticle_, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
	assert(sprite2DReticle_);
}

void Player::Update(const Camera& camera)
{
	(void)camera;

	if (invincibleTimer_ > 0)
	{
		--invincibleTimer_;
	}
	if (attackCooldownTimer_ > 0)
	{
		--attackCooldownTimer_;
	}

	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead())
		{
			delete bullet;
			return true;
		}

		return false;
	});

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
	if (isDebugUIEnabled_)
	{
		ImGui::Begin("Player");
		ImGui::DragFloat3("Position", &worldTransform_.translation_.x, 0.01f);
		ImGui::End();
	}
#endif

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, kMoveLimitY);

	UpdateWorldTransform(worldTransform_);
	outlineDarkTransform_.rotation_ = worldTransform_.rotation_;
	outlineDarkTransform_.translation_ = worldTransform_.translation_;
	outlineGlowTransform_.rotation_ = worldTransform_.rotation_;
	outlineGlowTransform_.translation_ = worldTransform_.translation_;
	UpdateWorldTransform(outlineDarkTransform_);
	UpdateWorldTransform(outlineGlowTransform_);
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
	// Blink only the player model after taking damage. Bullets remain visible.
	if (invincibleTimer_ == 0 || ((invincibleTimer_ / 4) % 2) == 0)
	{
		model_->Draw(worldTransform_, camera);
	}

	for (PlayerBullet* bullet : bullets_)
	{
		bullet->Draw(camera);
	}
}

void Player::DrawTrainingOutlineDark(const Camera& camera)
{
	model_->Draw(outlineDarkTransform_, camera, &outlineDarkColor_);
}

void Player::DrawTrainingOutlineGlow(const Camera& camera)
{
	model_->Draw(outlineGlowTransform_, camera, &outlineGlowColor_);
}

void Player::DrawBulletTrails(const Camera& camera)
{
	for (PlayerBullet* bullet : bullets_)
	{
		bullet->DrawTrail(camera);
	}
}

void Player::DrawBulletOutlines(const Camera& camera)
{
	for (PlayerBullet* bullet : bullets_)
	{
		bullet->DrawOutline(camera);
	}
}

void Player::DrawUI()
{
	sprite2DReticle_->SetPosition(position2DReticle_);
	sprite2DReticle_->Draw();
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
	outlineDarkTransform_.parent_ = parent;
	outlineGlowTransform_.parent_ = parent;
}

void Player::OnCollision()
{
	if (invincibleTimer_ > 0 || IsDead())
	{
		return;
	}

	GameAudio::GetInstance()->PlaySe(GameAudio::Se::PlayerHit);
	--health_;
	if (health_ > 0)
	{
		invincibleTimer_ = kInvincibleFrameCount;
	}
}

void Player::Attack()
{
	if (!isAttackEnabled_ || attackCooldownTimer_ > 0)
	{
		return;
	}

	XINPUT_STATE joyState{};
	XINPUT_STATE joyStatePre{};
	const bool isGamepadShot =
	    input_->GetJoystickState(0, joyState) && input_->GetJoystickStatePrevious(0, joyStatePre) &&
	    IsGamepadButtonTriggered(joyState, joyStatePre, XINPUT_GAMEPAD_A);

	if (input_->TriggerKey(DIK_SPACE) || isGamepadShot)
	{
		attackCooldownTimer_ = kAttackCooldownFrameCount;
		GameAudio::GetInstance()->PlaySe(GameAudio::Se::PlayerShot);
		const float kBulletSpeed = 1.0f;
		Vector3 velocity = {};
		if (bossTarget_ != nullptr && !bossTarget_->IsDead())
		{
			velocity = bossTarget_->GetWorldPosition() - GetWorldPosition();
		}
		else if (lockOn_ != nullptr && lockOn_->ExistTarget())
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
		newBullet->Initialize(bulletModel_, bulletTextureHandle_, GetWorldPosition(), velocity);

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
	(void)camera;
	position2DReticle_ = {static_cast<float>(WinApp::kWindowWidth) / 2.0f, static_cast<float>(WinApp::kWindowHeight) / 2.0f};
}
