#pragma once

#include "Collision.h"
#include "KamataEngine.h"

class Player;
class GameScene;

/// <summary>
/// Enemy
/// </summary>
class Enemy
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw(const KamataEngine::Camera& camera);
	KamataEngine::Vector3 GetWorldPosition() const;
	AABB GetAABB() const;
	void OnCollision(const Player* player);
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	bool IsDead() const { return isDead_; }
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	static float GetGroundOffset() { return -kModelBottom * kModelScale; }

private:
	enum class Behavior
	{
		kUnknown = 0,
		kWalk,
		kDeath,
	};

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	GameScene* gameScene_ = nullptr;
	KamataEngine::Vector3 velocity_ = {};
	Behavior behavior_ = Behavior::kWalk;
	Behavior behaviorRequest_ = Behavior::kUnknown;
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	static inline const float kWalkSpeed = 0.03f;
	static inline const float kWalkMotionAngleStart = -20.0f;
	static inline const float kWalkMotionAngleEnd = 20.0f;
	static inline const float kWalkMotionTime = 1.0f;
	static inline const float kDeathMotionTime = 0.8f;
	static inline const float kDeathSpinSpeedY = 22.0f;
	static inline const float kDeathSpinSpeedX = 8.0f;
	static inline const float kDeathFloatSpeed = 0.04f;
	static inline const float kModelScale = 0.8f;
	static inline const float kModelBottom = -0.073184f;
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.4f;

	float walkTimer_ = 0.0f;
	float deathTimer_ = 0.0f;

	void UpdateBehaviorTransition();
	void BehaviorWalkInitialize();
	void BehaviorWalkUpdate();
	void BehaviorDeathInitialize();
	void BehaviorDeathUpdate();
};
