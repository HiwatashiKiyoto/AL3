#include "Skydome.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;

void Skydome::Initialize(Model* model)
{
	assert(model);

	model_ = model;

	worldTransform_.Initialize();
}

void Skydome::Update()
{
	UpdateWorldTransform(worldTransform_);
}

void Skydome::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}
