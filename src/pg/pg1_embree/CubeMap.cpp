#include "stdafx.h"
#include "CubeMap.h"

CubeMap::CubeMap(const char* basePath) {
	char buffer[100];
	const char* fileNames[6] = {
		"/posx.jpg",
		"/negx.jpg",
		"/posy.jpg",
		"/negy.jpg",
		"/posz.jpg",
		"/negz.jpg",
	};

	for (int i = 0; i < 6; i++) {
		strcpy(buffer, basePath);
		strcat(buffer, fileNames[i]);
		maps_[i] = new Texture(buffer);
		showed[i] = false;
	}
}

CubeMap::~CubeMap() {
	for (int i = 0; i < 6; i++) {
		delete maps_[i];
		maps_[i] = NULL;
	}
}

Color3f CubeMap::get_texel(Vector3 &direction) {
	float tmp, u, v;
	int map = getMapIndex(direction);

	switch (map)
	{
		case NEG_X:
			tmp = (1.0f / abs(direction.x));
			u = (direction.y * tmp + 1) * 0.5f;
			v = (direction.z * tmp + 1) * 0.5f;
			break;

		case NEG_Y:
			tmp = (1.0f / abs(direction.y));
			u = 1 - (direction.x * tmp + 1) * 0.5f;
			v = (direction.z * tmp + 1) * 0.5f;
			break;

		case NEG_Z:
			tmp = (1.0f / abs(direction.z));
			u = (direction.x * tmp + 1) * 0.5f;
			v = (direction.y * tmp + 1) * 0.5f;
			break;

		case POS_X:
			tmp = 1.0f / abs(direction.x);
			u = 1 - (direction.y * tmp + 1) * 0.5f;
			v = (direction.z * tmp + 1) * 0.5f;
			break;

		case POS_Y:
			tmp = 1.0f / abs(direction.y);
			u = (direction.x * tmp + 1) * 0.5f;
			v = (direction.z * tmp + 1) * 0.5f;
			break;

		case POS_Z:
			tmp = 1.0f / abs(direction.z);
			u = (direction.x * tmp + 1) * 0.5f;
			v = 1 - (direction.y * tmp + 1) * 0.5f;
			break;
		default:
			printf("ERROR: unknown direction %d\n", map);
			break;
	}

	Color3f texel = maps_[map]->get_texel_bilinear(u, v);
	//Color3f texel = maps_[map]->get_texel(u, v);
	return texel;
}

int CubeMap::getMapIndex(Vector3 direction) {
	int lgI = direction.LargestComponent(true);

	if (lgI == 0) {
		return direction.x < 0 ? NEG_X : POS_X;
	}
	else if (lgI == 1) {
		return direction.y < 0 ? NEG_Y : POS_Y;
	}
	else {
		return direction.z < 0 ? NEG_Z : POS_Z;
	}
}