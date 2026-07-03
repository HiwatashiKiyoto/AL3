#pragma once

#include "KamataEngine.h"
#include "PlayerBullet.h"

#include <list>

// Player character
class Player
{
public:
	~Player();

	// Initialize
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle);

	// Update
	void Update();

	// Draw
	void Draw(const KamataEngine::Camera& camera);

	// Get position
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// Get rotation
	const KamataEngine::Vector3& GetRotation() const { return worldTransform_.rotation_; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

private:
	// Rotate
	void Rotate();

	// Attack
	void Attack();

private:
	// Input device
	KamataEngine::Input* input_ = nullptr;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// Bullets
	std::list<PlayerBullet*> bullets_;
};
