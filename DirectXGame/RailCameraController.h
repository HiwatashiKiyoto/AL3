#pragma once

#include "KamataEngine.h"

#include <vector>

// Rail camera controller
class RailCameraController
{
public:
	// Initialize
	void Initialize(const KamataEngine::Vector3& position, const KamataEngine::Vector3& rotation, float farZ);

	// Update
	void Update(bool advanceRail = true);

	// Draw the future Catmull-Rom camera rail without moving the camera on it yet.
	void DrawRailPath(const KamataEngine::Camera& camera) const;

	// Get camera
	const KamataEngine::Camera& GetCamera() const { return camera_; }

	// Get world transform
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	void SetDebugUIEnabled(bool enabled) { isDebugUIEnabled_ = enabled; }

private:
	// Sample the precomputed rail by world-space distance.
	KamataEngine::Vector3 SampleRailByDistance(float distance) const;

	// World transform data
	KamataEngine::WorldTransform worldTransform_;

	// Rotation that the camera smoothly follows
	KamataEngine::Vector3 targetRotation_ = {};
	KamataEngine::Vector3 aimRotation_ = {};
	KamataEngine::Vector3 railRotation_ = {};

	// Progress along the Catmull-Rom camera rail.
	float railT_ = 0.0f;
	float railDistance_ = 0.0f;
	float railLength_ = 0.0f;

	// Control points for the camera rail used by exercise 03_12_ex1.
	std::vector<KamataEngine::Vector3> controlPoints_;
	std::vector<KamataEngine::Vector3> railSamples_;
	std::vector<float> railCumulativeLengths_;

	// Camera
	KamataEngine::Camera camera_;
	bool isDebugUIEnabled_ = true;
};
