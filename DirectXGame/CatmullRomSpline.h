#pragma once

#include "KamataEngine.h"

#include <vector>

// Interpolate the segment from p1 to p2 using four surrounding control points.
KamataEngine::Vector3 CatmullRomInterpolation(
    const KamataEngine::Vector3& p0, const KamataEngine::Vector3& p1, const KamataEngine::Vector3& p2,
    const KamataEngine::Vector3& p3, float t);

// Get a position in the normalized [0, 1] range of an entire Catmull-Rom path.
KamataEngine::Vector3 CatmullRomPosition(const std::vector<KamataEngine::Vector3>& points, float t);
