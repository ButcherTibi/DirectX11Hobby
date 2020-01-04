#pragma once

// Standard
#include <variant>
#include <forward_list>
#include <map>
#include <optional>

// Mine
#include "ErrorStuff.h"
#include "FileIO.h"
#include "Meshes.h"


/* is case of demand for large file do:
- use char vector instead of string
- use std::future and/or promise (can't remember which one is which)
- give file completion rate using file index @param i
- call destructor explicit when variable not needed to reduce menory
 */


/* move i to next character */
void skipWhiteSpace(uint64_t& i, std::vector<char>& text);

/* return true or false if symbol has been found */
bool seekSymbol(uint64_t& i, std::vector<char>& text, char symbol);

/* checks if next char sequence == keyword,
 * @param allow_white_space = should whitespaces be ignored in keyword */
ErrorStack confirmKeyword(uint64_t& i, std::vector<char>& text, std::string keyword);

bool checkForKeyword(uint64_t& i, std::vector<char>& text, std::string keyword);

/* assumes i after '"' */
ErrorStack parseString(uint64_t& i, std::vector<char>& text, std::string& out);

// ErrorStack parseNumber(uint64_t& i, std::vector<char>& text, std::variant<int64_t, double>& out);


struct JSONValue;

struct JSONField {
	std::string name;
	JSONValue* value;
};

struct JSONValue {
	std::variant<bool,
		int64_t,
		double,
		std::string,
		std::vector<JSONValue*>,  // array
		std::vector<JSONField>,  // object
		nullptr_t>
		value;
};

class JSONGraph {
public:
	/* Since a JSON value can be an array or object thus recursive,
	 * someone must store where those JSON values point to */
	std::forward_list<JSONValue> values;

	JSONValue* root = nullptr;

	/* Represent integers and real numbers on 32 or 64 bit types */
	bool use_64int = true;
	bool use_double = false;

private:
	/* Stored here for heap reuse in number parsing */
	std::vector<char> integer_part;
	std::vector<char> frac_part;
	std::vector<char> exp_part;

	/* Stored here for heap reuse in string parsing */
	std::string temp_string;

public:
	/* @param i assumes i is at first digit of number,
	 * @param is_large ? int64_t and double : int32_t and float */
	ErrorStack parseNumber(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i after '[' */
	ErrorStack parseArray(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i after '{' */
	ErrorStack parseObject(uint64_t& i, std::vector<char>& text, JSONValue& json_value);
};

// TODO: add support for specifing if a field requires certain representation
/* use @param offset to skip start of file if desired
 * @param use_64int parse ints as int64_t or int32_t
 * @param use_double parse real numbers as float or double */
ErrorStack parseJSON(std::vector<char>& text, uint64_t offset, bool use_64int, bool use_double,
	JSONGraph& json);



namespace bin {

	class Vector {
	public:
		std::vector<char> bytes;

		uint64_t bit_count = 0;

	public:
		/* add a bit to the end */
		void push_back(bool bit);

		/* return false if character is unrecognized */
		bool pushBase64Char(char b64_c);
	};

	ErrorStack loadFromURI(std::vector<char>& uri, Path& this_file, Vector& r_bin);
}


namespace gltf {

	struct Asset {
		std::string version;
	};

	struct Scene {
		std::vector<int64_t> nodes;
	};

	struct Node {
		std::optional<int64_t> mesh;
	};

	struct Primitive {
		std::map<std::string, int64_t> atributes;
		std::optional<int64_t> indices;
	};

	struct Mesh {
		std::vector<Primitive> primitives;
	};

	struct Buffer {
		std::string uri;
		int64_t byte_length;
	};

	struct BufferView {
		int64_t buffer;
		int64_t byte_offset = 0;
		int64_t byte_length;
		std::optional<uint64_t> target;
	};

	struct Accessor {
		std::optional<int64_t> buffer_view;
		int64_t byte_offset = 0;
		int64_t component_type;
		int64_t count;
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
	};

	enum ComponentType {
		BYTE = 5120,
		UNSIGNED_BYTE = 5121,
		SHORT = 5122,
		UNSIGNED_SHORT = 5123,
		UNSIGNED_INT = 5125,
		FLOAT = 5126
	};

	/* Atributes */
	#define atribute_name_position "POSITION"
	#define atribute_name_normal "NORMAL"


	/* Helper functions */

	ErrorStack loadIndexesFromBuffer(gltf::Structure& gltf_struct, uint64_t acc_idx,
		std::vector<bin::Vector>& bin_buffs, std::vector<uint32_t>& indexes);

	ErrorStack loadVec3FromBuffer(gltf::Structure& gltf_struct, uint64_t acc_idx, 
		std::vector<bin::Vector>& bin_buffs, std::vector<glm::vec3>& vecs);


	/* Functions */

	ErrorStack importMeshes(Path path, std::vector<LinkageMesh>& meshes);
};
