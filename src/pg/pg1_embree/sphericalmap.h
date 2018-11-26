#pragma once
#include <string>
#include "texture.h"

class SphericalMap
{
public:
	SphericalMap(const std::string & file_name);
	Color3f texel(const float x, const float y, const float z);
	~SphericalMap();

private:
	std::unique_ptr<Texture> texture_;
};