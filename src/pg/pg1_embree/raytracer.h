#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"
#include "sphericalmap.h"
#include "CubeMap.h"

/*! \class Raytracer
\brief General ray tracer class.

\author Tomáš Fabián
\version 0.1
\date 2018
*/
class Raytracer : public SimpleGuiDX11
{
public:
	Raytracer( const int width, const int height, 
		const float fov_y, const Vector3 view_from, const Vector3 view_at,
		const char * config = "threads=0,verbose=3" );
	~Raytracer();

	int InitDeviceAndScene( const char * config );

	int ReleaseDeviceAndScene();

	void LoadScene( const std::string file_name );

	void reorinet(Normal3f & N, const int v_x, const int v_y, const int v_z);

	Color4f get_pixel( const int x, const int y, const float t = 0.0f ) override;

	Color4f Normal_shader(const int x, const int y, const float t);

	Color4f shader(RTCRayHit &ray_hit, const float t, float ior, int depth);

	float shadow_ray(Vector3 & p, Vector3 & l_d, float dist, RTCIntersectContext context);

	Vector3 sampleHemisphere(Vector3 normal);

	int Ui();

	bool transparent_ = true;

	Vector3 _light_pos = Vector3(200, -200, 300);
	Vector3 _light_color = Vector3(1, 1, 1);

	int _antiAliasingConst1 = 1;
	int _antiAliasingConst10 = 0;
	float _light_intensity = 1;
	int _antiAliasingConst = 1;

	bool background = true;

private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	CubeMap *cubeMap_;
	//SphericalMap *map;

	RTCDevice device_;
	RTCScene scene_;
	Camera camera_;
};
