#include "Vec2.h"
#include "Vec3.h"

Vec2::Vec2() : x(0.0f), y(0.0f) {}

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2::operator Vec3() const
{
	return Vec3(x, y, 0.0f);
}
