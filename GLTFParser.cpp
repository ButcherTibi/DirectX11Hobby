

// Header
#include "Importer.h"


/* WARNING: These macros add declarations */

#define expectJSONint64(val, int64_num) \
	int64_t* int64_num = std::get_if<int64_t>(&val->value); \
	if (int64_num == nullptr) { \
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#int64_num) + " to be of INT32 type"); \
	}

#define expectJSONString(val, strg) \
	std::string* strg = std::get_if<std::string>(&val->value); \
	if (strg == nullptr) { \
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#strg) + " to be of STRING type"); \
	}

#define expectJSONArray(val, arr) \
	std::vector<JSONValue*>* arr = std::get_if<std::vector<JSONValue*>>(&val->value); \
	if (arr == nullptr) { \
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#arr) + " to be of ARRAY type"); \
	}

#define expectJSONObject(val, obj) \
	std::vector<JSONField>* obj = std::get_if<std::vector<JSONField>>(&val->value); \
	if (obj == nullptr) { \
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, \
			"expected JSON field " + std::string(#obj) + " to be of OBJECT type"); \
	}

ErrorStack jsonToGLTF(JSONGraph& json_graph, gltf::Structure& gltf_struct)
{
	JSONValue& root = *json_graph.root;

	std::vector<JSONField> gltf_fields = std::get<std::vector<JSONField>>(root.value);
	if (gltf_fields.empty()) {
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"GLTF has no fields");
	}

	for (JSONField& field : gltf_fields) {

		if (field.name == "asset") {

			expectJSONObject(field.value, asset_fields);

			bool version_found = false;

			for (JSONField& asset_field : *asset_fields) {

				if (asset_field.name == "version") {

					expectJSONString(asset_field.value, version);

					gltf_struct.asset.version = *version;
					version_found = true;
				}
			}

			if (!version_found) {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
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

				gltf::Mesh& mesh = gltf_struct.meshes[mesh_idx];

				bool primitives_found = false;
				for (JSONField& mesh_field : *mesh_fields) {

					if (mesh_field.name == "primitives") {

						expectJSONArray(mesh_field.value, primitives);

						mesh.primitives.resize(primitives->size());

						primitives_found = true;

						uint32_t primitive_idx = 0;
						for (JSONValue* primitive : *primitives) {

							expectJSONObject(primitive, primitive_fields);

							gltf::Primitive& prim = mesh.primitives[primitive_idx];

							bool attributes_found = false;
							for (JSONField& primitive_field : *primitive_fields) {

								// Primitives fields
								if (primitive_field.name == "attributes") {

									bool pos_found = false;

									expectJSONObject(primitive_field.value, atributes);							
									for (JSONField& atributes_field : *atributes) {

										// Vertex Atributes
										if (atributes_field.name == atribute_name_position) {

											expectJSONint64(atributes_field.value, position);
											prim.atributes.insert(std::pair<std::string, int64_t>(atribute_name_position, *position));

											pos_found = true;
										}
										else if (atributes_field.name == atribute_name_normal) {

											expectJSONint64(atributes_field.value, normal);
											prim.atributes.insert(std::pair<std::string, int64_t>(atribute_name_normal, *normal));
										}
									}
									attributes_found = true;

									if (!pos_found) {
										return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
											"required field meshes.primitives.attributes.POSITION could not be found");
									}
								}
								else if (primitive_field.name == "indices") {

									expectJSONint64(primitive_field.value, indices);

									prim.indices = *indices;
								}
							}

							if (!attributes_found) {
								return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
									"required field meshes.primitives.attributes could not be found");
							}

							primitive_idx++;
						}
					}
				}

				if (!primitives_found) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
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

					gltf::Buffer& buff = gltf_struct.buffers[buffer_idx];

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
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
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

					gltf::BufferView& buff_view = gltf_struct.buffer_views[buffer_view_idx];

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
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field bufferViews.buffer could not be found");
				}
				if (!byte_length_found) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
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

					gltf::Accessor& acc = gltf_struct.accessors[accessor_idx];

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
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.componentType could not be found");
				}
				if (!count_found) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.count could not be found");
				}
				if (!type_found) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
						"required field accessors.type could not be found");
				}

				accessor_idx++;
			}
		}
	}

	return ErrorStack();
}

ErrorStack gltf::importMeshes(Path path, std::vector<LinkageMesh>& meshes)
{
	ErrorStack err;

	std::vector<char> text;
	err = path.read(text);

	if (err.isBad()) {
		return err;
	}

	// JSON Text to JSON Types
	JSONGraph json;
	uint64_t i = 0;

	err = parseJSON(text, 0, true, false, json);
	if (err.isBad()) {
		err.pushError(code_location, "failed to parse JSON");
		return err;
	}

	// JSON Types to C++ Types
	gltf::Structure gltf_struct;

	err = jsonToGLTF(json, gltf_struct);
	if (err.isBad()) {
		err.pushError(code_location, "failed to parse GLTF");
		return err;
	}

	// Convert URI to binary data
	std::vector<bin::Vector> bin_buffs;
	{
		bin_buffs.resize(gltf_struct.buffers.size());
		std::vector<char> char_uri;

		for (uint64_t i = 0; i < gltf_struct.buffers.size(); i++) {
			
			std::string& uri = gltf_struct.buffers[i].uri;
			char_uri.assign(uri.begin(), uri.end());

			bin::Vector& bin_buff = bin_buffs[i];

			err = bin::loadFromURI(char_uri, path, bin_buff);
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
		for (gltf::Mesh& gltf_mesh : gltf_struct.meshes) {
			for (gltf::Primitive& prim : gltf_mesh.primitives) {

				mesh_count++;
			}
		}
		meshes.resize(mesh_count);

		// Heap reuse
		VertexAtributes attrs;
		std::vector<uint32_t> indexes;

		uint64_t mesh_idx = 0;
		for (gltf::Mesh& gltf_mesh : gltf_struct.meshes) {
			for (gltf::Primitive& prim : gltf_mesh.primitives) {

				// Indexes
				{
					gltf::Accessor& acc = gltf_struct.accessors[prim.indices.value()];
					gltf::BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];

					uint64_t offset = buff_view.byte_offset + acc.byte_offset;

					if (acc.type != "SCALAR") {
						return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, 
							"expected indices accessor type to be SCALAR but instead got " + acc.type);
					}

					bin::Vector& bin_buff = bin_buffs[buff_view.buffer];

					err = loadIndexesFromBuffer(offset, acc.component_type, acc.count, bin_buff,
						indexes);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load indexes from buffer");
						return err;
					}
				}

				// Positions
				{
					uint64_t acc_idx = prim.atributes.at(atribute_name_position);

					gltf::Accessor& acc = gltf_struct.accessors[acc_idx];
					gltf::BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];

					uint64_t offset = buff_view.byte_offset + acc.byte_offset;

					if (acc.type != "VEC3") {
						return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
							"expected position atribute accessor type to be VEC3 but instead got " + acc.type);
					}

					bin::Vector& bin_buff = bin_buffs[buff_view.buffer];

					err = loadVec3FromBuffer(offset, acc.component_type, acc.count, bin_buff, attrs.positions);
					if (err.isBad()) {
						err.pushError(code_location, "failed to load positions from binary buffer");
						return err;
					}
				}

				// Normals
				auto normal_it = prim.atributes.find(atribute_name_normal);
				if (normal_it != prim.atributes.end()) {

					gltf::Accessor& acc = gltf_struct.accessors[normal_it->second];
					gltf::BufferView& buff_view = gltf_struct.buffer_views[acc.buffer_view.value()];


				}
				else {
					prim.atributes.clear();
				}

				// Create Linkage Mesh
				addTriangleListToMesh(meshes[mesh_idx], indexes, attrs, true);

				mesh_idx++;
			}
		}
	}

	return err;
}