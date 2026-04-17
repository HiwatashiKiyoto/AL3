#include "Player.h"
#include"WorldTransformConfig.h"

void Player::Initialize(KamataEngine::Model* model)
{ 
	model_ = model;
	worldTransform_.Initialize();

	// 初期位置を調整
	worldTransform_.translation_.x = 10.0f;
	worldTransform_.translation_.y = 2.0f;
}



void Player::Update() 
{ 
	WorldTransformConfig(worldTransform_); 
}

void Player::Draw(const KamataEngine::Camera& camera) 
{ 
	KamataEngine::Model::PreDraw();
	model_->Draw(worldTransform_, camera); 
	KamataEngine::Model::PostDraw();
}
