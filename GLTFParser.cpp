

// Header
#include "Importer.h"


/* GLTF Types */

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
	GLTF_BYTE = 5120,
	GLTF_UNSIGNED_BYTE = 5121,
	GLTF_SHORT = 5122,
	GLTF_UNSIGNED_SHORT = 5123,
	GLTF_UNSIGNED_INT = 5125,
	GLTF_FLOAT = 5126
};

/* Atributes */
#define atribute_name_position "POSITION"
#define atribute_name_texcoord_0 "TEXCOORD_0"
#define atribute_name_normal "NORMAL"


class BitVector {
public:
	std::vector<char> bytes;

	uint64_t bit_count = 0;

public:
	/* add a bit to the end */
	void push_back(bool bit);

	/* return false if character is unrecognized */
	bool pushBase64Char(char b64_c);
};

void BitVector::push_back(bool bit)
{
	if (!bit_count || bit_count % 8 == 0) {
		bytes.push_back(bit << 7);
	}
	else {
		uint8_t next_bit = 8 - (bit_count % 8) - 1;
		bytes[bit_count / 8] |= bit << next_bit;
	}
	bit_count++;
}

/* converts one Base64 character to 6 bits */
bool BitVector::pushBase64Char(char c)
{
	uint8_t d;

	// A to Z
	if (c > 0x40 && c < 0x5b) {
		d = c - 65;  // Base64 A is 0
	}
	// a to z
	else if (c > 0x60 && c < 0x7b) {
		d = c - 97 + 26;  // Base64 a is 26
	}
	// 0 to 9
	else if (c > 0x2F && c < 0x3a) {
		d = c - 48 + 52;  // Base64 0 is 52
	}
	else if (c == '+') {
		d = 0b111110;
	}
	else if (c == '/') {
		d = 0b111111;
	}
	else if (c == '=') {
		d = 0;
	}
	else {
		return false;
	}

	push_back(d & 0b100000);
	push_back(d & 0b010000);
	push_back(d & 0b001000);
	push_back(d & 0b000100);
	push_back(d & 0b000010);
	push_back(d & 0b000001);

	return true;
}

ErrStack loadFromURI(std::string& uri, FileSysPath& this_file, BitVector& r_bin)
{
	ErrStack err;

	// Data URI
	std::string bin_tag = "data:application/octet-stream;base64,";
	size_t uri_i = uri.find(bin_tag);

	if (uri_i != std::string::npos) {

		uri_i += bin_tag.length();
		r_bin.bytes.reserve(uri.size());

		// decode rest of URI as Base64 binary
		for (; uri_i < uri.size(); uri_i++) {

			if (uri[uri_i] >= 0x21) {
				r_bin.pushBase64Char(uri[uri_i]);
			}
		}
	}
	// Relative URI path
	else {
		FileSysPath bin_file = this_file;
		bin_file.pop_back();  // now point to directory containing file
		bin_file.push_back(uri);  // point to file

		if (bin_file.hasExtension("bin")) {

			// load directly 
			checkErrStack(bin_file.read(r_bin.bytes), 
				"failed to read from URI");
			r_bin.bit_count = r_bin.bytes.size() * 8;
		}
		else {
			return ErrStack(code_location, "unsupported URI");
		}
		// TODO: GLB format
	}

	return ErrStack();
}

/* WARNING: These macros add declarations */

#define expectJSONint64(val, int64_num) \
	int64_t* int64_num = std::get_if<int64_t>(&val->value); \
	if (int64_num == nullptr) { \
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#int64_num) + " to be of INT32 type"); \
	}

#define expectJSONString(val, strg) \
	std::string* strg = std::get_if<std::string>(&val->value); \
	if (strg == nullptr) { \
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#strg) + " to be of STRING type"); \
	}

#define expectJSONArray(val, arr) \
	std::vector<JSONValue*>* arr = std::get_if<std::vector<JSONValue*>>(&val->value); \
	if (arr == nullptr) { \
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#arr) + " to be of ARRAY type"); \
	}

#define expectJSONObject(val, obj) \
	std::vector<JSONField>* obj = std::get_if<std::vector<JSONField>>(&val->value); \
	if (obj == nullptr) { \
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#obj) + " to be of OBJECT type"); \
	}


 /* only checks for complaiance with required fields of the GLTF Standard */
ErrStack jsonToGLTF(JSONGraph& json_graph, Structure& gltf_struct)
{
	JSONValue& root = *json_graph.root;

	std::vector<JSONField> gltf_fields = std::get<std::vector<JSONField>>(root.value);
	if (gltf_fields.empty()) {
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"GLTF has no fields");
	}

	for (JSONField& field : gltf_fields) {

		if (field.name == "asset") {

			expectJSONObject(field.value, asset_fields);

			for (JSONField& asset_field : *asset_fields) {

				if (asset_field.name == "version") {

					expectJSONString(asset_field.value, version);

					gltf_struct.asset.version = *version;
				}
			}

			if (!gltf_struct.asset.version.length()) {
				return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
					"required field asset.version could not be found");
			}
		}
		else if (field.name == "scenes") {

			expectJSONArray(field.value, scenes);

			gltf_struct.scenes.resize(scenes->size());

			uint32_t scene_idx = 0;
			for (JSONValue* scene : *scenes) {

				expectJSONObject(scene, scene_fields);

				for (JSONField& scene_field : *scene_fields) {

					if (scene_field.name == "nodes") {

						expectJSONArray(scene_field.value, nodes);

						gltf_struct.scenes[scene_idx].nodes.resize(nodes->size());

						uint32_t node_idx = 0;
						for (JSONValue* node : *nodes) {

							expectJSONint64(node, root_node);

							gltf_struct.scenes[scene_idx].nodes[node_idx] = *root_node;

							node_idx++;
						}
					}
				}

				scene_idx++;
			}
		}
		else if (field.name == "nodes") {

			expectJSONArray(field.value, nodes);

			gltf_struct.nodes.resize(nodes->size());

			uint32_t nodes_idx = 0;
			for (JSONValue* node : *nodes) {

				expectJSONObject(node, node_fields);

				for (JSONField& node_field : *node_fields) {

					if (node_field.name == "mesh") {

						expectJSONint64(node_field.value, mesh);

						gltf_struct.nodes[nodes_idx].mesh = *mesh;
					}
				}
				nodes_idx++;
			}
		}
		else if (field.name == "meshes") {

			expectJSONArray(field.value, meshes);

			gltf_struct.meshes.resize(meshes->size());

			uint32_t mesh_idx = 0;
			for (JSONValue* mesh : *meshes) {

				expectJSONObject(mesh, mesh_fields);

				Mesh& mesh = gltf_struct.meshes[mesh_idx];

				for (JSONField& mesh_field : *mesh_fields) {

					if (mesh_field.name == "primitives") {

						expectJSONArray(mesh_field.value, primitives);

						mesh.primitives.resize(primitives->size());

						uint32_t primitive_idx = 0;
						for (JSONValue* primitive : *primitives) {

							expectJSONObject(primitive, primitive_fields);

							Primitive& prim = mesh.primitives[primitive_idx];

							for (JSONField& primitive_field : *primitive_fields) {

								// Primitives fields
								if (primitive_field.name == "attributes") {

									expectJSONObject(primitive_field.value, atributes);
									for (JSONField& atributes_field : *atributes) {

										expectJSONint64(atributes_field.value, attr_value);
										prim.atributes.insert(std::pair<std::string, int64_t>(atributes_field.name, *attr_value));
									}

									if (prim.atributes.find(atribute_name_position) == prim.atributes.end()) {
										return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
											"required field meshes.primitives.attributes.POSITION could not be found");
									}
								}
								else if (primitive_field.name == "indices") {

									expectJSONint64(primitive_field.value, indices);

									prim.indices = *indices;
								}
							}

							if (!prim.atributes.size()) {
								return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
									"required field meshes.primitives.attributes could not be found");
							}

							primitive_idx++;
						}
					}
				}

				if (!mesh.primitives.size()) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field meshes.primitives could not be found");
				}

				mesh_idx++;
			}
		}
		else if (field.name == "buffers") {

			expectJSONArray(field.value, buffers);

			gltf_struct.buffers.resize(buffers->size());

			uint32_t buffer_idx = 0;
			for (JSONValue* buffer : *buffers) {

				expectJSONObject(buffer, buffer_fields);

				bool byte_length_found = false;
				for (JSONField& buffer_field : *buffer_fields) {

					Buffer& buff = gltf_struct.buffers[buffer_idx];

					if (buffer_field.name == "uri") {

						expectJSONString(buffer_field.value, uri);
						buff.uri = *uri;
					}
					else if (buffer_field.name == "byteLength") {

						expectJSONint64(buffer_field.value, byte_length);
						buff.byte_length = *byte_length;

						byte_length_found = true;
					}
				}

				if (!byte_length_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field buffers.byteLength could not be found");
				}

				buffer_idx++;
			}
		}
		else if (field.name == "bufferViews") {

			expectJSONArray(field.value, buffer_views);

			gltf_struct.buffer_views.resize(buffer_views->size());

			uint32_t buffer_view_idx = 0;
			for (JSONValue* buffer_view : *buffer_views) {

				expectJSONObject(buffer_view, buffer_view_fields);

				bool buffer_found = false;
				bool byte_length_found = false;
				for (JSONField& buffer_view_field : *buffer_view_fields) {

					BufferView& buff_view = gltf_struct.buffer_views[buffer_view_idx];

					if (buffer_view_field.name == "buffer") {

						expectJSONint64(buffer_view_field.value, buffer_idx);
						buff_view.buffer = *buffer_idx;

						buffer_found = true;
					}
					else if (buffer_view_field.name == "byteOffset") {

						expectJSONint64(buffer_view_field.value, byte_offset);
						buff_view.byte_offset = *byte_offset;
					}
					else if (buffer_view_field.name == "byteLength") {

						expectJSONint64(buffer_view_field.value, byte_length);
						buff_view.byte_length = *byte_length;

						byte_length_found = true;
					}
					else if (buffer_view_field.name == "target") {

						expectJSONint64(buffer_view_field.value, target);
						buff_view.target = *target;
					}
				}

				if (!buffer_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field bufferViews.buffer could not be found");
				}
				if (!byte_length_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field bufferViews.byteLength could not be found");
				}

				buffer_view_idx++;
			}
		}
		else if (field.name == "accessors") {

			expectJSONArray(field.value, accessors);

			gltf_struct.accessors.resize(accessors->size());

			uint32_t accessor_idx = 0;
			for (JSONValue* accessor : *accessors) {

				expectJSONObject(accessor, accessor_fields);

				bool component_type_found = false;
				bool count_found = false;
				bool type_found = false;
				for (JSONField& accessor_field : *accessor_fields) {

					Accessor& acc = gltf_struct.accessors[accessor_idx];

					if (accessor_field.name == "bufferView") {

						expectJSONint64(accessor_field.value, buffer_view);
						acc.buffer_view = *buffer_view;
					}
					else if (accessor_field.name == "byteOffset") {

						expectJSONint64(accessor_field.value, byte_offset);
						acc.byte_offset = *byte_offset;
					}
					else if (accessor_field.name == "componentType") {

						expectJSONint64(accessor_field.value, component_type);
						acc.component_type = *component_type;

						component_type_found = true;
					}
					else if (accessor_field.name == "count") {

						expectJSONint64(accessor_field.value, count);
						acc.count = *count;

						count_found = true;
					}
					else if (accessor_field.name == "type") {

						expectJSONString(accessor_field.value, type);
						acc.type = *type;

						type_found = true;
					}
				}

				if (!component_type_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.componentType could not be found");
				}
				if (!count_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.count could not be found");
				}
				if (!type_found) {
					return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.type could not be found");
				}

				accessor_idx++;
			}
		}
	}

	return ErrStack();
}

ErrStack loadIndexesFromBuffer(Structure& gltf_struct, uint64_t acc_idx, 
	std::vector<BitVector>& bin_buffs, std::vector<uint32_t>& r_indexes)
{
	Accessor& acc = gltf_struct.accessors[acc_idx];
	BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];

	if (acc.type != "SCALAR") {
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"expected indices accessor type to be SCALAR but instead got " + acc.type);
	}

	uint64_t count = acc.count;

	r_indexes.resize(count);
	BitVector& buffer = bin_buffs[buff_view.buffer];
	uint64_t offset = buff_view.byte_offset + acc.byte_offset;

	switch (acc.component_type) {
		// fast path
	case GLTF_UNSIGNED_INT: {

		std::memcpy(r_indexes.data(), buffer.bytes.data() + offset, sizeof(uint32_t) * r_indexes.size());
		break;
	}

	case GLTF_UNSIGNED_BYTE: {

		std::vector<uint8_t> uint8_idxs;
		uint8_idxs.resize(count);

		std::memcpy(uint8_idxs.data(), buffer.bytes.data() + offset, sizeof(uint8_t) * uint8_idxs.size());

		for (uint64_t i = 0; i < count; i++) {
			r_indexes[i] = uint8_idxs[i];
		}
		break;
	}

	case GLTF_UNSIGNED_SHORT: {

		std::vector<uint16_t> uint16_idxs;
		uint16_idxs.resize(count);

		std::memcpy(uint16_idxs.data(), buffer.bytes.data() + offset, sizeof(uint16_t) * uint16_idxs.size());

		for (uint64_t i = 0; i < count; i++) {
			r_indexes[i] = uint16_idxs[i];
		}
		break;
	}
	default:
		return ErrStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"invalid component_type for index buffer allowed types are BYTE, UNSIGNED_SHORT, UNSIGNED_INT");
	}

	return ErrStack();
}

ErrStack loadVec2FromBuffer(Structure& gltf_struct, uint64_t acc_idx,
	std::vector<BitVector>& bin_buffs, std::vector<glm::vec2>& r_vecs)
{
	Accessor& acc = gltf_struct.accessors[acc_idx];
	BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];

	if (acc.type != "VEC2") {
		return ErrStack(code_location,
			"expected texture coordinates atribute accesor type to be VEC2 but instead got " + acc.type);
	}

	r_vecs.resize(acc.count);
	BitVector& buff = bin_buffs[buff_view.buffer];
	uint64_t offset = buff_view.byte_offset + acc.byte_offset;

	if (acc.component_type == GLTF_FLOAT) {
		std::memcpy(r_vecs.data(), buff.bytes.data() + offset, sizeof(glm::vec2) * acc.count);
	}
	else {
		return ErrStack(code_location,
			"invalid component_type for texture coordinates, can only be FLOAT");
	}

	return ErrStack();
}

ErrStack loadVec3FromBuffer(Structure& gltf_struct, uint64_t acc_idx, 
	std::vector<BitVector>& bin_buffs, std::vector<glm::vec3>& r_vecs)
{
	Accessor& acc = gltf_struct.accessors[acc_idx];
	BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];

	if (acc.type != "VEC3") {
		return ErrStack(code_location,
			"expected position atribute accessor type to be VEC3 but instead got " + acc.type);
	}

	r_vecs.resize(acc.count);
	BitVector& buffer = bin_buffs[buff_view.buffer];
	uint64_t offset = buff_view.byte_offset + acc.byte_offset;

	if (acc.component_type == GLTF_FLOAT) {
		std::memcpy(r_vecs.data(), buffer.bytes.data() + offset, sizeof(glm::vec3) * acc.count);
	}
	else {
		return ErrStack(code_location,
			"invalid component_type for position, can only be FLOAT");
	}

	return ErrStack();
}

ErrStack importGLTFMeshes(FileSysPath& path, std::vector<LinkageMesh>& meshes)
{
	ErrStack err;

	std::vector<char> file_content;
	checkErrStack1(path.read(file_content));

	// JSON Text to JSON Types
	JSONGraph json;
	uint64_t i = 0;

	err = parseJSON(file_content, 0, json);
	if (err.isBad()) {
		err.pushError(code_location, "failed to parse JSON");
		return err;
	}

	// JSON Types to C++ Types
	Structure gltf_struct;

	err = jsonToGLTF(json, gltf_struct);
	if (err.isBad()) {
		err.pushError(code_location, "failed to parse GLTF");
		return err;
	}

	// Convert URI to buffer data
	std::vector<BitVector> bin_buffs;
	{
		bin_buffs.resize(gltf_struct.buffers.size());

		for (uint64_t i = 0; i < gltf_struct.buffers.size(); i++) {
			
			std::string& uri = gltf_struct.buffers[i].uri;
			BitVector& bin_buff = bin_buffs[i];

			err = loadFromURI(uri, path, bin_buff);
			if (err.isBad()) {
				err.pushError(code_location, "failed to parse URI");
				return err;
			}
		}
	}

	// Extract meaningfull data
	{
		// Primitives also count as meshes
		uint64_t mesh_count = 0;
		for (Mesh& gltf_mesh : gltf_struct.meshes) {
			for (Primitive& prim : gltf_mesh.primitives) {

				mesh_count++;
			}
		}
		meshes.resize(mesh_count);

		// Heap reuse
		VertexAtributes attrs;
		std::vector<uint32_t> indexes;

		uint64_t mesh_idx = 0;
		for (Mesh& gltf_mesh : gltf_struct.meshes) {
			for (Primitive& prim : gltf_mesh.primitives) {

				// Indexes
				{
					err = loadIndexesFromBuffer(gltf_struct, prim.indices.value(), bin_buffs, indexes);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load indexes from buffer");
						return err;
					}
				}

				// Positions
				{
					uint64_t acc_idx = prim.atributes.at(atribute_name_position);

					err = loadVec3FromBuffer(gltf_struct, acc_idx, bin_buffs, attrs.positions);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load positions from buffer");
						return err;
					}
				}

				// UVs
				auto uv_it = prim.atributes.find(atribute_name_texcoord_0);
				if (uv_it != prim.atributes.end()) {

					err = loadVec2FromBuffer(gltf_struct, uv_it->second, bin_buffs, attrs.uvs);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load texture coordinates from buffer");
						return err;
					}
				}
				else {
					attrs.uvs.clear();
				}

				// Normals
				auto normal_it = prim.atributes.find(atribute_name_normal);
				if (normal_it != prim.atributes.end()) {

					err = loadVec3FromBuffer(gltf_struct, normal_it->second, bin_buffs, attrs.normals);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load normals from buffer");
						return err;
					}
				}
				else {
					attrs.normals.clear();
				}

				// Create Linkage Mesh
				addTriangleListToMesh(meshes[mesh_idx], indexes, attrs, true);

				mesh_idx++;
			}
		}
	}

	return err;
}