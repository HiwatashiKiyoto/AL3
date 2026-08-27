#pragma once

#include "KamataEngine.h"

#include <list>
#include <utility>

class Enemy;
class Player;

class LockOn
{
public:
	~LockOn();

	// Initialize
	void Initialize(uint32_t textureHandle);

	// Update
	void Update(Player* player, const std::list<Enemy*>& enemies, const KamataEngine::Camera& camera);

	// Draw
	void Draw();

	// Is locking on
	bool ExistTarget() const { return target_ != nullptr; }

	// Get target
	Enemy* GetTarget() const { return target_; }

private:
	KamataEngine::Vector3 Project(
	    const KamataEngine::Vector3& worldPosition, float viewportX, float viewportY, float viewportWidth, float viewportHeight,
	    const KamataEngine::Matrix4x4& viewProjection);

private:
	// Lock-on reticle sprite drawn in both debug and release builds.
	KamataEngine::Sprite* spriteReticle_ = nullptr;

	// Lock-on target
	Enemy* target_ = nullptr;

	// Lock-on target screen position
	KamataEngine::Vector2 targetScreenPosition_ = {};
};
