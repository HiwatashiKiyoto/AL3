#pragma once

#include "EnemyBullet.h"
#include "KamataEngine.h"

#include <list>

/// <summary>
/// Enemy
/// </summary>
class Enemy
{
public:
	~Enemy();

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
	// Fire bullet
	void Fire();

	// Initialize approach phase
	void ApproachPhaseInitialize();

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

	// Bullets
	std::list<EnemyBullet*> bullets_;

	// Phase
	Phase phase_ = Phase::Approach;

	// Fire interval
	static const int kFireInterval = 60;

	// Fire timer
	int32_t fireTimer_ = 0;
};
