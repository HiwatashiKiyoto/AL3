#pragma once

#include "KamataEngine.h"

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
	void Initialize(
	    KamataEngine::Model* model, uint32_t textureHandle,
	    const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

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

	// Set the homing target. The player is owned by GameScene.
	void SetPlayer(Player* player) { player_ = player; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Callback function called when collision is detected
	void OnCollision();

private:
	// Rotate the elongated bullet model along its current velocity.
	void UpdateRotation();

	// Life time
	static const int32_t kLifeTime = 60 * 5;

	// Percentage of the remaining angle corrected each frame.
	static constexpr float kHomingStrength = 0.02f;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Borrowed homing target data
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
