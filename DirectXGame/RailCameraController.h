#pragma once

#include "KamataEngine.h"

// Rail camera controller
class RailCameraController
{
public:
	// Initialize
	void Initialize(const KamataEngine::Vector3& position, const KamataEngine::Vector3& rotation, float farZ);

	// Update
	void Update();

	// Get camera
	const KamataEngine::Camera& GetCamera() const { return camera_; }

	// Get world transform
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

private:
	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Camera
	KamataEngine::Camera camera_;
};
