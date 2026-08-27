#pragma once

#include "KamataEngine.h"

// Skydome
class Skydome
{
public:
	// Initialize
	void Initialize(KamataEngine::Model* model, uint32_t overrideTextureHandle = 0u, float scale = 1.0f);

	// Update
	void Update(const KamataEngine::Vector3& cameraPosition);

	// Draw
	void Draw(const KamataEngine::Camera& camera);

private:
	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;
	uint32_t overrideTextureHandle_ = 0u;
};
