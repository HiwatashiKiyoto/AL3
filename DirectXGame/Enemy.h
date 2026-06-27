#pragma once

#include "KamataEngine.h"

/// <summary>
/// Enemy
/// </summary>
class Enemy
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw(const KamataEngine::Camera& camera);

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

	float walkTimer_ = 0.0f;
};
