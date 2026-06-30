#pragma once

#include "KamataEngine.h"

/// <summary>
/// Enemy
/// </summary>
class Enemy
{
public:
	// Behavior phase
	enum class Phase
	{
		Approach,
		Leave,
	};

	/// <summary>
	/// Initialize
	/// </summary>
	/// <param name="model">Model</param>
	/// <param name="position">Initial position</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position);

	/// <summary>
	/// Update
	/// </summary>
	void Update();

	/// <summary>
	/// Draw
	/// </summary>
	/// <param name="camera">Camera</param>
	void Draw(const KamataEngine::Camera& camera);

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	const char* GetPhaseName() const;

private:
	// Update approach phase
	void UpdateApproach();

	// Update leave phase
	void UpdateLeave();

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// Phase
	Phase phase_ = Phase::Approach;
};
