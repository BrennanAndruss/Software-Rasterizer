#include "Vec3.h"
#include "Vec2.h"

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

inline Vec3::operator Vec2() const
{
	return Vec2(x, y);
}
