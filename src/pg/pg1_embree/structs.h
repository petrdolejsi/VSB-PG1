#pragma once
#include "vector3.h"

struct Vertex3f { float x, y, z; }; // a single vertex position structure matching certain format

using Normal3f = Vertex3f; // a single vertex normal structure matching certain format

inline void reorient_against(Normal3f & n, const float v_x, const float v_y, const float v_z) {
	if ((n.x * v_x + n.y * v_y + n.z * v_z) > 0.0f) {
		n.x *= -1;
		n.y *= -1;
		n.z *= -1;
	}
}

struct Coord2f { float u, v; }; // texture coord structure

struct Triangle3ui { unsigned int v0, v1, v2; }; // indicies of a single triangle, the struct must match certain format, e.g. RTC_FORMAT_UINT3

struct Color4f
{
	struct { float r, g, b, a; }; // a = 1 means that the pixel is opaque

	Color4f operator+(const Color4f & v)
	{
		return Color4f{ this->r + v.r, this->g + v.g, this->b + v.b, this->a };
	}

	Color4f operator*(const float a) {
		return Color4f{ this->r * a, this->g * a, this->b * a, this->a };
	}

	Color4f operator*(const Color4f & v)
	{
		return Color4f{ this->r * v.r, this->g * v.g, this->b * v.b, this->a };
	}

	Color4f operator*(const Vector3 & v)
	{
		return Color4f{ this->r * v.x, this->g * v.y, this->b * v.z, this->a };
	}
};

struct Color3f
{
	struct { float r, g, b; }; 

	Color3f operator*(const float a)
	{
		return Color3f{ this->r * a, this->g * a, this->b * a };
	}

	Color3f operator/(const float a)
	{
		return Color3f{ this->r / a, this->g / a, this->b / a };
	}

	Color3f operator+(const Color3f & v)
	{
		return Color3f{ this->r + v.r, this->g + v.g, this->b + v.b };
	}
};
