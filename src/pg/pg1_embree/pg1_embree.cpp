#include "stdafx.h"
#include "tutorials.h"

int main()
{
	printf( "PG1, (c)2011-2018 Tomas Fabian\n\n" );

	_MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
	_MM_SET_DENORMALS_ZERO_MODE( _MM_DENORMALS_ZERO_ON );

	//return tutorial_1();
	//return tutorial_2();

	//return allied_avenger("../../../data/6887_allied_avenger.obj");
	return allied_avenger_bigger("../../../data/6887_allied_avenger.obj");

	//return sphere("../../../data/geosphere.obj");

	//return box("../../../data/cornell_box2.obj");
}
