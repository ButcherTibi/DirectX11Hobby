#pragma once

// Standard
#include <set>

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


// Forward
struct MeshDrawcall;

enum class FillMode {
	SOLID,
	WIREFRAME,
	SOLID_WITH_WIREFRAME
};

enum class CullMode {
	BACK,
	NONE,
	FRONT
};

enum class RasterizerMode {
	SOLID,
	WIREFRANE,
	SOLID_WITH_WIREFRAME,
	SOLID_WITH_WIREFRAME_FRONT_NONE
};
// RasterizerState
// solid front
// solid back
// wireframe none
// solid and wireframe front
// solid and wireframe back

/* which subprimitive holds the surface data to respond to the light */
namespace ShadingNormal {
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
	glm::vec3 albedo_color = { 1.0f, 0.0f, 0.0f };
	float roughness = 0.0f;
	float metallic = 0.05f;
	float specular = 0.04f;
};

struct MeshWireframeColors {
	glm::vec3 front_color = { 0.0f, 1.0f, 0.0f };
	glm::vec4 back_color = { 0.0f, 1.0f, 0.0f, 0.25f };
};


/* Contains a mesh and where to look for it in the vertex and index buffer for it */
struct MeshInBuffers {
	scme::SculptMesh mesh;

	uint32_t mesh_vertex_start;
	uint32_t mesh_vertex_count;

	// AABBs
	uint32_t aabb_vertex_start;
	uint32_t aabb_vertex_count;
	uint32_t aabb_index_start;
	uint32_t aabb_index_count;
};


/* Contains the instance data of a mesh */
struct MeshInstance {
	MeshDrawcall* parent_drawcall;
	MeshInBuffers* parent_mesh;

	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;

	PhysicalBasedMaterial pbr_material;
	MeshWireframeColors wireframe_colors;
};


struct MeshInstanceSet {
	MeshInBuffers* mesh;

	uint32_t mesh_insta_start;
	uint32_t mesh_insta_count;

	std::vector<MeshInstance*> instances;
};


/* Contains mesh instances to be rendered in a specific way,
instances cannot be shared across drawcalls*/
struct MeshDrawcall {
	std::string name;

	std::vector<MeshInstanceSet> mesh_instance_sets;
	
	// Drawcall
	FillMode fill_mode;
	CullMode cull_mode;

	RasterizerMode rasterizer_mode;
	glm::vec4 wireframe_color;

	// PRB Shader, Sculpt Shader, Fresnel Shader

	// Post effects
	bool show_aabbs;
	// transparency
};


/* Contains mesh instances or other layers, mesh instances and layers can be shared across multiple layers */
struct MeshLayer {
	MeshLayer* parent;
	std::set<MeshLayer*> children;

	std::string name;
	std::set<MeshInstance*> instances;
};


struct CreateTriangleInfo {
	MeshTransform transform;
	float size = 1;

	uint32_t shading_normal = ShadingNormal::POLY;
	PhysicalBasedMaterial material;
};

struct CreateQuadInfo {
	MeshTransform transform;
	float size = 1.f;

	uint32_t shading_normal = ShadingNormal::POLY;
	PhysicalBasedMaterial material;
};

struct CreateCubeInfo {
	MeshTransform transform;
	float size = 1;

	uint32_t shading_normal = ShadingNormal::POLY;
	PhysicalBasedMaterial material;
};

struct CreateCylinderInfo {
	MeshTransform transform;
	float diameter = 1.0f;
	float height = 1.0f;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;
	bool with_caps = true;

	uint32_t shading_normal = ShadingNormal::POLY;
	PhysicalBasedMaterial material;
};

struct CreateUV_SphereInfo {
	MeshTransform transform;
	float diameter = 1.0f;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;
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

	std::list<MeshInBuffers> meshes;
	std::list<MeshInstance> instances;
	std::list<MeshDrawcall> drawcalls;
	MeshDrawcall* last_used_drawcall;

	// Layers
	std::list<MeshLayer> layers;
	MeshLayer* last_used_layer;

	// Shading
	uint32_t shading_normal = ShadingNormal::POLY;

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
	void assignToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer = nullptr);

	MeshDrawcall& createDrawcall(std::string& name = std::string(""));
	void transferToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall);

	MeshInstance& createTriangleMesh(CreateTriangleInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance& createQuadMesh(CreateQuadInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance& createCubeMesh(CreateCubeInfo& infos, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance& createCylinder(CreateCylinderInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance& createUV_Sphere(CreateUV_SphereInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
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