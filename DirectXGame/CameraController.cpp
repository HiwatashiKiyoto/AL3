#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include<algorithm>

KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2, float t)
{
	KamataEngine::Vector3 result;
	result.x = v1.x + (v2.x - v1.x) * t;
	result.y = v1.y + (v2.y - v1.y) * t;
	result.z = v1.z + (v2.z - v1.z) * t;
	return result;
}

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
		
		//プレイヤーの現在の速度を取得
		KamataEngine::Vector3 targetVelocity = target_->GetVelocity();

		// 追従対象とオフセットからカメラの座標を計算
		destination_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
		destination_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
		destination_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;
	
		//座標補間によりゆったり追従
		camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);

		//移動範囲制限
		camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left + kMargin.left);   
		camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right - kMargin.right);  
		camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom + kMargin.bottom); 
		camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top - kMargin.top);    
	
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

	camera_->UpdateMatrix();
}
