
// Header
#include "GLTF_File.hpp"


using namespace gltf;

// TODO: replace these with exception functions

double expectJSON_Double(json::Field* field)
{
	double* dbl = std::get_if<double>(&field->value->value);
	if (dbl == nullptr) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type DOUBLE");
	}

	return *dbl;
}

double expectJSON_Double(json::Value* value)
{
	double* dbl = std::get_if<double>(&value->value);
	if (dbl == nullptr) {
		throw ErrStack(code_location,
			"expected JSON value to be of type DOUBLE");
	}

	return *dbl;
}

std::string expectJSON_String(json::Field* field)
{
	std::string* str = std::get_if<std::string>(&field->value->value);
	if (str == nullptr) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type STRING");
	}
	
	return *str;
}

std::vector<uint64_t> expectJSON_IntegerArray(json::Field* field)
{
	auto arr = std::get_if<std::vector<json::Value*>>(&field->value->value);
	if (arr == nullptr) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type ARRAY");
	}

	std::vector<uint64_t> ints(arr->size());
	uint32_t i = 0;
	for (uint64_t& integer : ints) {
		
		try {
			integer = (uint64_t)expectJSON_Double((*arr)[i++]);
		}
		catch (ErrStack& e) {
			e.pushError(code_location,
				"expected JSON ARRAY to be composed on of INT64s");
			throw e;
		}
	}

	return ints;
}

glm::mat4x4 expectJSON_Mat4x4(json::Field* field)
{
	auto arr = std::get_if<std::vector<json::Value*>>(&field->value->value);

	if (arr == nullptr) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type ARRAY");
	}

	if (arr->size() != 16) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type ARRAY of length 16");
	}

	glm::mat4x4 matrix;

	uint8_t i = 0;
	for (uint8_t row = 0; row < 4; row++) {
		for (uint8_t col = 0; col < 4; col++) {

			try {
				matrix[row][col] = (float)expectJSON_Double((*arr)[i++]);
			}
			catch (ErrStack& e) {
				e.pushError(code_location,
					"expected JSON ARRAY to be composed on of DOUBLEs");
				throw e;
			}
		}
	}

	return matrix;
}

std::vector<json::Value*>* expectJSON_Array(json::Field* field)
{
	auto arr = std::get_if<std::vector<json::Value*>>(&field->value->value);
	if (arr == nullptr) {
		throw ErrStack(code_location,
			"expected JSON field \"" + field->name + "\" to be of type ARRAY");
	}

	return arr;
}

//std::vector<json::Field>* expectJSON_Object()
//{
//
//}

#define expectJSONObject(val, obj) \
	std::vector<json::Field>* obj = std::get_if<std::vector<json::Field>>(&val->value); \
	if (obj == nullptr) { \
		return ErrStack(code_location, \
			"expected JSON field \"" + std::string(#obj) + "\" to be of OBJECT type"); \
	}


constexpr auto data_URI_prefix = "data:application/octet-stream;base64,";
std::string bin_suffix = ".bin";


bool hasEnding(std::string& str, std::string& ending) {
	if (str.length() >= ending.length()) {
		return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
	}
	return false;
}


ErrStack Structure::_loadIndexesFromBuffer(uint64_t acc_idx,
	std::vector<base64::BitVector>& bin_buffs, std::vector<uint32_t> & r_indexes)
{
	try {
		Accessor& acc = accessors[acc_idx];
		BufferView& buff_view = buffer_views[acc.buffer_view];
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
	}
	catch (...) {
		return ErrStack(code_location, "unknow exception occured while trying to load indexes from accessor");
	}

	return ErrStack();
}

ErrStack Structure::_loadVec3FromBuffer(uint64_t acc_idx,
	std::vector<base64::BitVector>& bin_buffs, std::vector<glm::vec3>& r_vecs)
{
	try {
		Accessor& acc = accessors[acc_idx];
		BufferView& buff_view = buffer_views[acc.buffer_view];
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
	}
	catch (...) {
		return ErrStack(code_location, "unknow exception occured while trying to load vec3 from accessor");
	}

	return ErrStack();
}

ErrStack Structure::importGLTF(io::FilePath& path_to_gltf_file)
{
	ErrStack err_stack;

	json::Graph json_graph;
	{
		std::vector<uint8_t> file;
		err_stack = path_to_gltf_file.read(file);
		if (err_stack.isBad()) {
			err_stack.pushError(code_location, "failed to read GLTF file at the level of bytes");
			return err_stack;
		}

		err_stack = json_graph.importJSON(file);
		if (err_stack.isBad()) {
			err_stack.pushError(code_location, "failed to read GLTF file at the level of JSON");
			return err_stack;
		}
	}
	
	json::Value& root = *json_graph.root;

	auto gltf_fields = std::get<std::vector<json::Field>>(root.value);
	if (gltf_fields.empty()) {
		return ErrStack(code_location, "GLTF has no fields");
	}

	try {
		for (json::Field& field : gltf_fields) {

			if (field.name == "asset") {

				try {
					expectJSONObject(field.value, asset_fields);

					for (json::Field& asset_field : *asset_fields) {

						if (asset_field.name == "version") {
							asset.version = expectJSON_String(&asset_field);
						}
						else if (asset_field.name == "generator") {
							asset.generator = expectJSON_String(&asset_field);
						}
						else if (asset_field.name == "copyright") {
							asset.copyright = expectJSON_String(&asset_field);;
						}
					}
				}
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"asset\"");
					throw e;
				}
			}
			else if (field.name == "scenes") {

				uint32_t scene_idx = 0;
				try {
					auto new_scenes = expectJSON_Array(&field);
					scenes.resize(new_scenes->size());

					for (json::Value* new_scene : *new_scenes) {

						Scene& scene = scenes[scene_idx];

						expectJSONObject(new_scene, scene_fields);
						for (json::Field& scene_field : *scene_fields) {

							// Scene Nodes
							if (scene_field.name == "nodes") {

								scene.nodes = expectJSON_IntegerArray(&scene_field);
							}
							// Scene Name
							else if (scene_field.name == "name") {
								scenes[scene_idx].name = expectJSON_String(&scene_field);;
							}
						}

						scene_idx++;
					}
				}
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"scenes\" of index " + std::to_string(scene_idx));
					throw e;
				}
			}
			else if (field.name == "nodes") {

				uint32_t nodes_idx = 0;
				try {
					auto new_nodes = expectJSON_Array(&field);
					nodes.resize(new_nodes->size());

					for (json::Value* new_node : *new_nodes) {

						Node& node = nodes[nodes_idx];

						expectJSONObject(new_node, node_fields);
						for (json::Field& node_field : *node_fields) {

							if (node_field.name == "mesh") {
								node.mesh = (uint64_t)expectJSON_Double(&node_field);
							}
							else if (node_field.name == "name") {
								node.name = expectJSON_String(&node_field);
							}
							else if (node_field.name == "matrix") {
								node.matrix = expectJSON_Mat4x4(&node_field);
								node.uses_matrix = true;
							}
						}
						nodes_idx++;
					}
				}
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"nodes\" of index " + std::to_string(nodes_idx));
					throw e;
				}
			}
			else if (field.name == "meshes") {

				uint32_t mesh_idx = 0;
				try {
					auto new_meshes = expectJSON_Array(&field);
					meshes.resize(new_meshes->size());

					for (json::Value* new_mesh : *new_meshes) {

						Mesh& mesh = meshes[mesh_idx];

						expectJSONObject(new_mesh, mesh_fields);
						for (json::Field& mesh_field : *mesh_fields) {

							if (mesh_field.name == "name") {
								mesh.name = expectJSON_String(&mesh_field);
							}
							else if (mesh_field.name == "primitives") {

								auto primitives = expectJSON_Array(&mesh_field);
								mesh.primitives.resize(primitives->size());

								uint32_t primitive_idx = 0;
								for (json::Value* primitive : *primitives) {

									Primitive& prim = mesh.primitives[primitive_idx];

									expectJSONObject(primitive, primitive_fields);
									for (json::Field& primitive_field : *primitive_fields) {

										// Primitive Indices
										if (primitive_field.name == "indices") {
											prim.indices = (uint64_t)expectJSON_Double(&primitive_field);
										}
										// Primitive Atributes
										else if (primitive_field.name == "attributes") {

											expectJSONObject(primitive_field.value, atributes);
											for (json::Field& atributes_field : *atributes) {

												prim.attributes.insert(std::pair<std::string, uint64_t>(
													atributes_field.name,
													(uint64_t)expectJSON_Double(&atributes_field)));
											}

											if (prim.attributes.find(Atributes::position) == prim.attributes.end()) {
												return ErrStack(code_location,
													"required field meshes.primitives.attributes.POSITION could not be found on mesh " + mesh.name);
											}
										}
										// Primitive Mode
										else if (primitive_field.name == "mode") {
											prim.mode = (uint64_t)expectJSON_Double(&primitive_field);
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
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"meshes\" of index " + std::to_string(mesh_idx));
					throw e;
				}
			}
			else if (field.name == "buffers") {

				uint32_t buffer_idx = 0;
				try {
					auto new_buffers = expectJSON_Array(&field);
					buffers.resize(new_buffers->size());

					for (json::Value* new_buffer : *new_buffers) {

						Buffer& buff = buffers[buffer_idx];
						bool byte_length_found = false;

						expectJSONObject(new_buffer, buffer_fields);
						for (json::Field& buffer_field : *buffer_fields) {

							if (buffer_field.name == "name") {
								buff.name = expectJSON_String(&buffer_field);
							}
							else if (buffer_field.name == "uri") {
								buff.uri = expectJSON_String(&buffer_field);
							}
							else if (buffer_field.name == "byteLength") {
								buff.byte_length = (uint64_t)expectJSON_Double(&buffer_field);
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
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"buffers\" of index " + std::to_string(buffer_idx));
					throw e;
				}
			}
			else if (field.name == "bufferViews") {

				uint32_t buffer_view_idx = 0;
				try {
					auto new_buffer_views = expectJSON_Array(&field);
					buffer_views.resize(new_buffer_views->size());

					for (json::Value* new_buffer_view : *new_buffer_views) {

						BufferView& buff_view = buffer_views[buffer_view_idx];

						bool buffer_found = false;
						bool byte_length_found = false;

						expectJSONObject(new_buffer_view, buffer_view_fields);
						for (json::Field& buffer_view_field : *buffer_view_fields) {

							if (buffer_view_field.name == "name") {
								buff_view.name = expectJSON_String(&buffer_view_field);
							}
							else if (buffer_view_field.name == "buffer") {
								buff_view.buffer = (uint64_t)expectJSON_Double(&buffer_view_field);
								buffer_found = true;
							}
							else if (buffer_view_field.name == "byteOffset") {
								buff_view.byte_offset = (uint64_t)expectJSON_Double(&buffer_view_field);
							}
							else if (buffer_view_field.name == "byteLength") {
								buff_view.byte_length = (uint64_t)expectJSON_Double(&buffer_view_field);;
								byte_length_found = true;
							}
							else if (buffer_view_field.name == "target") {
								buff_view.target = (uint64_t)expectJSON_Double(&buffer_view_field);;
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
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"bufferViews\" of index " + std::to_string(buffer_view_idx));
					throw e;
				}
			}
			else if (field.name == "accessors") {
				
				uint32_t accessor_idx = 0;
				try {
					auto new_accessors = expectJSON_Array(&field);
					accessors.resize(new_accessors->size());

					for (json::Value* new_accessor : *new_accessors) {

						Accessor& acc = accessors[accessor_idx];

						bool component_type_found = false;
						bool count_found = false;
						bool type_found = false;

						expectJSONObject(new_accessor, accessor_fields);
						for (json::Field& accessor_field : *accessor_fields) {

							if (accessor_field.name == "name") {
								acc.name = expectJSON_String(&accessor_field);
							}
							else if (accessor_field.name == "bufferView") {
								acc.buffer_view = (uint64_t)expectJSON_Double(&accessor_field);
							}
							else if (accessor_field.name == "byteOffset") {
								acc.byte_offset = (uint64_t)expectJSON_Double(&accessor_field);
							}
							else if (accessor_field.name == "componentType") {
								acc.component_type = (uint64_t)expectJSON_Double(&accessor_field);

								component_type_found = true;
							}
							else if (accessor_field.name == "count") {
								acc.count = (uint64_t)expectJSON_Double(&accessor_field);

								count_found = true;
							}
							else if (accessor_field.name == "type") {
								acc.type = expectJSON_String(&accessor_field);

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
				catch (ErrStack& e) {
					e.pushError(code_location, "failed to parse GLTF field \"accessors\" of index " + std::to_string(accessor_idx));
					throw e;
				}
			}
		}

	}
	catch (ErrStack& e) {
		e.pushError(code_location, "failed to parse malformed GLTF file, check exception error");
		return e;
	}
	catch (...) {
		return ErrStack(code_location, "failed to parse GLTF file, unknown exception occured");
	}

	// Convert URI to buffer data
	std::vector<base64::BitVector> bin_buffs;
	{
		bin_buffs.resize(buffers.size());
		io::FilePath relative_uri_root = path_to_gltf_file;
		relative_uri_root.pop_back();

		for (uint64_t i = 0; i < buffers.size(); i++) {

			Buffer& buffer = buffers[i];
			try {
				std::string& uri = buffer.uri;

				base64::BitVector& bin_buff = bin_buffs[i];
				bin_buff.bytes.reserve(buffer.byte_length);

				// Data URI
				size_t uri_i = uri.find(data_URI_prefix);
				if (uri_i != std::string::npos) {

					uri_i += 37;  // TODO: test for '\0'

					// decode rest of URI as Base64 binary
					for (; uri_i < uri.size(); uri_i++) {

						char b64 = uri[uri_i];

						// ignore invisible characters
						if (b64 >= 0x21) {
							bin_buff.pushBase64Char(b64);
						}
					}
				}
				// Relative URI
				else if (hasEnding(uri, bin_suffix)) {

					io::FilePath file_path = relative_uri_root;
					file_path.push_back(uri);

					std::vector<uint8_t> bin;

					err_stack = file_path.read(bin);
					if (err_stack.isBad()) {
						return ErrStack(code_location, "failed to read binary file referenced by URI");
					}

					bin_buff.bytes.resize(bin.size());
					memcpy(bin_buff.bytes.data(), bin.data(), bin_buff.bytes.size());
				}
				else {
					return ErrStack(code_location,
						"invalid URI for buffer " + buffer.name + " , buffer index " + std::to_string(i));
				}
			}
			catch (...) {
				return ErrStack(code_location,
					"unknown exception occured when trying to extract data from buffer " + buffer.name + " , buffer index " + std::to_string(i));
			}
		}
	}

	// Extract meaningfull data
	{
		uint64_t mesh_idx = 0;
		for (Mesh& mesh : meshes) {

			for (Primitive& prim : mesh.primitives) {
				if (prim.mode == Modes::TRIANGLES) {

					// Indexes
					checkErrStack(_loadIndexesFromBuffer(prim.indices, bin_buffs, prim.indexes),
						"failed to load indexes from buffer for mesh " + mesh.name + ", mesh index " + std::to_string(mesh_idx));

					// Positions
					{
						uint64_t acc_idx = prim.attributes.at(Atributes::position);

						checkErrStack(_loadVec3FromBuffer(acc_idx, bin_buffs, prim.positions),
							"failed to load positions from buffer for mesh " + mesh.name + ", mesh index " + std::to_string(mesh_idx))
					}

					// Normals
					auto normal_it = prim.attributes.find(Atributes::normal);
					if (normal_it != prim.attributes.end()) {

						checkErrStack(_loadVec3FromBuffer(normal_it->second, bin_buffs, prim.normals),
							"failed to load normals from buffer for mesh " + mesh.name + ", mesh index " + std::to_string(mesh_idx))
					}
				}
			}

			mesh_idx++;
		}
	}

	return err_stack;
}
