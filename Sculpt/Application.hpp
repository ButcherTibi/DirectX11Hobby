#pragma once

#include "SculptMesh.hpp"


class Application {
public:
	scme::SculptMesh mesh;

	float field_of_view;
	float z_near;
	float z_far;
};

extern Application application;