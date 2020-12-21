#pragma once

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"
#include "Renderer.hpp"


/* which subprimitive holds the surface data to respond to the light */
namespace MeshShadingSubPrimitive {
	enum {
		VERTEX,
		POLY,
		TESSELATION
	};
}

//namespace MeshHighlightMode {
//	enum : uint8_t {
//		VERTEX = 1,
//		NORMAL = 1 << 1,
//		EDGE   = 1 << 2,
//		POLY   = 1 << 3,
//		TESS   = 1 << 4
//	};
//}


/* light that is relative to the camera orientation */
struct CameraLight {
	glm::vec3 normal;

	glm::vec3 color;
	float intensity;
};


struct MeshTransform {
	glm::vec3 pos = { 0, 0, 0 };
	glm::quat rot = { 1, 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };
};

struct PhysicalBasedMaterial {
	glm::vec3 albedo_color = { 1, 1, 1 };
	float roughness = 0;
	float metallic = 0;
	float specular = 0;
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
	glm::vec3 pos = { 0, 0, 0};
	glm::quat rot = { 1, 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };
	float diameter = 1;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;

	uint32_t mesh_shading_subprimitive = MeshShadingSubPrimitive::POLY;
	glm::vec3 albedo_color = { 1, 1, 1 };
	float roughness = 0;
	float metallic = 0;
	float specular = 0.04f;
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