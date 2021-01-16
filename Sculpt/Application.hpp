#pragma once

// Standard
#include <set>

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"
#include "Renderer.hpp"


/* TODO:
- Surface Detail Rendering
- primitive highlighting
- compute shader mesh deform
- import GLTF meshes
- save mesh to file and load from file

// Dependent (maybe)
- Axis Aligned Bounding Boxes
- ray queries
- vert groups, edge groups, poly groups
- partial vertex buffer updates ???

- dynamic z_near and far
- frame camera to mesh
- handle large amount of objects changing by storing them inside many buffers
so that one object changing only rewrites a single buffer
*/


// Forward
struct MeshDrawcall;


enum class DisplayMode {
	SOLID,
	SOLID_WITH_WIREFRAME_FRONT,
	SOLID_WITH_WIREFRAME_NONE,
	WIREFRANE,
	WIREFRANE_PURE,
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
	float roughness = 0.3f;
	float metallic = 0.1f;
	float specular = 0.04f;
};

struct MeshWireframeColors {
	glm::vec3 front_color = { 0.0f, 1.0f, 0.0f };
	glm::vec4 back_color = { 0.0f, 0.33f, 0.0f, 1.0f };
	glm::vec3 tesselation_front_color = { 0.0f, 1.0f, 0.0f };
	glm::vec4 tesselation_back_color = { 0.0f, 0.33f, 0.0f, 1.0f };
	float tesselation_split_count = 4.0f;
	float tesselation_gap = 0.5f;
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

	std::string name;

	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;

	PhysicalBasedMaterial pbr_material;
	MeshWireframeColors wireframe_colors;
	
	uint32_t instance_id;
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
	
	DisplayMode rasterizer_mode;
	bool is_back_culled;
	bool show_aabbs;  // not implemented
};


/* Contains mesh instances or other layers, mesh instances and layers can be shared across multiple layers */
struct MeshLayer {
	MeshLayer* parent;
	std::set<MeshLayer*> children;

	std::string name;
	std::set<MeshInstance*> instances;
};


struct MeshInstanceSearchResult {
	MeshInstance* instance;
	uint32_t char_count;
};


struct CreateTriangleInfo {
	MeshTransform transform;
	float size = 1;
};

struct CreateQuadInfo {
	MeshTransform transform;
	float size = 1.f;
};

struct CreateCubeInfo {
	MeshTransform transform;
	float size = 1;
};

struct CreateCylinderInfo {
	MeshTransform transform;
	float diameter = 1.0f;
	float height = 1.0f;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;
	bool with_caps = true;
};

struct CreateUV_SphereInfo {
	MeshTransform transform;
	float diameter = 1.0f;

	uint32_t vertical_sides = 2;
	uint32_t horizontal_sides = 3;
};

struct GLTF_ImporterSettings {
	MeshLayer* dest_layer = nullptr;
	MeshDrawcall* dest_drawcall = nullptr;
};


namespace nui {
	class Window;
}

class Application {
public:
	MeshRenderer renderer;
	nui::Window* window;

	std::list<MeshInBuffers> meshes;

	uint32_t instance_id;  // TODO: unused
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
	MeshInBuffers* createMesh();

	// Instances
	MeshInstance* createInstance(MeshInBuffers* mesh, MeshLayer* dest_layer, MeshDrawcall* parent_drawcall);
	MeshInstance* copyInstance(MeshInstance* source, MeshLayer* dest_layer = nullptr);

	void assignInstanceToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer);
	void searchForInstances(std::string& search, std::vector<MeshInstanceSearchResult>& r_results);

	// Drawcalls
	MeshDrawcall* createDrawcall(std::string& name = std::string(""));
	void transferToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall);

	// Layers
	MeshLayer* createLayer(std::string& name, MeshLayer* parent_layer = nullptr);
	void parentLayer(MeshLayer* child_layer, MeshLayer* parent_layer);

	MeshInstance* createTriangleMesh(CreateTriangleInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createQuadMesh(CreateQuadInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createCubeMesh(CreateCubeInfo& infos, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createCylinder(CreateCylinderInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createUV_Sphere(CreateUV_SphereInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	ErrStack importMeshesFromGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings = GLTF_ImporterSettings(),
		std::vector<MeshInstance*>* r_instances = nullptr);

	void setCameraFocus(glm::vec3& new_focus);
	void arcballOrbitCamera(float deg_x, float deg_y);
	void pivotCameraAroundY(float deg_x, float deg_y);
	void panCamera(float amount_x, float amount_y);
	void dollyCamera(float amount);
	void setCameraPosition(float x, float y, float z);
	void setCameraRotation(float x, float y, float z);
};

extern Application application;