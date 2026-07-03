#pragma once

#include "KamataEngine.h"

/// <summary>
/// Player bullet
/// </summary>
class PlayerBullet
{
public:
	/// <summary>
	/// Initialize
	/// </summary>
	/// <param name="model">Model</param>
	/// <param name="position">Initial position</param>
	/// <param name="velocity">Velocity</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	/// <summary>
	/// Update
	/// </summary>
	void Update();

	/// <summary>
	/// Draw
	/// </summary>
	/// <param name="camera">Camera</param>
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Callback function called when collision is detected
	void OnCollision();

private:
	// Life time
	static const int32_t kLifeTime = 60 * 5;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// Velocity
	KamataEngine::Vector3 velocity_ = {};

	// Death timer
	int32_t deathTimer_ = kLifeTime;

	// Death flag
	bool isDead_ = false;
};
