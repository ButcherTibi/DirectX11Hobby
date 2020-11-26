#pragma once

// GLM
#include "glm\gtc\quaternion.hpp"

#include "SculptMesh.hpp"
#include "Renderer.hpp"


//enum class MeshTransparency {
//	OPAQUE_SHADING,
//	TRANSPARENT_SHADING,
//	WIREFRAME
//};

enum class MeshShading {
	FLAT_POLY,
	FLAT_TESSELATION,
	SMOOTH_VERTEX,
};

//namespace MeshHighlightMode {
//	enum : uint8_t {
//		VERTEX = 1,
//		NORMAL = 1 << 1,
//		EDGE   = 1 << 2,
//		POLY   = 1 << 3,
//		TESS   = 1 << 4
//	};
//}

class MeshInstance {
public:
	scme::SculptMesh* mesh;
	
	// GPU Instance
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;

	// GPU Draw Call
	// hide mesh
	MeshShading shading;
};

struct GLTF_ImporterSettings {

};

struct MeshLayer {
	std::list<MeshLayer*> children;

	std::string name;
	std::list<MeshInstance*> instances;
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
	void createTriangleMesh(glm::vec3& pos, glm::quat& rot, float size);
	void createCubeMesh(glm::vec3& pos, glm::quat& rot, float size);
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