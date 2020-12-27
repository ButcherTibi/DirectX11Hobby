#pragma once

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"
#include "Renderer.hpp"


/* TODO:
- Surface Detail Rendering
- wireframe with color
- primitive highlighting
- compute shader mesh deform
- import GLTF meshes
- save mesh to file and load from file
- calculate vertex normals on demand

// Dependent (maybe)
- Axis Aligned Bounding Boxes
- ray queries
- vert groups, edge groups, poly groups
- partial vertex buffer updates ???

- frame mesh
- handle large amount of objects changing by storing them inside many buffers
so that one object changing only rewrites a single buffers
*/


/* which subprimitive holds the surface data to respond to the light */
namespace MeshShadingSubPrimitive {
	enum {
		VERTEX,
		POLY,
		TESSELATION
	};
}

/* light that is relative to the camera orientation */
struct CameraLight {
	glm::vec3 normal;

	glm::vec3 color;
	float intensity;
};


struct MeshTransform {
	glm::vec3 pos = { .0f, .0f, .0f };
	glm::quat rot = { 1.0f, .0f, .0f, .0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
};

struct PhysicalBasedMaterial {
	glm::vec3 albedo_color = { 1.0f, 1.0f, 1.0f };
	float roughness = 0.0f;
	float metallic = 0.0f;
	float specular = 0.04f;
};


class MeshInstance {
public:
	scme::SculptMesh* mesh;
	
	/* GPU Instance */
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	
	uint32_t mesh_shading_subprimitive;

	// Material
	glm::vec3 albedo_color;
	float roughness;
	float metallic;
	float specular;

	// GPU Draw Call
	// hide mesh
};


struct MeshLayer {
	std::list<MeshLayer*> children;

	std::string name;
	std::list<MeshInstance*> instances;
};


struct CreateTriangleInfo {
	MeshTransform transform;
	float size = 1;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	PhysicalBasedMaterial material;
};

struct CreateQuadInfo {
	MeshTransform transform;
	float size = 1.f;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	PhysicalBasedMaterial material;
};

struct CreateCubeInfo {
	MeshTransform transform;
	float size = 1;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	PhysicalBasedMaterial material;
};

struct CreateCylinderInfo {
	MeshTransform transform;
	float diameter = 1;
	float height = 1;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;
	bool with_caps = true;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	PhysicalBasedMaterial material;
};

struct CreateUV_SphereInfo {
	MeshTransform transform;
	float diameter = 1;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	PhysicalBasedMaterial material;
};

struct GLTF_ImporterSettings {

};


namespace nui {
	class Window;
}

class Application {
public:
	MeshRenderer renderer;
	nui::Window* window;

	std::list<scme::SculptMesh> meshes;
	std::list<MeshInstance> instances;

	std::list<MeshLayer> layers;

	// Lighting
	std::array<CameraLight, 8> lights;
	float ambient_intensity;

	// Camera
	glm::vec3 camera_focus;
	float camera_field_of_view;
	float camera_z_near;
	float camera_z_far;
	glm::vec3 camera_pos;
	glm::quat camera_quat_inv;
	glm::vec3 camera_forward;

	float camera_orbit_sensitivity;
	float camera_pan_sensitivity;
	float camera_dolly_sensitivity;

public:
	MeshInstance& createTriangleMesh(CreateTriangleInfo& info);
	MeshInstance& createQuadMesh(CreateQuadInfo& info);
	MeshInstance& createCubeMesh(CreateCubeInfo& infos);
	MeshInstance& createCylinder(CreateCylinderInfo& info);
	MeshInstance& createUV_Sphere(CreateUV_SphereInfo& info);
	ErrStack importGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings);

	void setCameraFocus(glm::vec3& new_focus);
	void arcballOrbitCamera(float deg_x, float deg_y);
	void pivotCameraAroundY(float deg_x, float deg_y);
	void panCamera(float amount_x, float amount_y);
	void dollyCamera(float amount);
	void setCameraPosition(float x, float y, float z);
	void setCameraRotation(float x, float y, float z);
};

extern Application application;