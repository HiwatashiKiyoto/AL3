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
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* bulletModel, uint32_t textureHandle);

	// Update
	void Update(const KamataEngine::Camera& camera);

	// Draw
	void Draw(const KamataEngine::Camera& camera);

	// Draw UI
	void DrawUI();

	// Get position
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// Get rotation
	const KamataEngine::Vector3& GetRotation() const { return worldTransform_.rotation_; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Get 3D reticle world position
	KamataEngine::Vector3 Get3DReticleWorldPosition() const;

	// Set parent world transform
	void SetParent(const KamataEngine::WorldTransform* parent);

	// Get bullets
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// Callback function called when collision is detected
	void OnCollision();

private:
	// Rotate
	void Rotate();

	// Attack
	void Attack();

	// Update 3D reticle
	void Update3DReticle();

	// Update 2D reticle
	void Update2DReticle(const KamataEngine::Camera& camera);

private:
	// Input device
	KamataEngine::Input* input_ = nullptr;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Borrowed bullet model data
	KamataEngine::Model* bulletModel_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;

	// 3D reticle world transform data
	KamataEngine::WorldTransform worldTransform3DReticle_;

	// 2D reticle sprite
	KamataEngine::Sprite* sprite2DReticle_ = nullptr;

	// 2D reticle screen position
	KamataEngine::Vector2 position2DReticle_ = {};

	// Bullets
	std::list<PlayerBullet*> bullets_;
};
