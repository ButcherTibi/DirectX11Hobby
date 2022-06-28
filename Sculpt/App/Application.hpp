#pragma once

// Standard
#include <unordered_set>
#include <set>
#include <list>
#include <thread>

#include <SculptMesh/SculptMesh.hpp>
#include <ButchersToolbox/Filesys/Filesys.hpp>
#include "Camera/Camera.hpp"

#include "./CommonTypes.hpp"
#include "./Window/Window.hpp"
#include "./Input.hpp"


/* TODO:
- add brush circle
- delete vertex
- delete edge

- createUnorderedPoly, winding based on normal
- frame camera to mesh
- Surface Detail shading mode
- compute shader mesh deform
- save mesh to file and load from file
- vert groups, edge groups, poly groups
- mesh LOD using the AABBs
- MeshInstanceAABB
- dynamic shader reloading
*/


// Forward
struct MeshLayer;
struct MeshDrawcall;
struct MeshInstanceSet;
struct MeshInstance;
struct Mesh;


enum class DisplayMode {
	SOLID,
	WIREFRAME_OVERLAY,
	WIREFRANE
};

/* which subprimitive holds the surface data to respond to the light */
namespace GPU_ShadingNormal {
	enum {
		VERTEX,
		POLY,
		TESSELATION  // quads are split into 2 triangles and each has a normal
	};
}

//
//enum class SelectionMode {
//
//};


/* light that is relative to the camera orientation */
struct CameraLight {
	glm::vec3 normal;

	glm::vec3 color;
	float intensity;
};

enum class AABB_RenderMode {
	NO_RENDER,  // disable AABB rendering
	LEAF_ONLY,  // only render AABB that have vertices assigned to it
	NORMAL  // render leafs and AABB that have child AABBs
};

// how to display a set of instances of a mesh
struct MeshDrawcall {
	std::string name;
	std::string description;

	DisplayMode display_mode;
	bool is_back_culled;
	AABB_RenderMode aabb_render_mode;
};


enum class ModifiedInstanceType {
	UPDATE,
	DELETED
};

// What instance was modified and what was modified
struct ModifiedMeshInstance {
	uint32_t idx;

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

	std::string name;
	std::string description;
	bool visible;  // not really used
	// bool can_collide;

	// Instance Data
	MeshTransform transform;

	PhysicalBasedMaterial pbr_material;
	MeshWireframeColors wireframe_colors;

	bool instance_select_outline;

public:
	scme::SculptMesh& getSculptMesh();

	void localizePosition(glm::vec3& global_position);

	// Scheduling methods
	// Each of the below methods will schedule the renderer to load data on the GPU
	// Avoid calling more than one method

	// mark to have it's data updated on the GPU
	void markFullUpdate();
};


struct MeshInstanceRef {
	MeshInstanceSet* instance_set;
	uint32_t index_in_buffer;

public:
	bool operator==(MeshInstanceRef& other);

public:
	MeshInstance* get();
};


// Instances of mesh to rendered with a certain drawcall
struct MeshInstanceSet {
	Mesh* parent_mesh;

	MeshDrawcall* drawcall;  // with what settings to render this set of instances

	// the instances of the mesh to be rendered in one drawcall
	// adding or removing cause MeshInstance* to be invalid
	SparseVector<MeshInstance> instances;
	std::vector<ModifiedMeshInstance> modified_instances;
	dx11::ArrayBuffer<GPU_MeshInstance> gpu_instances;
	ComPtr<ID3D11ShaderResourceView> gpu_instances_srv;
};

// Vertices and indexes of geometry
struct Mesh {
	scme::SculptMesh mesh;

	std::list<MeshInstanceSet> sets;

	// should vertices for AABBs be generated for rendering and should they be rendered
	AABB_RenderMode aabb_render_mode;
	AABB_RenderMode prev_aabb_mode;  // to make AABB vertex buffer be recreated on AABB render mode switch
};


struct MeshLayer {
	MeshLayer* _parent;
	std::unordered_set<MeshLayer*> _children;

	std::string name;
	std::string description;
	std::list<MeshInstanceRef> instances;
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

struct CreateWavyGridInfo {
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


struct RaytraceInstancesResult {
	MeshInstanceRef inst_ref;
	uint32_t poly;
	glm::vec3 local_isect;  // local position of intersection same for instances of the same mesh
	glm::vec3 global_isect;  // local position of intersection
};


namespace Brushes {
	enum {
		STANDARD,
		SMOOTH
	};
}

struct StandardBrushSettings {
	// the max number inputs to be applied
	// usefull for high message amounts from device
	// uint32_t max_points;

	// the number of points to average position before
	// applying one
	// uint32_t average_count;

	// TODO: drag behind dot elasticy

	BrushProperty<uint32_t> sample_count;
	BrushProperty<float> radius;
	BrushProperty<float> strength;
	BrushProperty<BrushFalloff> falloff;  // only the local value is used

	std::vector<BrushStep> raw_steps;
	std::vector<BrushStep> applied_steps;

	// Secondary brush usully on SHIFT key
	//uint32_t second_brush;
};


struct SmoothBrushSettings {

};

// smooth valleys
// smooth peaks

// carve (pinch + standard)
// inflate
// clay strips

// sharpen
// chamfer
// bevel

// attract
// spread

// move
// move topology
// snake
// move contour


struct SculptContext {
	MeshInstanceRef target;
	
	// average N steps before applying one
	uint32_t global_sample_count;
	float global_brush_radius;  // size relative to viewport height	
	float global_brush_strength;  // strength as a fraction of the radius
	BrushFalloff global_brush_falloff;

	//float strength_time_unit;

	bool stroke_started;

	StandardBrushSettings standard_brush;
};


struct Viewport {
	u32 width;
	u32 height;

	bool pan_started = false;
};

struct DebugStuff {
	bool capture_frame;
};

class Application {
public:
	std::thread thread;  // this is the thread that runs the backend

	Window window;
	Input input;

	// Timing
	SteadyTime frame_start_time;
	float delta_time;  // the total duration of the last frame
	u32 min_frame_duration_ms;  // the minimum amount of time a frame must last (60 fps limit)

	Viewport viewport;
	Camera camera;

	// Meshes
	std::list<Mesh> meshes;

	// Drawcalls
	std::list<MeshDrawcall> drawcalls;

	// Layers
	std::list<MeshLayer> layers;



	// Selection

	// current selection of instances
	std::list<MeshInstanceRef> instance_selection;

	SculptContext sculpt;

	// Shading
	uint32_t shading_normal;  // what normal to use when shading the mesh in the pixel shader

	// Lighting
	std::array<CameraLight, 8> lights;  // lights don't have position the only have a direction
	float ambient_intensity;  // base ambient light added to the color

	DebugStuff debug;

public:
	Mesh& _createMesh();
	MeshInstanceRef _addInstance(Mesh& mesh, MeshLayer* dest_layer, MeshDrawcall* dest_drawcall);
	void _unparentInstanceFromParentLayer(MeshInstanceRef& instance);

	struct _RaytraceInstancesResult {
		MeshInstance* inst;
		uint32_t poly;
		glm::vec3 local_isect;  // local position of intersection same for instances of the same mesh
		glm::vec3 global_isect;  // local position of intersection
	};
public:
	void init(bool enable_debugger);
	void main(bool enable_debugger);
	void CPU_update();

	// reset the scene
	void reset();


	// Instances

	// Invalidates MeshInstance*
	MeshInstanceRef copyInstance(MeshInstanceRef& source);

	// Invalidates MeshInstance*
	void deleteInstance(MeshInstanceRef& inst);

	// unparent from parent layer and make child to destination layer
	void transferInstanceToLayer(MeshInstanceRef& instance, MeshLayer* dest_layer);

	// Copies Instance from one set to another set of the same Mesh that has a set with the
	// destination drawcall. If the set with the destination drawcall does not exist it is created
	// 
	// Returns a reference to instance allocated on a different instance set of the same mesh
	// Invalidates MeshInstance* and MeshInstanceRef
	//MeshInstanceRef moveInstanceToDrawcall(MeshInstanceRef& instance, MeshDrawcall* dest_drawcall);

	void rotateInstance(MeshInstance* mesh_instance, float x, float y, float z);


	// Drawcalls

	// Creates a drawcall to be associated with instances
	MeshDrawcall* createDrawcall();

	// Deletes a drawcall and moves instances to the last used drawcall
	//void deleteDrawcall();

	// get's the default drawcall referenced in the menus
	MeshDrawcall& getRootDrawcall();

	// Iterate over all instances that are rendered with that drawcall and turn on AABB
	// vertex generation
	// needs to be retoggled if mesh changes to recreate AABB vertex buffers
	void setAABB_RenderModeForDrawcall(MeshDrawcall* drawcall, AABB_RenderMode aabb_render_mode);


	// Layers

	// Creates a new layer by default parented to root
	MeshLayer* createLayer(MeshLayer* parent = nullptr);

	// unparent and make child to parent layer
	void transferLayer(MeshLayer* child_layer, MeshLayer* parent_layer);

	// Recursivelly sets the visibility state for all instances of a layer
	void setLayerVisibility(MeshLayer* layer, bool visible_state);


	// Create Meshes
	// All of them invalidate MeshInstance*

	MeshInstanceRef createEmptyMesh(MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createTriangle(CreateTriangleInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createQuad(CreateQuadInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createWavyGrid(CreateWavyGridInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createCube(CreateCubeInfo& infos, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createCylinder(CreateCylinderInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	MeshInstanceRef createUV_Sphere(CreateUV_SphereInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	
	// import a GLTF from path
	ErrStack importMeshesFromGLTF_File(filesys::Path<char>& path, GLTF_ImporterSettings& settings,
		std::vector<MeshInstanceRef>* r_instances = nullptr);

	//MeshInstance* createLine(CreateLineInfo& info, MeshLayer* dest_layer = nullptr, MeshDrawcall* dest_drawcall = nullptr);
	
	// all sources get converted into geometry and added to the destination mesh
	// Invalidates MeshInstance*
	void joinMeshes(std::vector<MeshInstanceRef>& sources, uint32_t destination_idx);


	// Sculpt Mode


	// Raycasts

	// performs a raycast from camera position to pixel world position of the mouse
	bool mouseRaycastInstances(RaytraceInstancesResult& r_isect);


	// Scene

	// reset everything, also used for initialization
	void resetToHardcodedStartup();


	// Other

	// which primitive type will have it's normal interact with the lighting
	void setShadingNormal(uint32_t shading_normal);


	// Debug

	// void triggerRenderDocCaptureOnKey(uint32_t key_down = VK_F7);

	// trigger Visual Studio breakpoint when key is down
	void triggerBreakpointOnKey(uint32_t key_down = VK_F8);
};

extern Application app;
