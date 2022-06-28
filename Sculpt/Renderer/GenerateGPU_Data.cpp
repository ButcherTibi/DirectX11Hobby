#include "./Renderer.hpp"

#include <App/Application.hpp>


void MeshRenderer::loadVertices()
{
	for (Mesh& mesh : app.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		if (sculpt_mesh.verts.size() == 0) {
			continue;
		}

		auto load_aabbs = [&]() {

			mesh.prev_aabb_mode = mesh.aabb_render_mode;

			uint32_t aabb_count = 0;
			for (scme::VertexBoundingBox& aabb : sculpt_mesh.aabbs) {

				switch (mesh.aabb_render_mode) {
				case AABB_RenderMode::LEAF_ONLY: {
					if (aabb.isLeaf() && aabb.hasVertices()) {
						aabb_count++;
					}
					break;
				}
				case AABB_RenderMode::NORMAL: {
					if (aabb.isLeaf()) {
						aabb_count++;
					}
					break;
				}
				}
			}

			auto& aabb_verts = sculpt_mesh.aabb_verts;
			aabb_verts.resize(aabb_count * 36 + 3);

			// AABB rendering uses the Vertex shader in which the 0 index is discarded
			// with non-indexing drawing
			{
				auto& second = aabb_verts[1];
				second.pos = { 0, 0, 0 };

				auto& third = aabb_verts[2];
				third.pos = { 0, 0, 0 };
			}

			uint32_t vertex_idx = 3;
			for (scme::VertexBoundingBox& aabb : sculpt_mesh.aabbs) {

				switch (mesh.aabb_render_mode) {
				case AABB_RenderMode::LEAF_ONLY: {
					if (aabb.isLeaf() == false || aabb.hasVertices() == false) {
						continue;
					}
					break;
				}
				case AABB_RenderMode::NORMAL: {
					if (aabb.isLeaf() == false) {
						continue;
					}
					break;
				}
				}

				glm::vec3& min = aabb.aabb.min;
				glm::vec3& max = aabb.aabb.max;

				// Forward (Classic winding)
				DirectX::XMFLOAT3 gpu_aabbs_positions[8];
				gpu_aabbs_positions[0] = dxConvert(min.x, max.y, max.z);  // top left
				gpu_aabbs_positions[1] = dxConvert(max.x, max.y, max.z);  // top right
				gpu_aabbs_positions[2] = dxConvert(max.x, min.y, max.z);  // bot right
				gpu_aabbs_positions[3] = dxConvert(min.x, min.y, max.z);  // bot left

				// Backward
				gpu_aabbs_positions[4] = dxConvert(min.x, max.y, min.z);  // top left
				gpu_aabbs_positions[5] = dxConvert(max.x, max.y, min.z);  // top right
				gpu_aabbs_positions[6] = dxConvert(max.x, min.y, min.z);  // bot right
				gpu_aabbs_positions[7] = dxConvert(min.x, min.y, min.z);  // bot left

				// Front Face
				aabb_verts[vertex_idx + 0].pos = gpu_aabbs_positions[0];
				aabb_verts[vertex_idx + 1].pos = gpu_aabbs_positions[1];
				aabb_verts[vertex_idx + 2].pos = gpu_aabbs_positions[3];

				aabb_verts[vertex_idx + 3].pos = gpu_aabbs_positions[1];
				aabb_verts[vertex_idx + 4].pos = gpu_aabbs_positions[2];
				aabb_verts[vertex_idx + 5].pos = gpu_aabbs_positions[3];

				// Right Face
				aabb_verts[vertex_idx + 6].pos = gpu_aabbs_positions[1];
				aabb_verts[vertex_idx + 7].pos = gpu_aabbs_positions[5];
				aabb_verts[vertex_idx + 8].pos = gpu_aabbs_positions[2];

				aabb_verts[vertex_idx + 9].pos = gpu_aabbs_positions[5];
				aabb_verts[vertex_idx + 10].pos = gpu_aabbs_positions[6];
				aabb_verts[vertex_idx + 11].pos = gpu_aabbs_positions[2];

				// Back Face
				aabb_verts[vertex_idx + 12].pos = gpu_aabbs_positions[5];
				aabb_verts[vertex_idx + 13].pos = gpu_aabbs_positions[4];
				aabb_verts[vertex_idx + 14].pos = gpu_aabbs_positions[6];

				aabb_verts[vertex_idx + 15].pos = gpu_aabbs_positions[4];
				aabb_verts[vertex_idx + 16].pos = gpu_aabbs_positions[7];
				aabb_verts[vertex_idx + 17].pos = gpu_aabbs_positions[6];

				// Left Face
				aabb_verts[vertex_idx + 18].pos = gpu_aabbs_positions[4];
				aabb_verts[vertex_idx + 19].pos = gpu_aabbs_positions[0];
				aabb_verts[vertex_idx + 20].pos = gpu_aabbs_positions[7];

				aabb_verts[vertex_idx + 21].pos = gpu_aabbs_positions[0];
				aabb_verts[vertex_idx + 22].pos = gpu_aabbs_positions[3];
				aabb_verts[vertex_idx + 23].pos = gpu_aabbs_positions[7];

				// Top Face	
				aabb_verts[vertex_idx + 24].pos = gpu_aabbs_positions[4];
				aabb_verts[vertex_idx + 25].pos = gpu_aabbs_positions[5];
				aabb_verts[vertex_idx + 26].pos = gpu_aabbs_positions[0];

				aabb_verts[vertex_idx + 27].pos = gpu_aabbs_positions[5];
				aabb_verts[vertex_idx + 28].pos = gpu_aabbs_positions[1];
				aabb_verts[vertex_idx + 29].pos = gpu_aabbs_positions[0];

				// Back Face
				aabb_verts[vertex_idx + 30].pos = gpu_aabbs_positions[6];
				aabb_verts[vertex_idx + 31].pos = gpu_aabbs_positions[7];
				aabb_verts[vertex_idx + 32].pos = gpu_aabbs_positions[2];

				aabb_verts[vertex_idx + 33].pos = gpu_aabbs_positions[7];
				aabb_verts[vertex_idx + 34].pos = gpu_aabbs_positions[3];
				aabb_verts[vertex_idx + 35].pos = gpu_aabbs_positions[2];

				// Front Face
				aabb_verts[vertex_idx + 1].normal.x = 1;
				aabb_verts[vertex_idx + 2].normal.x = 1;

				aabb_verts[vertex_idx + 3].normal.x = 1;
				aabb_verts[vertex_idx + 5].normal.x = 1;

				// Right Face
				aabb_verts[vertex_idx + 7].normal.x = 1;
				aabb_verts[vertex_idx + 8].normal.x = 1;

				aabb_verts[vertex_idx + 9].normal.x = 1;
				aabb_verts[vertex_idx + 11].normal.x = 1;

				// Back Face
				aabb_verts[vertex_idx + 13].normal.x = 1;
				aabb_verts[vertex_idx + 14].normal.x = 1;

				aabb_verts[vertex_idx + 15].normal.x = 1;
				aabb_verts[vertex_idx + 17].normal.x = 1;

				// Left Face
				aabb_verts[vertex_idx + 19].normal.x = 1;
				aabb_verts[vertex_idx + 20].normal.x = 1;

				aabb_verts[vertex_idx + 21].normal.x = 1;
				aabb_verts[vertex_idx + 23].normal.x = 1;

				// Top Face
				aabb_verts[vertex_idx + 25].normal.x = 1;
				aabb_verts[vertex_idx + 26].normal.x = 1;

				aabb_verts[vertex_idx + 27].normal.x = 1;
				aabb_verts[vertex_idx + 29].normal.x = 1;

				// Bot Face
				aabb_verts[vertex_idx + 31].normal.x = 1;
				aabb_verts[vertex_idx + 32].normal.x = 1;

				aabb_verts[vertex_idx + 27].normal.x = 1;
				aabb_verts[vertex_idx + 35].normal.x = 1;

				// normal.xy is unused

				vertex_idx += 36;
			}

			sculpt_mesh.gpu_aabb_verts.upload(aabb_verts);
		};

		// Update AABBs (does not depend GPU data being updated)
		if (mesh.aabb_render_mode != AABB_RenderMode::NO_RENDER)
		{
			// redo render mode changed
			if (mesh.aabb_render_mode != mesh.prev_aabb_mode) {

				mesh.prev_aabb_mode = mesh.aabb_render_mode;
				load_aabbs();
			}
			// positions updated
			else if (sculpt_mesh.modified_verts.size()) {
				load_aabbs();
			}
			// no data so load
			else if (sculpt_mesh.gpu_aabb_verts.count() == 0) {
				load_aabbs();
			}
		}
		// Free GPU Memory
		else if (mesh.aabb_render_mode == AABB_RenderMode::NO_RENDER && sculpt_mesh.gpu_aabb_verts.count() != 0) {
			sculpt_mesh.gpu_aabb_verts.deallocate();

			mesh.prev_aabb_mode = mesh.aabb_render_mode;
		}

		// Figure out what to update
		bool dirty_vertex_list = false;
		bool dirty_vertex_pos = false;
		bool dirty_index_buff = false;
		bool dirty_tess_tris = false;
		bool dirty_instance = false;
		bool dirty_debug = false;
		{
			if (sculpt_mesh.verts.size() != sculpt_mesh.gpu_verts.count() - 1) {
				dirty_vertex_list = true;
			}

			for (scme::ModifiedVertex& modified_v : sculpt_mesh.modified_verts) {

				switch (modified_v.state) {
				case scme::ModifiedVertexState::UPDATE: {
					dirty_vertex_pos = true;
					dirty_tess_tris = true;
					break;
				}
				case scme::ModifiedVertexState::DELETED: {
					dirty_vertex_list = true;
				}
				}
			}

			for (scme::ModifiedPoly& modified_p : sculpt_mesh.modified_polys) {

				switch (modified_p.state) {
				case scme::ModifiedPolyState::UPDATE: {
					dirty_index_buff = true;
					dirty_tess_tris = true;
					break;
				}
				case scme::ModifiedPolyState::DELETED: {
					dirty_index_buff = true;
				}
				}
			}

			/*for (auto& modified_instance : sculpt_mesh.modified_instances) {

				switch (modified_instance.state) {
				case scme::ModifiedInstanceType::UPDATE: {
					dirty_instance = true;
					break;
				}
				case scme::ModifiedInstanceType::DELETED: {
					dirty_instance = true;
				}
				}
			}*/

			if (dirty_vertex_list || dirty_vertex_pos ||
				dirty_index_buff || dirty_tess_tris ||
				dirty_instance)
			{
				dirty_debug = true;
			}
		}

		// Update Mesh Data
		{
			if (dirty_vertex_list) {
				sculpt_mesh.uploadVertexAddsRemoves();
			}

			if (dirty_vertex_pos) {
				sculpt_mesh.uploadVertexPositions();
			}

			if (dirty_index_buff) {
				sculpt_mesh.uploadIndexBufferChanges();
			}

			if (dirty_tess_tris) {
				sculpt_mesh.uploadTesselationTriangles(scme::TesselationModificationBasis::MODIFIED_POLYS);
			}

			if (dirty_vertex_pos && app.shading_normal == GPU_ShadingNormal::VERTEX) {
				sculpt_mesh.uploadVertexNormals();
			}

			if (sculpt_mesh.modified_verts.size() > 0) {

				for (scme::ModifiedVertex& modified_v : sculpt_mesh.modified_verts) {

					switch (modified_v.state) {
					case scme::ModifiedVertexState::UPDATE: {

						// it is posible to have the vertex as both deleted and updated multiple times
						// when running multiple sculpt mesh operations
						if (sculpt_mesh.verts.isDeleted(modified_v.idx) == false) {

							sculpt_mesh.moveVertexInAABBs(modified_v.idx);
						}
						break;
					}
					}
				}
			}

			sculpt_mesh.modified_verts.clear();
			sculpt_mesh.modified_polys.clear();
		}

		// Instances
		{
			for (MeshInstanceSet& set : mesh.sets) {

				if (set.modified_instances.size()) {

					auto& gpu_instances = set.gpu_instances;

					if (gpu_instances.buff == nullptr) {
						gpu_instances.dev = dev.Get();
						gpu_instances.ctx3 = im_ctx.Get();
						gpu_instances.init_desc = {};
						gpu_instances.init_desc.Usage = D3D11_USAGE_DEFAULT;
						gpu_instances.init_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						gpu_instances.init_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
						gpu_instances.init_desc.StructureByteStride = sizeof(GPU_MeshInstance);
					}

					gpu_instances.resize(set.instances.capacity());

					for (ModifiedMeshInstance& modified_instance : set.modified_instances) {

						GPU_MeshInstance gpu_inst;

						switch (modified_instance.type) {
						case ModifiedInstanceType::UPDATE: {

							if (set.instances.isDeleted(modified_instance.idx) == false) {

								MeshInstance& instance = set.instances[modified_instance.idx];
								gpu_inst.pos = dxConvert(instance.transform.pos);
								gpu_inst.rot = dxConvert(instance.transform.rot);

								PhysicalBasedMaterial& pbr_mat = instance.pbr_material;
								gpu_inst.albedo_color = dxConvert(pbr_mat.albedo_color);
								gpu_inst.roughness = glm::clamp(pbr_mat.roughness, 0.05f, 1.f);
								gpu_inst.metallic = pbr_mat.metallic;
								gpu_inst.specular = pbr_mat.specular;

								MeshWireframeColors wire_colors = instance.wireframe_colors;
								gpu_inst.wireframe_front_color = dxConvert(wire_colors.front_color);
								gpu_inst.wireframe_back_color = dxConvert(wire_colors.back_color);
								gpu_inst.wireframe_tess_front_color = dxConvert(wire_colors.tesselation_front_color);
								gpu_inst.wireframe_tess_back_color = dxConvert(wire_colors.tesselation_back_color);
								gpu_inst.wireframe_tess_split_count = wire_colors.tesselation_split_count;
								gpu_inst.wireframe_tess_gap = wire_colors.tesselation_gap;

								gpu_instances.upload(modified_instance.idx, gpu_inst);
							}
							break;
						}

						case ModifiedInstanceType::DELETED: {
							gpu_inst.rot.w = 2.f;
							gpu_instances.upload(modified_instance.idx, gpu_inst);
							break;
						}
						}
					}

					set.modified_instances.clear();

					// Shader Resource View
					set.gpu_instances_srv = nullptr;

					D3D11_SHADER_RESOURCE_VIEW_DESC desc;
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
					desc.Buffer.ElementOffset = 0;
					desc.Buffer.ElementWidth = sizeof(GPU_MeshInstance);
					desc.Buffer.NumElements = set.gpu_instances.capacity();

					throwDX11(dev->CreateShaderResourceView(
						set.gpu_instances.get(), &desc, set.gpu_instances_srv.GetAddressOf()));
				}
			}
		}
	}
}