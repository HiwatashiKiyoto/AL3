#pragma once

#include "KamataEngine.h"

// Skydome
class Skydome
{
public:
	// Initialize
	void Initialize(KamataEngine::Model* model);

	// Update
	void Update();

	// Draw
	void Draw(const KamataEngine::Camera& camera);

private:
	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;
};
