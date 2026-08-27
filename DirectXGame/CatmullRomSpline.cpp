#include "CatmullRomSpline.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t)
{
	t = std::clamp(t, 0.0f, 1.0f);
	const float t2 = t * t;
	const float t3 = t2 * t;

	const Vector3 e3 = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
	const Vector3 e2 = 2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3;
	const Vector3 e1 = -p0 + p2;
	const Vector3 e0 = 2.0f * p1;

	return 0.5f * (e3 * t3 + e2 * t2 + e1 * t + e0);
}

Vector3 CatmullRomPosition(const std::vector<Vector3>& points, float t)
{
	assert(points.size() >= 4 && "Catmull-Rom requires at least four control points");

	t = std::clamp(t, 0.0f, 1.0f);
	const size_t division = points.size() - 1;
	const float areaWidth = 1.0f / static_cast<float>(division);

	size_t index = static_cast<size_t>(t / areaWidth);
	if (index >= division)
	{
		index = division - 1;
	}

	float localT = (t - static_cast<float>(index) * areaWidth) / areaWidth;
	localT = std::clamp(localT, 0.0f, 1.0f);

	const size_t index1 = index;
	const size_t index2 = index + 1;
	const size_t index0 = index == 0 ? index1 : index - 1;
	const size_t index3 = index2 + 1 >= points.size() ? index2 : index2 + 1;

	return CatmullRomInterpolation(points[index0], points[index1], points[index2], points[index3], localT);
}
