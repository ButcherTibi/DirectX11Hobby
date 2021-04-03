#pragma once

// Standard
#include <set>  // TODO: replace with unordered set

#include "SculptMesh.hpp"
#include "Renderer.hpp"


/* TODO:
- camera on point rotation
- updateaza UI

- frame camera to mesh
- selected drawcall set display mode
- Surface Detail shading mode
- primitive highlighting
- compute shader mesh deform
- save mesh to file and load from file
- vert groups, edge groups, poly groups
- partial vertex buffer updates ???
- dynamic z_near and far
- mesh LOD using the AABBs
- MeshInstanceAABB
- dynamic shader reloading
- separate UI updates for CPU and GPU for
  CPU:
  -> map previuos frame readback textures
  -> map buffers for change
  GPU:
  -> unmap textures that will be invalidated
  -> unmap buffer to lock them into place
*/


// Forward
struct MeshLayer;
struct MeshDrawcall;
struct MeshInstance;


enum class DisplayMode {
	SOLID,
	SOLID_WITH_WIREFRAME_FRONT,
	SOLID_WITH_WIREFRAME_NONE,
	WIREFRANE,
	WIREFRANE_PURE,
};

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

	// Octree
	uint32_t aabbs_vertex_start;
	uint32_t aabbs_vertex_count;

	std::set<MeshInstance*> child_instances;
};


/* Contains the instance data of a mesh */
struct MeshInstance {
	MeshLayer* parent_layer;
	MeshDrawcall* parent_drawcall;
	MeshInBuffers* parent_mesh;

	std::string name;
	uint32_t id;
	bool visible;
	// bool can_collide;

	// Instance Data
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
	
	DisplayMode display_mode;
	bool is_back_culled;
	bool _debug_show_octree;
};


/* Contains mesh instances and other layers */
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

	uint32_t rows = 2;
	uint32_t columns = 3;
	bool with_caps = true;
};

struct CreateUV_SphereInfo {
	MeshTransform transform;
	float diameter = 1.0f;

	uint32_t rows = 2;
	uint32_t columns = 3;
};

struct GLTF_ImporterSettings {
	MeshLayer* dest_layer = nullptr;
	MeshDrawcall* dest_drawcall = nullptr;
};

struct CreateLineInfo {
	glm::vec3 origin = { 0.f, 0.f, 0.f};
	glm::vec3 direction = { 0.f, 0.f, -1.0f };
	float length = 1.0f;
};


namespace nui {
	class Window;
}

class Application {
public:
	// User Interface
	nui::Instance ui_instance;
	nui::Window* main_window;

	// Renderer
	MeshRenderer renderer;

	// Meshes
	std::list<MeshInBuffers> meshes;

	// Instances
	uint32_t instance_id;
	std::list<MeshInstance> instances;

	// Drawcalls
	std::list<MeshDrawcall> drawcalls;
	MeshDrawcall* last_used_drawcall;

	// Layers
	std::list<MeshLayer> layers;
	MeshLayer* last_used_layer;

	// Shading
	uint32_t shading_normal;

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
	D3D11_MAPPED_SUBRESOURCE _instance_id_mask;

	// returns true if mesh is without instances
	void _deleteFromDrawcall(MeshInstance* inst);

public:
	MeshInBuffers* createMesh();

	// Instances
	MeshInstance* createInstance(MeshInBuffers* mesh, MeshLayer* dest_layer, MeshDrawcall* parent_drawcall);
	MeshInstance* copyInstance(MeshInstance* source);
	void deleteInstance(MeshInstance* inst);

	void transferInstanceToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer);

	void rotateInstanceAroundY(MeshInstance* mesh_instance, float radians);

	void searchForInstances(std::string& keyword, std::vector<MeshInstanceSearchResult>& r_results);

	// Drawcalls
	MeshDrawcall* createDrawcall(std::string& name = std::string(""));
	void transferInstanceToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall);

	// Layers
	MeshLayer* createLayer(std::string& name, MeshLayer* parent_layer = nullptr);
	void transferLayer(MeshLayer* child_layer, MeshLayer* parent_layer);

	void setLayerVisibility(MeshLayer* layer, bool visible_state);

	// Create Objects
	MeshInstance* createTriangle(CreateTriangleInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createQuad(CreateQuadInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createCube(CreateCubeInfo& infos, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createCylinder(CreateCylinderInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createUV_Sphere(CreateUV_SphereInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	ErrStack importMeshesFromGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings,
		std::vector<MeshInstance*>* r_instances = nullptr);
	MeshInstance* createLine(CreateLineInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	
	// What mesh instance is under pixel
	void lookupInstanceMask(uint32_t pixel_x, uint32_t pixel_y, uint32_t& r_instance_id);

	// Raycasts

	/* performs a raycast in the mesh instance regardless of properties */
	bool raycast(MeshInstance* mesh_instance, glm::vec3& ray_origin, glm::vec3& ray_direction,
		uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_point);

	MeshInstance* mouseRaycastInstances(uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_point);

	// Camera
	void setCameraFocus(glm::vec3& new_focus);
	void arcballOrbitCamera(float deg_x, float deg_y);
	void pivotCameraAroundY(float deg_x, float deg_y);
	void panCamera(float amount_x, float amount_y);
	void dollyCamera(float amount);
	void setCameraPosition(float x, float y, float z);
	void setCameraRotation(float x, float y, float z);
};

extern Application application;