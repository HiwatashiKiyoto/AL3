#pragma once

#include "KamataEngine.h"

class GameScene;
class Player;

// Static OBJ boss with its own health, movement and straight projectile attack.
class Boss
{
public:
	void Initialize(
	    KamataEngine::Model* model, KamataEngine::Model* bulletModel, Player* player,
	    GameScene* gameScene, uint32_t bulletTextureHandle, const KamataEngine::Vector3& position);
	void Update(bool battleActive);
	void Draw(const KamataEngine::Camera& camera);
	void OnCollision();
	void SetPresentationScale(float scale);

	KamataEngine::Vector3 GetWorldPosition() const;
	int32_t GetHealth() const { return health_; }
	int32_t GetMaxHealth() const { return kMaxHealth; }
	bool IsDead() const { return health_ <= 0; }

private:
	void Fire();
	void UpdateTransforms();
	void UpdateGamingMaterial();

	static constexpr int32_t kMaxHealth = 40;
	static constexpr int32_t kFireInterval = 85;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform modelTransform_;
	KamataEngine::ObjectColor gamingColor_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;
	uint32_t bulletTextureHandle_ = 0u;
	Player* player_ = nullptr;
	GameScene* gameScene_ = nullptr;
	KamataEngine::Vector3 basePosition_{};
	int32_t health_ = kMaxHealth;
	int32_t battleTimer_ = 0;
	int32_t fireTimer_ = kFireInterval;
	int32_t colorTimer_ = 0;
	float presentationScale_ = 1.0f;
};
