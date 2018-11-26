#include "stdafx.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"
#include "sphericalmap.h"
#include "utils.h"
#include <limits>
#include <cstddef>

#define PI 3.14159265359

Raytracer::Raytracer(const int width, const int height,
	const float fov_y, const Vector3 view_from, const Vector3 view_at,
	const char * config) : SimpleGuiDX11(width, height)
{
	InitDeviceAndScene(config);

	//map = new SphericalMap("../../../data/map.jpg");
	cubeMap_ = new CubeMap("../../../data/map");

	camera_ = Camera(width, height, fov_y, view_from, view_at);
}

Raytracer::~Raytracer()
{
	ReleaseDeviceAndScene();

	delete cubeMap_;
}

int Raytracer::InitDeviceAndScene(const char * config)
{
	device_ = rtcNewDevice(config);
	error_handler(nullptr, rtcGetDeviceError(device_), "Unable to create a new device.\n");
	rtcSetDeviceErrorFunction(device_, error_handler, nullptr);

	ssize_t triangle_supported = rtcGetDeviceProperty(device_, RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED);

	// create a new scene bound to the specified device
	scene_ = rtcNewScene(device_);

	return S_OK;
}

int Raytracer::ReleaseDeviceAndScene()
{
	rtcReleaseScene(scene_);
	rtcReleaseDevice(device_);

	return S_OK;
}

void Raytracer::LoadScene(const std::string file_name)
{
	const int no_surfaces = LoadOBJ(file_name.c_str(), surfaces_, materials_);

	// surfaces loop
	for (auto surface : surfaces_)
	{
		RTCGeometry mesh = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_TRIANGLE);

		Vertex3f * vertices = (Vertex3f *)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
			sizeof(Vertex3f), 3 * surface->no_triangles());

		Triangle3ui * triangles = (Triangle3ui *)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
			sizeof(Triangle3ui), surface->no_triangles());

		rtcSetGeometryUserData(mesh, (void*)(surface->get_material()));

		rtcSetGeometryVertexAttributeCount(mesh, 2);

		Normal3f * normals = (Normal3f *)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3,
			sizeof(Normal3f), 3 * surface->no_triangles());

		Coord2f * tex_coords = (Coord2f *)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT2,
			sizeof(Coord2f), 3 * surface->no_triangles());

		// triangles loop
		for (int i = 0, k = 0; i < surface->no_triangles(); ++i)
		{
			Triangle & triangle = surface->get_triangle(i);

			// vertices loop
			for (int j = 0; j < 3; ++j, ++k)
			{
				const Vertex & vertex = triangle.vertex(j);

				vertices[k].x = vertex.position.x;
				vertices[k].y = vertex.position.y;
				vertices[k].z = vertex.position.z;

				normals[k].x = vertex.normal.x;
				normals[k].y = vertex.normal.y;
				normals[k].z = vertex.normal.z;

				tex_coords[k].u = vertex.texture_coords[0].u;
				tex_coords[k].v = vertex.texture_coords[0].v;
			} // end of vertices loop

			triangles[i].v0 = k - 3;
			triangles[i].v1 = k - 2;
			triangles[i].v2 = k - 1;
		} // end of triangles loop

		rtcCommitGeometry(mesh);
		unsigned int geom_id = rtcAttachGeometry(scene_, mesh);
		rtcReleaseGeometry(mesh);
	} // end of surfaces loop

	rtcCommitScene(scene_);
}

Color4f Raytracer::get_pixel(const int x, const int y, const float t)
{
	//return Normal_shader(x, y, t);

	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.primID = RTC_INVALID_GEOMETRY_ID;
	hit.Ng_x = 0.0f; // geometry normal
	hit.Ng_y = 0.0f;
	hit.Ng_z = 0.0f;

	// merge ray and hit structures
	RTCRayHit ray_hit;
	ray_hit.ray = camera_.GenerateRay(x + 0.5f, y + 0.5f);
	ray_hit.hit = hit;

	Color4f resultColor = { 0 ,0 ,0, 1 };

	resultColor = resultColor + shader(ray_hit, t, IOR_AIR, 4);

	_antiAliasingConst = _antiAliasingConst1 + _antiAliasingConst10 * 10;
	
	for (int i = 0; i < _antiAliasingConst - 1; i++)
	{
		float randomX = Random();
		float randomY = Random();
		ray_hit.ray = camera_.GenerateRay(x + randomX, y + randomY);
		ray_hit.hit = hit;
		resultColor = resultColor + shader(ray_hit, t, IOR_AIR, 4);
	}

	return resultColor = { linearToSrgb((float)resultColor.r / _antiAliasingConst),
		linearToSrgb((float)resultColor.g / _antiAliasingConst),
		linearToSrgb((float)resultColor.b / _antiAliasingConst), 1 };

}

void Raytracer::reorinet(Normal3f &N, const int v_x, const int v_y, const int v_z)
{

	if ((N.x*v_x + N.x*v_y + N.x*v_z) < 0.0f)
	{
		N.x = N.x * -1.0f;
		N.y = N.y * -1.0f;
		N.z = N.z * -1.0f;
	}
}


inline Vector3 reflect(const Vector3 & v, const Vector3 & n)
{
	return (2.0f*(v.DotProduct(n))) * n - v;
}

Color4f Raytracer::Normal_shader(const int x, const int y, const float t)
{

	// TODO generate primary ray and perform ray cast on the scene

	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.primID = RTC_INVALID_GEOMETRY_ID;
	hit.Ng_x = 0.0f; // geometry normal
	hit.Ng_y = 0.0f;
	hit.Ng_z = 0.0f;

	// merge ray and hit structures
	RTCRayHit ray_hit;
	ray_hit.ray = camera_.GenerateRay(x + 0.5f, y + 0.5f);
	ray_hit.hit = hit;

	// intersect ray with the scene
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene_, &context, &ray_hit);

	if (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		// we hit something
		RTCGeometry geometry = rtcGetGeometry(scene_, ray_hit.hit.geomID);
		Normal3f normal;
		// get interpolated normal
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v,
			RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);
		// and texture coordinates
		Coord2f tex_coord;
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v,
			RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &tex_coord.u, 2);

		reorinet(normal, 1, 1, 1);

		Material * material = (Material *)(rtcGetGeometryUserData(geometry));
		return Color4f{ material->diffuse.x, material->diffuse.y, material->diffuse.z, 1.0f };
	}

	return Color4f{ 0.0f, 0.0f, 0.0f, 1.0f };

}

Color4f Raytracer::shader(RTCRayHit &ray_hit, const float t, float ior, int depth)
{
	// intersect ray with the scene
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene_, &context, &ray_hit);

	Color4f resultColor = Color4f{ 0, 0, 0, 1 };

	if (ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		// we hit something
		RTCGeometry geometry = rtcGetGeometry(scene_, ray_hit.hit.geomID);
		Normal3f normal;
		// get interpolated normal
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v,
			RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);

		reorient_against(normal, ray_hit.ray.dir_x, ray_hit.ray.dir_y, ray_hit.ray.dir_z);

		// and texture coordinates
		Coord2f tex_coord;
		rtcInterpolate0(geometry, ray_hit.hit.primID, ray_hit.hit.u, ray_hit.hit.v,
			RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &tex_coord.u, 2);

		Material * material = (Material *)(rtcGetGeometryUserData(geometry));

		const Triangle & triangle = surfaces_[ray_hit.hit.geomID]->get_triangle(ray_hit.hit.primID);

		Vector3 n = Vector3(normal.x, normal.y, normal.z);
		Vector3 v = Vector3(-ray_hit.ray.dir_x, -ray_hit.ray.dir_y, -ray_hit.ray.dir_z);

		if (n.DotProduct(v) < 0) {
			n = -n;
		}

		Vector3 p = Vector3(ray_hit.ray.org_x + ray_hit.ray.tfar * ray_hit.ray.dir_x,
			ray_hit.ray.org_y + ray_hit.ray.tfar * ray_hit.ray.dir_y,
			ray_hit.ray.org_z + ray_hit.ray.tfar * ray_hit.ray.dir_z);

		if (depth > 0) {
			if (material->shader == Material::Shader::GLASS && transparent_) {

				Vector3 d = -v;

				float n1;
				float n2;

				n1 = ior;

				if (n1 == IOR_AIR)
				{
					n2 = IOR_GLASS;
				}
				else
				{
					n2 = IOR_AIR;
				}

				float cos_01 = (n).DotProduct(v);
				if (cos_01 < 0) {
					cos_01 = (-n).DotProduct(v);
				}

				float n_d = n1 / n2;
				float sqrt_value = 1 - powf(n_d, 2) * (1 - powf(cos_01, 2));

				if (!(acosf(cos_01) > 0 && sqrt_value > 0)) {
					return shader(ray_hit, depth - 1, n2, t);
				}

				float cos_02 = sqrt(sqrt_value);

				Vector3 l = n_d * d + (n_d * cos_01 - cos_02) * n;

				Vector3 r = (2 * (n.DotProduct(v))) * n - v;

				float Rs = powf((n2 * cos_02 - n1 * cos_01) / (n2 * cos_02 + n1 * cos_01), 2);
				float Rp = powf((n2 * cos_01 - n1 * cos_02) / (n2 * cos_01 + n1 * cos_02), 2);
				float ReflectedCoef = 0.5f * (Rs + Rp);

				float RefractedCoef = 1.0f - ReflectedCoef;

				RTCRayHit reflectedRay;
				reflectedRay.ray.org_x = p.x;
				reflectedRay.ray.org_y = p.y;
				reflectedRay.ray.org_z = p.z;

				reflectedRay.ray.dir_x = r.x;
				reflectedRay.ray.dir_y = r.y;
				reflectedRay.ray.dir_z = r.z;

				reflectedRay.ray.tnear = 0.01f;
				reflectedRay.ray.time = 0.0f;

				reflectedRay.ray.mask = 0xFFFFFFFF;
				reflectedRay.ray.id = 0;
				reflectedRay.ray.flags = 0;

				RTCHit hit1;
				hit1.geomID = RTC_INVALID_GEOMETRY_ID;
				hit1.primID = RTC_INVALID_GEOMETRY_ID;
				hit1.Ng_x = 0.0f;
				hit1.Ng_y = 0.0f;
				hit1.Ng_z = 0.0f;

				reflectedRay.hit = hit1;

				RTCRayHit refractedRay;
				refractedRay.ray.org_x = p.x;
				refractedRay.ray.org_y = p.y;
				refractedRay.ray.org_z = p.z;

				refractedRay.ray.dir_x = l.x;
				refractedRay.ray.dir_y = l.y;
				refractedRay.ray.dir_z = l.z;

				refractedRay.ray.tnear = 0.01f;
				refractedRay.ray.tfar = FLT_MAX;
				refractedRay.ray.time = 0.0f;

				refractedRay.ray.mask = 0xFFFFFFFF;
				refractedRay.ray.id = 0;
				refractedRay.ray.flags = 0;

				RTCHit hit2;
				hit2.geomID = RTC_INVALID_GEOMETRY_ID;
				hit2.primID = RTC_INVALID_GEOMETRY_ID;
				hit2.Ng_x = 0.0f; // geometry normal
				hit2.Ng_y = 0.0f;
				hit2.Ng_z = 0.0f;

				refractedRay.hit = hit2;

				Color4f colorReflected = shader(reflectedRay, t, n1, depth - 1);

				Color4f colorRefracted = shader(refractedRay, t, n2, depth - 1);

				Color4f attenuation = { 1,1,1,1 };

				if (n1 == IOR_AIR) {
					Vector3 vectorToIntersection = (p - Vector3{ ray_hit.ray.org_x, ray_hit.ray.org_y, ray_hit.ray.org_z });
					float dstToIntersection = vectorToIntersection.L2Norm();
					attenuation = Color4f{ exp(-0.0001f*dstToIntersection) * material->getDiffuse(tex_coord).x, exp(-0.0001f*dstToIntersection) * material->getDiffuse(tex_coord).y, exp(-0.0001f*dstToIntersection) * material->getDiffuse(tex_coord).z };
				}

				colorRefracted.r = RefractedCoef * colorRefracted.r * attenuation.r + ReflectedCoef * colorReflected.r;
				colorRefracted.g = RefractedCoef * colorRefracted.g * attenuation.g + ReflectedCoef * colorReflected.g;
				colorRefracted.b = RefractedCoef * colorRefracted.b * attenuation.b + ReflectedCoef * colorReflected.b;
				colorRefracted.a = RefractedCoef * colorRefracted.a * attenuation.a + ReflectedCoef * colorReflected.a;

				resultColor = colorRefracted;
			}
			else if (material->shader == Material::Shader::BLACK_PLASTIC)
			{
				Vector3 l_d = _light_pos - p;
				float dist = l_d.L2Norm();
				l_d.Normalize();

				float shadow = shadow_ray(p, l_d, dist, context);

				float normalLigthScalarProduct = n.DotProduct(v);
				Vector3 lr = 2 * (normalLigthScalarProduct)* n - (v);
				//lr.Normalize();

				RTCRayHit reflectedRay;
				reflectedRay.ray.org_x = p.x;
				reflectedRay.ray.org_y = p.y;
				reflectedRay.ray.org_z = p.z;

				reflectedRay.ray.dir_x = lr.x;
				reflectedRay.ray.dir_y = lr.y;
				reflectedRay.ray.dir_z = lr.z;

				reflectedRay.ray.tnear = 0.01f;
				reflectedRay.ray.tfar = FLT_MAX;
				reflectedRay.ray.time = 0.0f;

				reflectedRay.ray.mask = 0xFFFFFFFF;
				reflectedRay.ray.id = 0;
				reflectedRay.ray.flags = 0;

				RTCHit hit1;
				hit1.geomID = RTC_INVALID_GEOMETRY_ID;
				hit1.primID = RTC_INVALID_GEOMETRY_ID;
				hit1.Ng_x = 0.0f; // geometry normal
				hit1.Ng_y = 0.0f;
				hit1.Ng_z = 0.0f;

				reflectedRay.hit = hit1;

				resultColor = shader(reflectedRay, t, 1, 0) * shadow * Vector3 { material->getDiffuse(tex_coord).x, material->getDiffuse(tex_coord).y, material->getDiffuse(tex_coord).z };
			}
			else if (material->shader == Material::Shader::PATH)
			{
				if (material->emission.x != 0 && material->emission.y != 0 && material->emission.z != 0) {
					return Color4f{ material->emission.x, material->emission.y, material->emission.z, 1 };
				}

				Vector3 omegaI = sampleHemisphere(n);

				float inversePdf = 2 * PI;

				RTCRayHit reflectedRay;
				reflectedRay.ray.org_x = p.x;
				reflectedRay.ray.org_y = p.y;
				reflectedRay.ray.org_z = p.z;

				reflectedRay.ray.dir_x = omegaI.x;
				reflectedRay.ray.dir_y = omegaI.y;
				reflectedRay.ray.dir_z = omegaI.z;

				reflectedRay.ray.tnear = 0.01f;
				reflectedRay.ray.tfar = FLT_MAX;
				reflectedRay.ray.time = 0.0f;

				reflectedRay.ray.mask = 0xFFFFFFFF;
				reflectedRay.ray.id = 0;
				reflectedRay.ray.flags = 0;

				RTCHit hit1;
				hit1.geomID = RTC_INVALID_GEOMETRY_ID;
				hit1.primID = RTC_INVALID_GEOMETRY_ID;
				hit1.Ng_x = 0.0f; // geometry normal
				hit1.Ng_y = 0.0f;
				hit1.Ng_z = 0.0f;

				reflectedRay.hit = hit1;

				Color4f l_i = shader(reflectedRay, t, 1, 0);

				Color4f f_r = Color4f{ material->diffuse.x, material->diffuse.y, material->diffuse.z, 1 } * (1 / PI);
				resultColor =  l_i * f_r * omegaI.DotProduct(n) * inversePdf;
			}
			else
			{
				Vector3 l_d = _light_pos - p;

				float dist = l_d.L2Norm();
				l_d.Normalize();

				Vector3 l_r = reflect(l_d, n);

				float shadow = shadow_ray(p, l_d, dist, context);

				Vector3 ambient = material->ambient * _light_color;
				Vector3 diffuse = shadow * material->getDiffuse(tex_coord) * (max(0.0f, n.DotProduct(l_d)) * _light_color);
				Vector3 specular = shadow * material->getSpecular(tex_coord) * powf(max(0.0f, l_r.DotProduct(v)), 1);
				Vector3 color = (ambient + (specular + diffuse));

				resultColor = Color4f{ color.x, color.y, color.z, _light_intensity };
			}
		}
		else
		{
			Vector3 l_d = _light_pos - p;

			float dist = l_d.L2Norm();
			l_d.Normalize();

			Vector3 l_r = reflect(l_d, n);

			float shadow = shadow_ray(p, l_d, dist, context);

			Vector3 ambient = material->ambient * _light_color;
			Vector3 diffuse = shadow * material->getDiffuse(tex_coord) * (max(0.0f, n.DotProduct(l_d)) * _light_color);
			Vector3 specular = shadow * material->getSpecular(tex_coord) * powf(max(0.0f, l_r.DotProduct(v)), 1);
			Vector3 color = (ambient + (specular + diffuse));

			resultColor = Color4f{ color.x, color.y, color.z, _light_intensity };
		}
	}
	else
	{
		if (background)
		{
			Vector3 v = Vector3(-ray_hit.ray.dir_x, -ray_hit.ray.dir_y, -ray_hit.ray.dir_z);
			Color3f mapColor = cubeMap_->get_texel(v);
			resultColor = Color4f{ mapColor.r, mapColor.g, mapColor.b, _light_intensity };
		}
		else
		{
			resultColor = Color4f{ 0.0f, 0.0f, 0.0f, 1.0f };
		}
	}

	return resultColor;
}

float Raytracer::shadow_ray(Vector3 & p, Vector3 & l_d, const float dist, RTCIntersectContext context)
{
	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.primID = RTC_INVALID_GEOMETRY_ID;

	RTCRay ray = RTCRay();
	ray.org_x = p.x; // ray origin
	ray.org_y = p.y;
	ray.org_z = p.z;

	ray.dir_x = l_d.x;
	ray.dir_y = l_d.y;
	ray.dir_z = l_d.z;

	ray.tnear = 0.1f;
	ray.tfar = dist;

	ray.time = 0.0f;

	ray.mask = 0; // can be used to mask out some geometries for some rays
	ray.id = 0; // identify a ray inside a callback function
	ray.flags = 0; // reserved

	rtcInitIntersectContext(&context);
	rtcOccluded1(scene_, &context, &ray);

	if (ray.tfar < dist) {
		return 0.4f;
	}
	else {
		return 1.0f;
	}
}

Vector3 Raytracer::sampleHemisphere(Vector3 normal) {
	float random1 = Random();
	float random2 = Random();

	float x = 2 * cosf(2 * PI * random1) * sqrt(random2 * (1 - random2));
	float y = 2 * sinf(2 * PI * random1) * sqrt(random2 * (1 - random2));
	float z = 1 - 2 * random2;

	Vector3 omegaI = Vector3{ x, y, z };

	if (omegaI.DotProduct(normal) < 0) {
		omegaI *= -1;
	}
	return omegaI;
}

int Raytracer::Ui()
{
	static float f = 0.0f;
	//static int counter = 0;

	// we use a Begin/End pair to created a named window
	ImGui::Begin("Ray Tracer Params");

	ImGui::Text("Surfaces = %d", surfaces_.size());
	ImGui::Text("Materials = %d", materials_.size());
	ImGui::Separator();
	ImGui::Checkbox("Vsync", &vsync_);
	ImGui::Checkbox("Transparent", &transparent_);

	//ImGui::Checkbox( "Demo Window", &show_demo_window );      // Edit bools storing our window open/close state
	//ImGui::Checkbox( "Another Window", &show_another_window );

	//ImGui::SliderFloat( "float", &f, 0.0f, 1.0f );            // Edit 1 float using a slider from 0.0f to 1.0f  
	if (ImGui::TreeNode("Light"))
	{
		ImGui::SliderFloat("Light x", &_light_pos.x, -500, 500);
		ImGui::SliderFloat("Light y", &_light_pos.y, -500, 500);
		ImGui::SliderFloat("Light z", &_light_pos.z, -500, 500);
		ImGui::SliderFloat("Light intensity", &_light_intensity, 0.0, 1.0);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Sampling"))
	{
		ImGui::SliderInt("Sampling x1", &_antiAliasingConst1, 1, 10);
		ImGui::SliderInt("Sampling x10", &_antiAliasingConst10, 0, 100);
		if (ImGui::Button("Sampling -")) 
		{
			if (_antiAliasingConst != 1)
			{
				if (_antiAliasingConst1 == 0) {
					_antiAliasingConst1 = 9;
					_antiAliasingConst10--;
				}
				else
				{
					_antiAliasingConst1--;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Sampling +"))
		{
			if (_antiAliasingConst != 1009)
			{
				if (_antiAliasingConst1 == 9) {
					_antiAliasingConst1 = 0;
					_antiAliasingConst10++;
				}
				else
				{
					_antiAliasingConst1++;
				}
			}
		}
		ImGui::SameLine();
		ImGui::Text("Super sampling = %d", _antiAliasingConst);
		ImGui::TreePop();
	}

	char buf[32];
	sprintf(buf, "%d/%d", (int)number_, (height_ * width_));
	ImGui::ProgressBar((float)number_ / (height_ * width_), ImVec2(-1, 0), buf);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	// 3. Show another simple window.
	/*if ( show_another_window )
	{
	ImGui::Begin( "Another Window", &show_another_window );   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text( "Hello from another window!" );
	if ( ImGui::Button( "Close Me" ) )
	show_another_window = false;
	ImGui::End();
	}*/

	return 0;
}
