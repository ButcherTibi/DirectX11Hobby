#pragma once

// Standard
#include <unordered_map>

// GLM
#include <glm\vec3.hpp>

#include "JSON_File.hpp"


namespace base64 {

	class BitVector {
	public:
		std::vector<uint8_t> bytes;
		uint64_t bit_count = 0;

	public:
		/* add a bit to the end */
		void push_back(bool bit);

		/* converts one Base64 character to 6 bits
		 * @returns false if character is unrecognized */
		bool pushBase64Char(char b64_c);
	};
}


namespace gltf {

	struct Asset {
		std::string version;
		std::string generator;
		std::string copyright;
	};


	namespace Modes {
		enum {
			POINTS,
			LINES,
			LINE_LOOP,
			LINE_STRIP,
			TRIANGLES,
			TRIANGLE_STRIP,
			TRIANGLE_FAN
		};
	}

	namespace Atributes {
		constexpr char* position = "POSITION";
		constexpr char* normal = "NORMAL";
	}

	struct Primitive {
		std::unordered_map<std::string, uint64_t> attributes;
		uint64_t indices;
		uint64_t mode;

		// Extracted Data
		std::vector<uint32_t> indexes;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
	};


	struct Mesh {
		std::string name;
		std::vector<Primitive> primitives;
	};


	struct Node {
		std::string name;
		std::vector<uint64_t> children;

		// Content
		uint64_t mesh;

		// Transformation
		double translation[3] = { 0, 0, 0 };
		double rotation[4] = { 0, 0, 0, 1 };
		double scale[3] = { 1, 1, 1 };
		double matrix[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
	};


	struct Scene {
		std::vector<uint64_t> nodes;
		std::string name;
	};


	struct Buffer {
		std::string name;
		std::string uri;
		uint64_t byte_length;
	};


	namespace Targets {
		enum {
			ARRAY_BUFFER = 34962,
			ELEMENT_ARRAY_BUFFER = 34963
		};
	}


	struct BufferView {
		std::string name;
		uint64_t buffer;
		uint64_t byte_offset = 0;
		uint64_t byte_length;
		uint64_t byte_stride;
		uint64_t target;
	};


	namespace ComponentType {
		enum {
			BYTE = 5120,
			UNSIGNED_BYTE = 5121,
			SHORT = 5122,
			UNSIGNED_SHORT = 5123,
			UNSIGNED_INT = 5125,
			FLOAT = 5126
		};
	}

	namespace Types {
		constexpr char* scalar = "SCALAR";
		constexpr char* vec2 = "VEC2";
		constexpr char* vec3 = "VEC3";
		constexpr char* vec4 = "VEC4";
		constexpr char* mat2 = "MAT2";
		constexpr char* mat3 = "MAT3";
		constexpr char* mat4 = "MAT4";
	};

	struct Accessor {
		std::string name;

		uint64_t buffer_view;
		uint64_t byte_offset = 0;
		uint64_t component_type;
		bool normalized = false;
		uint64_t count;
		std::string type;
	};


	struct Structure {
		Asset asset;
		std::vector<Scene> scenes;
		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::vector<Buffer> buffers;
		std::vector<BufferView> buffer_views;
		std::vector<Accessor> accessors;

	public:
		ErrStack _loadIndexesFromBuffer(uint64_t acc_idx,
			std::vector<base64::BitVector>& bin_buffs, std::vector<uint32_t>& r_indexes);

		ErrStack _loadVec3FromBuffer(uint64_t acc_idx,
			std::vector<base64::BitVector>& bin_buffs, std::vector<glm::vec3>& r_vecs);

		ErrStack importGLTF(json::Graph& json_graph);
	};
}
