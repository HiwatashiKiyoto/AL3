#pragma once

#include "Collision.h"
#include "KamataEngine.h"

class Player;

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

	static float GetGroundOffset() { return -kModelBottom * kModelScale; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Vector3 velocity_ = {};

	static inline const float kWalkSpeed = 0.03f;
	static inline const float kWalkMotionAngleStart = -20.0f;
	static inline const float kWalkMotionAngleEnd = 20.0f;
	static inline const float kWalkMotionTime = 1.0f;
	static inline const float kModelScale = 0.8f;
	static inline const float kModelBottom = -0.073184f;
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.4f;

	float walkTimer_ = 0.0f;
};
