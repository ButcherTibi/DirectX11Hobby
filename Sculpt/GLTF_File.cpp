
// Header
#include "GLTF_File.hpp"


using namespace gltf;


/* These macros add declarations */
#define expectJSONint64(val, int64_num) \
	int64_t* int64_num = std::get_if<int64_t>(&val->value); \
	if (int64_num == nullptr) { \
		return ErrStack(code_location, \
			"expected JSON field " + std::string(#int64_num) + " to be of INT64 type"); \
	}

#define expectJSONString(val, strg) \
	std::string* strg = std::get_if<std::string>(&val->value); \
	if (strg == nullptr) { \
		return ErrStack(code_location, \
			"expected JSON field " + std::string(#strg) + " to be of STRING type"); \
	}

#define expectJSONArray(val, arr) \
	std::vector<json::Value*>* arr = std::get_if<std::vector<json::Value*>>(&val->value); \
	if (arr == nullptr) { \
		return ErrStack(code_location, \
			"expected JSON field " + std::string(#arr) + " to be of ARRAY type"); \
	}

#define expectJSONObject(val, obj) \
	std::vector<json::Field>* obj = std::get_if<std::vector<json::Field>>(&val->value); \
	if (obj == nullptr) { \
		return ErrStack(code_location, \
			"expected JSON field " + std::string(#obj) + " to be of OBJECT type"); \
	}


constexpr char* data_URI_prefix = "data:application/octet-stream;base64,";


ErrStack Structure::_loadIndexesFromBuffer(uint64_t acc_idx,
	std::vector<base64::BitVector>& bin_buffs, std::vector<uint32_t> & r_indexes)
{
	Accessor& acc = accessors[acc_idx];
	BufferView& buff_view = buffer_views[acc.buffer_view];
	Buffer& buffer = buffers[buff_view.buffer];
	base64::BitVector& bin_buffer = bin_buffs[buff_view.buffer];

	if (acc.type != Types::scalar) {
		return ErrStack(code_location,
			"expected indices accessor type to be SCALAR but instead got " + acc.type + " for accessor " + acc.name);
	}

	r_indexes.resize(acc.count);
	uint64_t offset = buff_view.byte_offset + acc.byte_offset;

	switch (acc.component_type) {
	case ComponentType::UNSIGNED_INT: {

		std::memcpy(r_indexes.data(), bin_buffer.bytes.data() + offset, sizeof(uint32_t) * r_indexes.size());
		break;
	}

	case ComponentType::UNSIGNED_SHORT: {

		std::vector<uint16_t> uint16_idxs;
		uint16_idxs.resize(acc.count);

		std::memcpy(uint16_idxs.data(), bin_buffer.bytes.data() + offset, sizeof(uint16_t) * uint16_idxs.size());

		for (uint64_t i = 0; i < acc.count; i++) {
			r_indexes[i] = uint16_idxs[i];
		}
		break;
	}
	default:
		return ErrStack(code_location,
			"invalid component_type for index buffer allowed types are UNSIGNED_SHORT, UNSIGNED_INT for accessor " + acc.name);
	}

	return ErrStack();
}

ErrStack Structure::_loadVec3FromBuffer(uint64_t acc_idx,
	std::vector<base64::BitVector>& bin_buffs, std::vector<glm::vec3>& r_vecs)
{
	Accessor& acc = accessors[acc_idx];
	BufferView& buff_view = buffer_views[acc.buffer_view];
	Buffer& buffer = buffers[buff_view.buffer];
	base64::BitVector& bin_buffer = bin_buffs[buff_view.buffer];

	if (acc.type != Types::vec3) {
		return ErrStack(code_location,
			"expected position atribute accessor type to be VEC3 but instead got " + acc.type + " for accessor " + acc.name);
	}

	r_vecs.resize(acc.count);
	uint64_t offset = buff_view.byte_offset + acc.byte_offset;

	if (acc.component_type == ComponentType::FLOAT) {

		// TODO: account for stride
		std::memcpy(r_vecs.data(), bin_buffer.bytes.data() + offset, sizeof(float[3]) * acc.count);
	}
	else {
		return ErrStack(code_location,
			"invalid component_type for position, can only be FLOAT for accessor " + acc.name);
	}

	return ErrStack();
}

ErrStack Structure::importGLTF(json::Graph& json_graph)
{
	json::Value& root = *json_graph.root;

	std::vector<json::Field> gltf_fields = std::get<std::vector<json::Field>>(root.value);
	if (gltf_fields.empty()) {
		return ErrStack(code_location, "GLTF has no fields");
	}

	for (json::Field& field : gltf_fields) {

		if (field.name == "asset") {

			expectJSONObject(field.value, asset_fields);

			for (json::Field& asset_field : *asset_fields) {

				if (asset_field.name == "version") {

					expectJSONString(asset_field.value, version);
					asset.version = *version;
				}
				else if (asset_field.name == "generator") {

					expectJSONString(asset_field.value, generator);
					asset.generator = *generator;
				}
				else if (asset_field.name == "copyright") {

					expectJSONString(asset_field.value, copyright);
					asset.copyright = *copyright;
				}
			}
		}
		else if (field.name == "scenes") {

			expectJSONArray(field.value, new_scenes);
			scenes.resize(new_scenes->size());

			uint32_t scene_idx = 0;
			for (json::Value* new_scene : *new_scenes) {

				Scene& scene = scenes[scene_idx];

				expectJSONObject(new_scene, scene_fields);
				for (json::Field& scene_field : *scene_fields) {

					// Scene Nodes
					if (scene_field.name == "nodes") {

						expectJSONArray(scene_field.value, nodes);
						scene.nodes.resize(nodes->size());

						uint32_t node_idx = 0;
						for (json::Value* node : *nodes) {

							expectJSONint64(node, root_node);
							scene.nodes[node_idx] = *root_node;

							node_idx++;
						}
					}
					// Scene Name
					else if (scene_field.name == "name") {

						expectJSONString(scene_field.value, name);
						scenes[scene_idx].name = *name;
					}
				}

				scene_idx++;
			}
		}
		else if (field.name == "nodes") {

			expectJSONArray(field.value, new_nodes);
			nodes.resize(new_nodes->size());

			uint32_t nodes_idx = 0;
			for (json::Value* new_node : *new_nodes) {

				Node& node = nodes[nodes_idx];

				expectJSONObject(new_node, node_fields);
				for (json::Field& node_field : *node_fields) {

					if (node_field.name == "mesh") {

						expectJSONint64(node_field.value, mesh);
						node.mesh = *mesh;
					}
					else if (node_field.name == "name") {

						expectJSONString(node_field.value, name);
						node.name = *name;
					}
					// Transforms unused
					// Children unused
				}
				nodes_idx++;
			}
		}
		else if (field.name == "meshes") {

			expectJSONArray(field.value, new_meshes);
			meshes.resize(new_meshes->size());

			uint32_t mesh_idx = 0;
			for (json::Value* new_mesh : *new_meshes) {

				Mesh& mesh = meshes[mesh_idx];

				expectJSONObject(new_mesh, mesh_fields);
				for (json::Field& mesh_field : *mesh_fields) {
					
					if (mesh_field.name == "name") {

						expectJSONString(mesh_field.value, name);
						mesh.name = *name;
					}
					else if (mesh_field.name == "primitives") {

						expectJSONArray(mesh_field.value, primitives);
						mesh.primitives.resize(primitives->size());

						uint32_t primitive_idx = 0;
						for (json::Value* primitive : *primitives) {

							Primitive& prim = mesh.primitives[primitive_idx];

							expectJSONObject(primitive, primitive_fields);
							for (json::Field& primitive_field : *primitive_fields) {
					
								// Primitive Indices
								if (primitive_field.name == "indices") {

									expectJSONint64(primitive_field.value, indices);
									prim.indices = *indices;
								}
								// Primitive Atributes
								else if (primitive_field.name == "attributes") {

									expectJSONObject(primitive_field.value, atributes);
									for (json::Field& atributes_field : *atributes) {

										expectJSONint64(atributes_field.value, attr_value);
										prim.attributes.insert(std::pair<std::string, int64_t>(atributes_field.name, *attr_value));
									}

									if (prim.attributes.find(Atributes::position) == prim.attributes.end()) {
										return ErrStack(code_location,
											"required field meshes.primitives.attributes.POSITION could not be found on mesh " + mesh.name);
									}
								}
								// Primitive Mode
								else if (primitive_field.name == "mode") {

									expectJSONint64(primitive_field.value, mode);
									prim.mode = *mode;
								}
							}

							if (!prim.attributes.size()) {
								return ErrStack(code_location,
									"required field meshes.primitives.attributes could not be found on mesh " + mesh.name);
							}

							primitive_idx++;
						}
					}
				}

				if (!mesh.primitives.size()) {
					return ErrStack(code_location,
						"required field meshes.primitives could not be found on mesh " + mesh.name);
				}

				mesh_idx++;
			}
		}
		else if (field.name == "buffers") {

			expectJSONArray(field.value, new_buffers);
			buffers.resize(new_buffers->size());

			uint32_t buffer_idx = 0;
			for (json::Value* new_buffer : *new_buffers) {

				Buffer& buff = buffers[buffer_idx];
				bool byte_length_found = false;

				expectJSONObject(new_buffer, buffer_fields);	
				for (json::Field& buffer_field : *buffer_fields) {

					if (buffer_field.name == "name") {

						expectJSONString(buffer_field.value, name);
						buff.name = *name;
					}
					else if (buffer_field.name == "uri") {

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
					return ErrStack(code_location,
						"required field buffers.byteLength could not be found in buffer " + buff.name);
				}

				buffer_idx++;
			}
		}
		else if (field.name == "bufferViews") {

			expectJSONArray(field.value, new_buffer_views);
			buffer_views.resize(new_buffer_views->size());

			uint32_t buffer_view_idx = 0;
			for (json::Value* new_buffer_view : *new_buffer_views) {

				BufferView& buff_view = buffer_views[buffer_view_idx];

				bool buffer_found = false;
				bool byte_length_found = false;

				expectJSONObject(new_buffer_view, buffer_view_fields);
				for (json::Field& buffer_view_field : *buffer_view_fields) {

					if (buffer_view_field.name == "name") {

						expectJSONString(buffer_view_field.value, name);
						buff_view.name = *name;
					}
					else if (buffer_view_field.name == "buffer") {

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
					return ErrStack(code_location,
						"required field bufferViews.buffer could not be found in BufferView " + buff_view.name);
				}
				if (!byte_length_found) {
					return ErrStack(code_location,
						"required field bufferViews.byteLength could not be found BufferView " + buff_view.name);
				}

				buffer_view_idx++;
			}
		}
		else if (field.name == "accessors") {

			expectJSONArray(field.value, new_accessors);
			accessors.resize(new_accessors->size());

			uint32_t accessor_idx = 0;
			for (json::Value* new_accessor : *new_accessors) {

				Accessor& acc = accessors[accessor_idx];

				bool component_type_found = false;
				bool count_found = false;
				bool type_found = false;

				expectJSONObject(new_accessor, accessor_fields);
				for (json::Field& accessor_field : *accessor_fields) {	

					if (accessor_field.name == "name") {

						expectJSONString(accessor_field.value, name);
						acc.name = *name;
					}
					else if (accessor_field.name == "bufferView") {

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
					return ErrStack(code_location,
						"required field accessors.componentType could not be found in Accessor " + acc.name);
				}
				if (!count_found) {
					return ErrStack(code_location,
						"required field accessors.count could not be found in Accessor " + acc.name);
				}
				if (!type_found) {
					return ErrStack(code_location,
						"required field accessors.type could not be found in Accessor " + acc.name);
				}

				accessor_idx++;
			}
		}
	}

	ErrStack err_stack;

	// Convert URI to buffer data
	std::vector<base64::BitVector> bin_buffs;
	{
		bin_buffs.resize(buffers.size());

		for (uint64_t i = 0; i < buffers.size(); i++) {

			Buffer& buffer = buffers[i];
			std::string& uri = buffer.uri;

			base64::BitVector& bin_buff = bin_buffs[i];
			bin_buff.bytes.reserve(buffer.byte_length);

			// Data URI		
			size_t uri_i = uri.find(data_URI_prefix);

			if (uri_i != std::string::npos) {
				uri_i += sizeof(data_URI_prefix);  // TODO: test for '\0'	

				// decode rest of URI as Base64 binary
				for (; uri_i < uri.size(); uri_i++) {

					char b64 = uri[uri_i];

					// ignore invisible characters
					if (b64 >= 0x21) {
						bin_buff.pushBase64Char(b64);
					}
				}
			}
			// TODO: Relative URI
		}
	}

	// Extract meaningfull data
	{
		uint64_t mesh_idx = 0;
		for (Mesh& mesh : meshes) {
			for (Primitive& prim : mesh.primitives) {

				if (prim.mode != Modes::TRIANGLES) {
					
					// Indexes
					checkErrStack(_loadIndexesFromBuffer(prim.indices, bin_buffs, prim.indexes),
						"failed to load indexes from buffer");

					// Positions
					{
						uint64_t acc_idx = prim.attributes.at(Atributes::position);

						checkErrStack(_loadVec3FromBuffer(acc_idx, bin_buffs, prim.positions),
							"failed to load positions from buffer");
					}

					// Normals
					auto normal_it = prim.attributes.find(Atributes::normal);
					if (normal_it != prim.attributes.end()) {

						checkErrStack(_loadVec3FromBuffer(normal_it->second, bin_buffs, prim.normals),
							"failed to load normals from buffer");
					}
				}
			}

			mesh_idx++;
		}
	}

	return err_stack;
}
