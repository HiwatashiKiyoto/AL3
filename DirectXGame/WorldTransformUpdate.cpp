#include "WorldTransformUpdate.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation)
{
	Matrix4x4 matScale = MakeScaleMatrix(scale);
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation.z);
	Matrix4x4 matTranslate = MakeTranslateMatrix(translation);

	return matScale * matRotX * matRotY * matRotZ * matTranslate;
}
}

void UpdateWorldTransform(WorldTransform& worldTransform)
{
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
	if (worldTransform.parent_)
	{
		worldTransform.matWorld_ *= worldTransform.parent_->matWorld_;
	}
	worldTransform.TransferMatrix();
}
