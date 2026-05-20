#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include<algorithm>

/// <summary>
/// 21まで終わった
/// </summary>

void CameraController::Initialize() 
{
	camera_ = new KamataEngine::Camera();
	camera_->Initialize();
}

void CameraController::Update()
{
	if (target_) 
	{
		// 追従対象のワールドトランスフォームを参照

		const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
		// 追従対象とオフセットからカメラの座標を計算

		camera_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
		camera_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
		camera_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
	
		//移動範囲制限
		camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left);   
		camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right);  
		camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom); 
		camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top);    
	
	}

	// カメラの行列を更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() 
{
	if (target_) 
	{
		// 追従対象のワールドトランスフォームを参照
		const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
		// 追従対象とオフセットからカメラの座標計算
		camera_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
		camera_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
		camera_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
	}
}