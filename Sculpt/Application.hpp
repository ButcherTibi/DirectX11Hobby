#pragma once

// Standard
#include <unordered_set>
#include <list>

#include "SculptMesh.hpp"
#include "Renderer.hpp"


/* TODO:
- camera on point rotation

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
namespace nui {
	class Window;
}

struct MeshLayer;
struct MeshDrawcall;
struct MeshInstanceSet;
struct MeshInstance;
struct Mesh;


enum class DisplayMode {
	SOLID,
	SOLID_WITH_WIREFRAME_FRONT,
	SOLID_WITH_WIREFRAME_NONE,
	WIREFRANE,
	WIREFRAME_PURE
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

// how to display a set of instances of a mesh
struct MeshDrawcall {
	std::string name;

	DisplayMode display_mode;
	bool is_back_culled;
	bool _debug_show_octree;
};


enum class ModifiedInstanceType {
	UPDATE,
	DELETED
};

// What instance was modified and what was modified
struct ModifiedMeshInstance {
	uint32_t idx;  // 0xFFFF'FFFF to mark as deleted

	ModifiedInstanceType type;
};


// Size and position of a instance
struct MeshTransform {
	glm::vec3 pos = { .0f, .0f, .0f };
	glm::quat rot = { 1.0f, .0f, .0f, .0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
};

// The apperance of the material beloging to a mesh
struct PhysicalBasedMaterial {
	glm::vec3 albedo_color = { 1.0f, 0.0f, 0.0f };
	float roughness = 0.3f;
	float metallic = 0.1f;
	float specular = 0.04f;
};

// Coloring of a instance's wireframe
struct MeshWireframeColors {
	glm::vec3 front_color = { 0.0f, 1.0f, 0.0f };
	glm::vec4 back_color = { 0.0f, 0.20f, 0.0f, 1.0f };
	glm::vec3 tesselation_front_color = { 0.0f, 1.0f, 0.0f };
	glm::vec4 tesselation_back_color = { 0.0f, 0.20f, 0.0f, 1.0f };
	float tesselation_split_count = 4.0f;
	float tesselation_gap = 0.5f;
};


// A copy of a mesh with slightly different look
struct MeshInstance {
	MeshInstanceSet* instance_set;
	uint32_t index_in_buffer;
	MeshLayer* parent_layer;

	std::string name;  // user given name
	//uint32_t id;  
	bool visible;  // whether to render or not
	// bool can_collide;

	// Instance Data
	MeshTransform transform;

	PhysicalBasedMaterial pbr_material;
	MeshWireframeColors wireframe_colors;

public:
	// Scheduling methods
	// Each of the below methods will schedule the renderer to update data on the GPU
	// Avoid calling more than one method

	void markFullUpdate();
};


// Instances of mesh to rendered with a certain drawcall
struct MeshInstanceSet {
	Mesh* parent_mesh;

	MeshDrawcall* drawcall;  // with what settings to render this set of instances

	DeferredVector<MeshInstance> instances;  // the instances of the mesh to be rendered in one drawcall
	std::vector<ModifiedMeshInstance> modified_instances;
	dx11::ArrayBuffer<GPU_MeshInstance> gpu_instances;
};


// Vertices and indexes of geometry
struct Mesh {
	scme::SculptMesh mesh;

	std::list<MeshInstanceSet> sets;
};


struct MeshLayer {
	MeshLayer* parent;
	std::unordered_set<MeshLayer*> children;

	std::string name;
	std::unordered_set<MeshInstance*> instances;
};


// Mesh Creation Parameters

struct CreateTriangleInfo {
	MeshTransform transform;
	float size = 1.f;
};

struct CreateQuadInfo {
	MeshTransform transform;
	float size = 1.f;
};

struct CreateCubeInfo {
	MeshTransform transform;
	float size = 1.f;
};

struct CreateCylinderInfo {
	MeshTransform transform;
	float diameter = 1.f;
	float height = 1.f;

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


class Application {
public:
	// User Interface
	nui::Instance ui_instance;
	nui::Window* main_window;

	// Renderer
	MeshRenderer renderer;

	// Meshes
	std::list<Mesh> meshes;

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
	// TODO: what happens if the last used drawcall is deleted
	// void setLastUsedDrawcall();

public:

	// Instances

	MeshInstance* copyInstance(MeshInstance* source, MeshDrawcall* dest_drawcall = nullptr);

	//void deleteInstance(MeshInstance* inst);

	void transferInstanceToLayer(MeshInstance* mesh_instance, MeshLayer* dest_layer);

	// Copies Instance from one set to another set of the same Mesh that has a set with the
	// destination drawcall. If the set with the destination drawcall does not exist it is created
	// 
	// Returns pointer to instance allocated on a different instance set of the same mesh
	// Warning: passed in pointer is invalidated
	//MeshInstance* transferInstanceToDrawcall(MeshInstance* mesh_instance, MeshDrawcall* dest_drawcall);

	void rotateInstanceAroundY(MeshInstance* mesh_instance, float radians);


	// Drawcalls

	// Creates a drawcall to be associated with instances
	MeshDrawcall* createDrawcall();

	// Deletes a drawcall and moves instances to the last used drawcall
	//void deleteDrawcall();


	// Layers

	// Creates a new orfan layer
	MeshLayer* createLayer();

	void transferLayer(MeshLayer* child_layer, MeshLayer* parent_layer);

	// Recursivelly sets all instances of a layer as invisible
	void setLayerVisibility(MeshLayer* layer, bool visible_state);


	// Create Objects
	MeshInstance* createEmptyMesh(MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createTriangle(CreateTriangleInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createQuad(CreateQuadInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createCube(CreateCubeInfo& infos, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	/*MeshInstance* createCylinder(CreateCylinderInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstance* createUV_Sphere(CreateUV_SphereInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	ErrStack importMeshesFromGLTF_File(io::FilePath& path, GLTF_ImporterSettings& settings,
		std::vector<MeshInstance*>* r_instances = nullptr);
	MeshInstance* createLine(CreateLineInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	

	// Raycasts

	// performs a raycast in the instance regardless of properties
	bool raycast(MeshInstance* mesh_instance, glm::vec3& ray_origin, glm::vec3& ray_direction,
		uint32_t& r_isect_poly, float& r_isect_distance, glm::vec3& r_isect_point);

	MeshInstance* mouseRaycastInstances(uint32_t& r_isect_poly, glm::vec3& r_isect_point);*/


	// Camera

	void setCameraFocus(glm::vec3& new_focus);

	void arcballOrbitCamera(float deg_x, float deg_y);

	void pivotCameraAroundY(float deg_x, float deg_y);

	void panCamera(float amount_x, float amount_y);

	void dollyCamera(float amount);

	void setCameraPosition(float x, float y, float z);

	void setCameraRotation(float x, float y, float z);


	// Scene

	void resetToHardcodedStartup();
};

extern Application application;
