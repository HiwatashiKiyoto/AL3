#include "Ground.h"

#include "WorldTransformUpdate.h"

#include <cassert>

using namespace KamataEngine;

void Ground::Initialize(Model* model)
{
	assert(model);

	model_ = model;

	worldTransform_.Initialize();
	UpdateWorldTransform(worldTransform_);
}

void Ground::Update()
{
	UpdateWorldTransform(worldTransform_);
}

void Ground::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}
