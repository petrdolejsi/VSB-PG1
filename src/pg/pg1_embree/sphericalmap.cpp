#include "stdafx.h"
#include "sphericalmap.h"
#define _USE_MATH_DEFINES
#include <math.h>

SphericalMap::SphericalMap(const std::string & file_name) {
	this->texture_ = std::make_unique<Texture>(file_name.c_str());
}

Color3f SphericalMap::texel(const float x, const float y, const float z) {
	const float theta = acos(z);
	const float phi = atan2f(y, x) + float(M_PI);

	const float u = 1.0f - phi * 0.5f * float(M_1_PI);
	const float v = theta * float(M_1_PI);

	return texture_->get_texel(u, v);
}