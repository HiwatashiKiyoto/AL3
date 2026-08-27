#include "Skydome.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;

void Skydome::Initialize(Model* model, uint32_t overrideTextureHandle, float scale)
{
	assert(model);

	model_ = model;
	overrideTextureHandle_ = overrideTextureHandle;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {scale, scale, scale};
}

void Skydome::Update(const Vector3& cameraPosition)
{
	// Keep the camera at the center of the dome so translation does not create
	// parallax in the infinitely distant background.
	worldTransform_.translation_ = cameraPosition;
	UpdateWorldTransform(worldTransform_);
}

void Skydome::Draw(const Camera& camera)
{
	if (overrideTextureHandle_ != 0u)
	{
		model_->Draw(worldTransform_, camera, overrideTextureHandle_);
	}
	else
	{
		model_->Draw(worldTransform_, camera);
	}
}
