#pragma once

#include <cmath>
struct Vec3;

struct Vec2
{
	float x, y;

	Vec2();
	Vec2(float x, float y);

	operator Vec3() const;
};

inline Vec2 operator+(const Vec2& a, const Vec2& b)
{
	return Vec2(a.x + b.x, a.y + b.y);
}

inline Vec2 operator-(const Vec2& a, const Vec2& b)
{
	return Vec2(a.x - b.x, a.y - b.y);
}

inline Vec2 operator*(const Vec2& a, const Vec2& b)
{
	return Vec2(a.x * b.x, a.y * b.y);
}

inline Vec2 operator*(const Vec2& a, float d)
{
	return Vec2(a.x * d, a.y * d);
}

inline Vec2 operator*(float d, const Vec2& a)
{
	return Vec2(d * a.x, d * a.y);
}

inline float dot(const Vec2& a, const Vec2& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float mag(const Vec2& a)
{
	return sqrt(dot(a, a));
}
