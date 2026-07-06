#include "RailCameraController.h"

#include "WorldTransformUpdate.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void RailCameraController::Initialize(const Vector3& position, const Vector3& rotation, float farZ)
{
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = rotation;
	UpdateWorldTransform(worldTransform_);

	camera_.Initialize();
	camera_.farZ = farZ;
}

void RailCameraController::Update()
{
	Input* input = Input::GetInstance();

	const float kRailSpeed = 0.2f;
	worldTransform_.translation_.z += kRailSpeed;

	const float kRotSpeed = 0.02f;
	if (input->PushKey(DIK_I))
	{
		worldTransform_.rotation_.x -= kRotSpeed;
	}
	if (input->PushKey(DIK_K))
	{
		worldTransform_.rotation_.x += kRotSpeed;
	}
	if (input->PushKey(DIK_J))
	{
		worldTransform_.rotation_.y -= kRotSpeed;
	}
	if (input->PushKey(DIK_L))
	{
		worldTransform_.rotation_.y += kRotSpeed;
	}
	if (input->PushKey(DIK_U))
	{
		worldTransform_.rotation_.z -= kRotSpeed;
	}
	if (input->PushKey(DIK_O))
	{
		worldTransform_.rotation_.z += kRotSpeed;
	}

#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Translation", &worldTransform_.translation_.x, 0.01f);
	ImGui::DragFloat3("Rotation", &worldTransform_.rotation_.x, 0.01f);
	ImGui::End();
#endif

	UpdateWorldTransform(worldTransform_);

	camera_.translation_ = worldTransform_.translation_;
	camera_.rotation_ = worldTransform_.rotation_;
	camera_.matView = Inverse(worldTransform_.matWorld_);
	camera_.UpdateProjectionMatrix();
	camera_.TransferMatrix();
}
