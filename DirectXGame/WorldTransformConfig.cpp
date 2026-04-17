#include "WorldTransformConfig.h"

void WorldTransformConfig(KamataEngine::WorldTransform& worldTransform) 
{
	float sx = worldTransform.scale_.x;
	float sy = worldTransform.scale_.y;
	float sz = worldTransform.scale_.z;

	float rx = worldTransform.rotation_.x;
	float ry = worldTransform.rotation_.y;
	float rz = worldTransform.rotation_.z;

	float tx = worldTransform.translation_.x;
	float ty = worldTransform.translation_.y;
	float tz = worldTransform.translation_.z;

	float sx_s = sinf(rx);
	float cx_c = cosf(rx);
	float sy_s = sinf(ry);
	float cy_c = cosf(ry);
	float sz_s = sinf(rz);
	float cz_c = cosf(rz);

	worldTransform.matWorld_.m[0][0] = sx * (cy_c * cz_c + sy_s * sx_s * sz_s);
	worldTransform.matWorld_.m[0][1] = sx * (sy_s * sx_s * cz_c - cy_c * sz_s);
	worldTransform.matWorld_.m[0][2] = sx * (sy_s * cx_c);
	worldTransform.matWorld_.m[0][3] = 0.0f;

	worldTransform.matWorld_.m[1][0] = sy * (cx_c * sz_s);
	worldTransform.matWorld_.m[1][1] = sy * (cx_c * cz_c);
	worldTransform.matWorld_.m[1][2] = sy * (-sx_s);
	worldTransform.matWorld_.m[1][3] = 0.0f;

	worldTransform.matWorld_.m[2][0] = sz * (cy_c * sx_s * sz_s - sy_s * cz_c);
	worldTransform.matWorld_.m[2][1] = sz * (cy_c * sx_s * cz_c + sy_s * sz_s);
	worldTransform.matWorld_.m[2][2] = sz * (cy_c * cx_c);
	worldTransform.matWorld_.m[2][3] = 0.0f;

	worldTransform.matWorld_.m[3][0] = tx;
	worldTransform.matWorld_.m[3][1] = ty;
	worldTransform.matWorld_.m[3][2] = tz;
	worldTransform.matWorld_.m[3][3] = 1.0f;


	worldTransform.TransferMatrix();
}
