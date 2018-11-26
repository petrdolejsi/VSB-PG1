#include "stdafx.h"
#include "triangle.h"

Triangle::Triangle( const Vertex & v0, const Vertex & v1, const Vertex & v2, Surface * surface )
{
	vertices_[0] = v0;
	vertices_[1] = v1;
	vertices_[2] = v2;	

	// ukazatel na surface schov�me (!pokud se tam vejde!) do paddingu prvn�ho vertexu
	*reinterpret_cast<Surface **>( &vertices_[0].pad ) = surface;	
}

Vertex Triangle::vertex( const int i )
{
	return vertices_[i];
}

Surface * Triangle::surface()
{	
	return *reinterpret_cast<Surface **>( vertices_[0].pad ); // FIX: chyb� verze pro 64bit
}
