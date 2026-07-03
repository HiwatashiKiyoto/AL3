#pragma once

#include "KamataEngine.h"

// Player class forward declaration
class Player;

/// <summary>
/// Enemy bullet
/// </summary>
class EnemyBullet
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

	// Set player
	void SetPlayer(Player* player) { player_ = player; }

private:
	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Update rotation
	void UpdateRotation();

	// Life time
	static const int32_t kLifeTime = 60 * 5;

	// Homing strength
	static constexpr float kHomingStrength = 0.02f;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Borrowed player data
	Player* player_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// Velocity
	KamataEngine::Vector3 velocity_ = {};

	// Death timer
	int32_t deathTimer_ = kLifeTime;

	// Death flag
	bool isDead_ = false;
};
