#pragma once

#include "KamataEngine.h"

struct AABB
{
	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};

bool IsCollision(const AABB& aabb1, const AABB& aabb2);
