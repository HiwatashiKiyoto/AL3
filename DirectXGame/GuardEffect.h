#pragma once

#include "KamataEngine.h"

/// <summary>
/// ガード成功時の演出用エフェクト
/// </summary>
class GuardEffect
{
public:
	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }
	static GuardEffect* Create(const KamataEngine::Vector3& position);

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

	static inline const float kSpreadDuration = 0.16f;
	static inline const float kFadeDuration = 0.34f;
	static inline const float kEffectFrontOffset = -4.8f;
	static inline const float kStartScale = 0.45f;
	static inline const float kEndScale = 2.1f;

	static KamataEngine::Model* model_;
	static KamataEngine::Camera* camera_;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_ = {0.75f, 0.95f, 1.0f, 1.0f};
	KamataEngine::Vector3 position_ = {};
	State state_ = State::kSpread;
	float counter_ = 0.0f;

	void ChangeState(State state);
	void UpdateTransform(float scaleT);
};
