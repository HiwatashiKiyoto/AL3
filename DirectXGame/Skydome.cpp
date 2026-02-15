#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(KamataEngine::Model* model) 
{ 
	model_ = model;

	worldTransform_.Initialize();
}

void Skydome::Update() 
{ 
	worldTransform_.TransferMatrix();
}

void Skydome::Draw(const KamataEngine::Camera& camera) 
{
	Model::PreDraw();

	model_->Draw(worldTransform_, camera);

	Model::PostDraw();
}

