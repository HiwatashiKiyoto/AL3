#pragma once

#include "KamataEngine.h"
#include "PlayerBullet.h"

#include <list>

class LockOn;
class Boss;

// Player character
class Player
{
public:
	~Player();

	// Initialize
	void Initialize(
	    KamataEngine::Model* model, KamataEngine::Model* bulletModel,
	    uint32_t reticleTextureHandle, uint32_t bulletTextureHandle);

	// Update
	void Update(const KamataEngine::Camera& camera);

	// Draw
	void Draw(const KamataEngine::Camera& camera);
	void DrawTrainingOutlineDark(const KamataEngine::Camera& camera);
	void DrawTrainingOutlineGlow(const KamataEngine::Camera& camera);

	// Draw player-bullet effect passes.
	void DrawBulletTrails(const KamataEngine::Camera& camera);
	void DrawBulletOutlines(const KamataEngine::Camera& camera);

	// Draw UI
	void DrawUI();

	// Get position
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// Get rotation
	const KamataEngine::Vector3& GetRotation() const { return worldTransform_.rotation_; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Get 3D reticle world position
	KamataEngine::Vector3 Get3DReticleWorldPosition() const;

	// Get 2D reticle screen position
	const KamataEngine::Vector2& Get2DReticlePosition() const { return position2DReticle_; }

	// Set parent world transform
	void SetParent(const KamataEngine::WorldTransform* parent);

	// Set lock-on system
	void SetLockOn(LockOn* lockOn) { lockOn_ = lockOn; }
	void SetBossTarget(Boss* boss) { bossTarget_ = boss; }

	// Get bullets
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// Callback function called when collision is detected
	void OnCollision();

	// Remaining health
	int32_t GetHealth() const { return health_; }
	int32_t GetMaxHealth() const { return kMaxHealth; }
	bool IsDead() const { return health_ <= 0; }
	void SetAttackEnabled(bool enabled) { isAttackEnabled_ = enabled; }
	void SetDebugUIEnabled(bool enabled) { isDebugUIEnabled_ = enabled; }

private:
	// Attack
	void Attack();

	// Update 3D reticle
	void Update3DReticle();

	// Update 2D reticle
	void Update2DReticle(const KamataEngine::Camera& camera);

private:
	// Input device
	KamataEngine::Input* input_ = nullptr;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform outlineDarkTransform_;
	KamataEngine::WorldTransform outlineGlowTransform_;
	KamataEngine::ObjectColor outlineDarkColor_;
	KamataEngine::ObjectColor outlineGlowColor_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Borrowed bullet model data
	KamataEngine::Model* bulletModel_ = nullptr;
	uint32_t bulletTextureHandle_ = 0u;

	// 3D reticle world transform data
	KamataEngine::WorldTransform worldTransform3DReticle_;

	// 2D reticle screen position
	KamataEngine::Vector2 position2DReticle_ = {};

	// 2D reticle sprite
	KamataEngine::Sprite* sprite2DReticle_ = nullptr;

	// Borrowed lock-on system
	LockOn* lockOn_ = nullptr;
	Boss* bossTarget_ = nullptr;

	// Bullets
	std::list<PlayerBullet*> bullets_;

	// Health and post-hit invincibility
	static constexpr int32_t kMaxHealth = 3;
	static constexpr int32_t kInvincibleFrameCount = 90;
	static constexpr int32_t kAttackCooldownFrameCount = 30;
	int32_t health_ = kMaxHealth;
	int32_t invincibleTimer_ = 0;
	int32_t attackCooldownTimer_ = 0;
	bool isAttackEnabled_ = true;
	bool isDebugUIEnabled_ = true;
};
