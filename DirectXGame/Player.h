#pragma once

#include "KamataEngine.h"

// Player character
class Player
{
public:
	// Initialize
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle);

	// Update
	void Update();

	// Draw
	void Draw(const KamataEngine::Camera& camera);

private:
	// Input device
	KamataEngine::Input* input_ = nullptr;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;
};
