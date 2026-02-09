#include "Player.h"
#include <cassert>

void Player::Initialize(KamataEngine::Model* model, uint32_t textureHandle,KamataEngine::Camera* camera) 
{
	// ヌルポチェック
	assert(model);

	// 引数の内容をメンバ変数に記憶
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	

	worldTransform_.Initialize();

}

void Player::Update() 
{
	//行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() 
{
	assert(camera_);
	// 3Dモデルの描画
	model_->Draw(worldTransform_, *camera_, textureHandle_);
}
