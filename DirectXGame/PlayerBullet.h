#pragma once

#include "KamataEngine.h"

#include <array>

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

	// Draw passes used by GameScene for additive trails and silhouette outlines.
	void DrawTrail(const KamataEngine::Camera& camera);
	void DrawOutline(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Callback function called when collision is detected
	void OnCollision();

private:
	// Life time
	static const int32_t kLifeTime = 60 * 5;
	static const size_t kTrailCount = 5;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform outlineTransform_;

	// Short afterimages make the projectile readable without changing collision.
	std::array<KamataEngine::WorldTransform, kTrailCount> trailTransforms_;

	// Per-pass colors for the orange body, dark outline, and fading trail.
	KamataEngine::ObjectColor bodyColor_;
	KamataEngine::ObjectColor outlineColor_;
	KamataEngine::ObjectColor trailColor_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// Velocity
	KamataEngine::Vector3 velocity_ = {};

	// Death timer
	int32_t deathTimer_ = kLifeTime;

	// Number of frames since firing, used by the spawn-pop and trail sampling.
	int32_t ageFrames_ = 0;

	// Death flag
	bool isDead_ = false;
};
