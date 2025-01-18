#pragma once

#include "Vec2.h"

struct Vec3
{
	float x, y, z;

	Vec3();
	Vec3(float x, float y, float z);

	operator Vec2() const;
};

inline Vec3 operator+(const Vec3& a, const Vec3& b)
{
	return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
	return Vec3(a.x - b.x, a.y - b.y, a.z + b.z);
}

inline Vec3 operator*(const Vec3& a, const Vec3& b)
{
	return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline Vec3 operator*(const Vec3& a, float d)
{
	return Vec3(a.x * d, a.y * d, a.z * d);
}

inline Vec3 operator*(float d, const Vec3& a)
{
	return Vec3(d * a.x, d * a.y, d * a.z);
}

inline float dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float mag(const Vec3& a)
{
	return sqrt(dot(a, a));
}