#pragma once

#include "KamataEngine.h"
#include <array>

/// <summary>
/// ヒット演出用エフェクト
/// </summary>
class HitEffect
{
public:
	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }
	static HitEffect* Create(const KamataEngine::Vector3& position);

	void Initialize(const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	bool IsDead() const { return state_ == State::kDead; }

private:
	enum class State
	{
		kSpread,
		kFade,
		kDead,
	};

	static inline const uint32_t kNumEllipses = 4;
	static inline const float kSpreadDuration = 0.22f;
	static inline const float kFadeDuration = 0.72f;
	static inline const float kEffectFrontOffset = -4.0f;
	static inline const float kCircleStartScale = 1.8f;
	static inline const float kCircleEndScale = 7.0f;
	static inline const float kEllipseWidth = 0.8f;
	static inline const float kEllipseLength = 11.0f;
	static inline const float kEllipseStartScale = 0.35f;
	static inline const float kEllipseEndScale = 1.0f;

	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;

	KamataEngine::WorldTransform circleWorldTransform_;
	std::array<KamataEngine::WorldTransform, kNumEllipses> ellipseWorldTransforms_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, kNumEllipses> ellipseRotations_ = {};
	KamataEngine::Vector3 position_ = {};
	State state_ = State::kSpread;
	float counter_ = 0.0f;

	void ChangeState(State state);
	void UpdateTransforms(float spreadT);
};
