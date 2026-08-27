#include "Enemy.h"

#include "AnimatedModel.h"
#include "GameAudio.h"
#include "GameScene.h"
#include "Player.h"
#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

Enemy::~Enemy()
{
}

void Enemy::Initialize(
    Model* model, Model* bulletModel, AnimatedModel* animatedModel,
    uint32_t modelTextureHandle, uint32_t bulletTextureHandle, const Vector3& position,
    BehaviorPattern behaviorPattern)
{
	assert(model || animatedModel);
	assert(bulletModel);

	model_ = model;
	bulletModel_ = bulletModel;
	animatedModel_ = animatedModel;
	textureHandle_ = modelTextureHandle;
	bulletTextureHandle_ = bulletTextureHandle;
	phase_ = Phase::Approach;
	// Temporarily restore the original rail-shooter movement: every enemy
	// travels straight without lateral, zigzag or player-following movement.
	(void)behaviorPattern;
	behaviorPattern_ = BehaviorPattern::Straight;
	behaviorTimer_ = 0;
	ApproachPhaseInitialize();

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransformModel_.Initialize();
	if (animatedModel_)
	{
		// Match the previous OBJ enemy's largest visible dimension (5.881322 * 0.35)
		// and center the FBX without changing movement, firing or collision coordinates.
		constexpr float kEnemyVisualExtent = 5.881322f * 0.35f;
		const float scale = kEnemyVisualExtent / animatedModel_->GetBoundsMaxExtent();
		const Vector3 center = animatedModel_->GetBoundsCenter();
		// Blender/FBX and the game use opposite front/left-right conventions.
		// Mirror X, then turn the FBX Y-facing direction toward the game front.
		worldTransformModel_.scale_ = {-scale, scale, scale};
		// Rotate around screen depth as well so the cat's head is above its body.
		worldTransformModel_.rotation_ = {
		    0.0f, 3.14159265358979323846f, 3.14159265358979323846f};
		// Center the visible FBX after applying its mirror and rotations. The old
		// component-wise sign correction left the rendered cat far away from the
		// logical position used by lock-on and collision.
		const Matrix4x4 modelOrientation =
		    MakeScaleMatrix(worldTransformModel_.scale_) *
		    MakeRotateXMatrix(worldTransformModel_.rotation_.x) *
		    MakeRotateYMatrix(worldTransformModel_.rotation_.y) *
		    MakeRotateZMatrix(worldTransformModel_.rotation_.z);
		const Vector3 transformedCenter = TransformNormal(center, modelOrientation);
		worldTransformModel_.translation_ = {-transformedCenter.x, -transformedCenter.y, -transformedCenter.z};
	}
	else
	{
		worldTransformModel_.scale_ = {0.35f, 0.35f, 0.35f};
		// The imported OBJ is offset from its origin. Recenter only its appearance so
		// movement, firing and collision continue to use worldTransform_'s origin.
		worldTransformModel_.translation_ = {0.02f, -1.95f, -2.76f};
	}
	worldTransformModel_.parent_ = &worldTransform_;
	UpdateWorldTransform(worldTransform_);
	UpdateWorldTransform(worldTransformModel_);
}

void Enemy::Update()
{
	if (isDead_)
	{
		return;
	}

	++behaviorTimer_;
	if (isTrainingDummy_)
	{
		UpdateWorldTransform(worldTransform_);
		UpdateWorldTransform(worldTransformModel_);
		return;
	}
	if (--fireTimer_ <= 0)
	{
		Fire();
		fireTimer_ = kFireInterval;
	}

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
	UpdateWorldTransform(worldTransformModel_);

#ifdef USE_IMGUI
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Position", &worldTransform_.translation_.x, 0.01f);
	ImGui::Text("Behavior: %s", GetBehaviorPatternName());
	ImGui::End();
#endif
}

void Enemy::Draw(const Camera& camera)
{
	if (animatedModel_)
	{
		animatedModel_->Draw(worldTransformModel_, camera);
	}
	else
	{
		model_->Draw(worldTransformModel_, camera, textureHandle_);
	}
}

void Enemy::Fire()
{
	assert(player_);
	assert(gameScene_);
	GameAudio::GetInstance()->PlaySe(GameAudio::Se::EnemyShot);

	const float kBulletSpeed = 1.0f;

	Vector3 playerPosition = player_->GetWorldPosition();
	Vector3 enemyPosition = GetWorldPosition();

	Vector3 velocity = playerPosition - enemyPosition;
	Normalize(velocity);
	velocity *= kBulletSpeed;

	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(bulletModel_, bulletTextureHandle_, enemyPosition, velocity);
	newBullet->SetPlayer(player_);

	gameScene_->AddEnemyBullet(newBullet);
}

void Enemy::ApproachPhaseInitialize()
{
	fireTimer_ = kFireInterval;
}

void Enemy::UpdateApproach()
{
	const Vector3 velocity = {0.0f, 0.0f, -0.1f};
	worldTransform_.translation_ += velocity;

	if (worldTransform_.translation_.z < 0.0f)
	{
		// The enemy has passed the player and left the playable side of the
		// screen.  Count the escape as a hit, then remove this enemy so that an
		// unreachable target cannot prevent the battle from ending.
		if (player_)
		{
			player_->OnCollision();
		}
		isDead_ = true;
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

const char* Enemy::GetBehaviorPatternName() const
{
	switch (behaviorPattern_)
	{
	case BehaviorPattern::Straight:
		return "Straight";
	case BehaviorPattern::MoveLeft:
		return "MoveLeft";
	case BehaviorPattern::MoveRight:
		return "MoveRight";
	case BehaviorPattern::Zigzag:
		return "Zigzag";
	}

	return "Unknown";
}

Vector3 Enemy::GetWorldPosition() const
{
	if (animatedModel_)
	{
		// Aim at the animated upper torso. The whole-model bounds center swings
		// around with the limbs and made the marker visibly miss the cat.
		return TransformCoord(animatedModel_->GetCurrentAimPoint(), worldTransformModel_.matWorld_);
	}

	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void Enemy::OnCollision()
{
	if (isDead_)
	{
		return;
	}
	GameAudio::GetInstance()->PlaySe(GameAudio::Se::Hit);
	isDead_ = true;
}
