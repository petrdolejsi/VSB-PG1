#pragma once
#include "stdafx.h"
#include "texture.h"
#include "vector3.h"

class CubeMap {
public:
	enum {
		POS_X = 0,
		NEG_X = 1,
		POS_Y = 2,
		POS_Z = 3,
		NEG_Z = 4,
		NEG_Y = 5
	} position;

	CubeMap(const char* basePath);
	~CubeMap();

	bool showed[6];
	Texture* maps_[6];
	Color3f get_texel(Vector3 &direction);
private:
	int getMapIndex(Vector3 direction);
};