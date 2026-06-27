#pragma once

#include "KamataEngine.h"
#include <array>

class DeathParticles
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	bool IsFinished() const { return isFinished_; }

private:
	static inline const uint32_t kNumParticles = 8;
	static inline const float kDuration = 1.0f;
	static inline const float kSpeed = 0.08f;
	static inline const float kAngleUnit = 2.0f * 3.141592654f / static_cast<float>(kNumParticles);
	static inline const float kParticleScale = 0.18f;

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	bool isFinished_ = false;
	float counter_ = 0.0f;
};
