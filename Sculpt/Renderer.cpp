
// Header
#include "Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"


void MeshRenderer::loadVertices()
{
	for (Mesh& mesh : application.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Vertices
		{
			auto& gpu_verts = sculpt_mesh.gpu_verts;

			if (sculpt_mesh.modified_verts.size() > 0) {

				if (gpu_verts.buff == nullptr) {
					gpu_verts.dev = this->dev5;
					gpu_verts.ctx3 = this->im_ctx3;
					gpu_verts.init_desc = {};
					gpu_verts.init_desc.Usage = D3D11_USAGE_DEFAULT;
					gpu_verts.init_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				}

				gpu_verts.resize(sculpt_mesh.verts.capacity());

				for (scme::ModifiedVertex& modified_v : sculpt_mesh.modified_verts) {

					switch (modified_v.state) {
					case scme::ModifiedVertexState::UPDATE: {
						scme::Vertex& cpu_v = sculpt_mesh.verts[modified_v.idx];

						GPU_MeshVertex gpu_v;
						gpu_v.pos = dxConvert(cpu_v.pos);
						gpu_v.normal = dxConvert(cpu_v.normal);

						gpu_verts.update(modified_v.idx, gpu_v);
						break;
					}

					case scme::ModifiedVertexState::DELETED: {
						GPU_MeshVertex gpu_v;
						gpu_v.normal.x = 999'999.f;

						gpu_verts.update(modified_v.idx, gpu_v);
						break;
					}
					}
				}

				sculpt_mesh.modified_verts.clear();
			}
		}

		// Polygons
		{
			if (sculpt_mesh.modified_polys.size() > 0) {

				// Indexes

				auto& gpu_indexs = sculpt_mesh.gpu_indexes;

				if (gpu_indexs.buff == nullptr) {
					gpu_indexs.dev = this->dev5;
					gpu_indexs.ctx3 = this->im_ctx3;
					gpu_indexs.init_desc = {};
					gpu_indexs.init_desc.Usage = D3D11_USAGE_DEFAULT;
					gpu_indexs.init_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				}

				// regardless if a poly is tris or quad, always load 6 indexes
				gpu_indexs.resize(sculpt_mesh.polys.capacity() * 6);

				for (scme::ModifiedPoly& modified_poly : sculpt_mesh.modified_polys) {

					scme::Poly& poly = sculpt_mesh.polys[modified_poly.idx];

					switch (modified_poly.state) {
					case scme::ModifiedPolyState::UPDATE: {

						scme::Edge& edge_0 = sculpt_mesh.edges[poly.edges[0]];
						scme::Edge& edge_1 = sculpt_mesh.edges[poly.edges[1]];
						scme::Edge& edge_2 = sculpt_mesh.edges[poly.edges[2]];

						uint32_t v0_idx = poly.flip_edge_0 ? edge_0.v1 : edge_0.v0;
						uint32_t v1_idx = poly.flip_edge_1 ? edge_1.v1 : edge_1.v0;
						uint32_t v2_idx = poly.flip_edge_2 ? edge_2.v1 : edge_2.v0;

						if (poly.is_tris) {

							gpu_indexs.update(6 * modified_poly.idx + 0, v0_idx);
							gpu_indexs.update(6 * modified_poly.idx + 1, v1_idx);
							gpu_indexs.update(6 * modified_poly.idx + 2, v2_idx);

							uint32_t zero = 0;
							gpu_indexs.update(6 * modified_poly.idx + 3, zero);
							gpu_indexs.update(6 * modified_poly.idx + 4, zero);
							gpu_indexs.update(6 * modified_poly.idx + 5, zero);
						}
						else {
							scme::Edge& edge_3 = sculpt_mesh.edges[poly.edges[3]];
							uint32_t v3_idx = poly.flip_edge_3 ? edge_3.v1 : edge_3.v0;

							if (poly.tesselation_type == 0) {

								gpu_indexs.update(6 * modified_poly.idx + 0, v0_idx);
								gpu_indexs.update(6 * modified_poly.idx + 1, v2_idx);
								gpu_indexs.update(6 * modified_poly.idx + 2, v3_idx);

								gpu_indexs.update(6 * modified_poly.idx + 3, v0_idx);
								gpu_indexs.update(6 * modified_poly.idx + 4, v1_idx);
								gpu_indexs.update(6 * modified_poly.idx + 5, v2_idx);
							}
							else {
								gpu_indexs.update(6 * modified_poly.idx + 0, v0_idx);
								gpu_indexs.update(6 * modified_poly.idx + 1, v1_idx);
								gpu_indexs.update(6 * modified_poly.idx + 2, v3_idx);

								gpu_indexs.update(6 * modified_poly.idx + 3, v1_idx);
								gpu_indexs.update(6 * modified_poly.idx + 4, v2_idx);
								gpu_indexs.update(6 * modified_poly.idx + 5, v3_idx);
							}
						}
						break;
					}

					case scme::ModifiedPolyState::DELETED: {

						uint32_t zero = 0;
						gpu_indexs.update(6 * modified_poly.idx + 0, zero);
						gpu_indexs.update(6 * modified_poly.idx + 1, zero);
						gpu_indexs.update(6 * modified_poly.idx + 2, zero);

						gpu_indexs.update(6 * modified_poly.idx + 3, zero);
						gpu_indexs.update(6 * modified_poly.idx + 4, zero);
						gpu_indexs.update(6 * modified_poly.idx + 5, zero);
						break;
					}
					}
				}

				// Triangles
				if (application.shading_normal != ShadingNormal::VERTEX) {

					auto& gpu_triangles = sculpt_mesh.gpu_triangles;

					if (gpu_triangles.buff == nullptr) {
						gpu_triangles.dev = this->dev5;
						gpu_triangles.ctx3 = this->im_ctx3;
						gpu_triangles.init_desc = {};
						gpu_triangles.init_desc.Usage = D3D11_USAGE_DEFAULT;
						gpu_triangles.init_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						gpu_triangles.init_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
						gpu_triangles.init_desc.StructureByteStride = sizeof(GPU_MeshTriangle);
					}

					// regardless if a poly is tris or quad, always load 2 triangles
					gpu_triangles.resize(sculpt_mesh.polys.capacity() * 2);

					switch (application.shading_normal) {
					case ShadingNormal::POLY: {

						for (scme::ModifiedPoly& modified_poly : sculpt_mesh.modified_polys) {

							scme::Poly& poly = sculpt_mesh.polys[modified_poly.idx];

							GPU_MeshTriangle tris;
							tris.normal = dxConvert(poly.normal);

							gpu_triangles.update(2 * modified_poly.idx, tris);

							if (poly.is_tris == false) {
								gpu_triangles.update(2 * modified_poly.idx + 1, tris);
							}
						}

						break;
					}
					case ShadingNormal::TESSELATION: {

						for (scme::ModifiedPoly& modified_poly : sculpt_mesh.modified_polys) {

							scme::Poly& poly = sculpt_mesh.polys[modified_poly.idx];

							GPU_MeshTriangle tris;

							if (poly.is_tris) {
								tris.normal = dxConvert(poly.normal);
								gpu_triangles.update(2 * modified_poly.idx, tris);
							}
							else {
								tris.normal = dxConvert(poly.tess_normals[0]);
								gpu_triangles.update(2 * modified_poly.idx, tris);

								tris.normal = dxConvert(poly.tess_normals[1]);
								gpu_triangles.update(2 * modified_poly.idx + 1, tris);
							}
						}

						break;
					}
					}

					sculpt_mesh.modified_polys.clear();

					if (sculpt_mesh.gpu_triangles_srv == nullptr) {

						D3D11_SHADER_RESOURCE_VIEW_DESC desc;
						desc.Format = DXGI_FORMAT_UNKNOWN;
						desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
						desc.Buffer.ElementOffset = 0;
						desc.Buffer.ElementWidth = sizeof(GPU_MeshTriangle);
						desc.Buffer.NumElements = sculpt_mesh.gpu_triangles.size();  // this may not be correct

						throwDX11(dev5->CreateShaderResourceView(
							sculpt_mesh.gpu_triangles.get(), &desc, sculpt_mesh.gpu_triangles_srv.GetAddressOf()));
					}
				}
			}
		}

		// Instances
		{
			for (MeshInstanceSet& set : mesh.sets) {

				auto& gpu_instances = set.gpu_instances;

				if (gpu_instances.buff == nullptr) {
					gpu_instances.dev = this->dev5;
					gpu_instances.ctx3 = this->im_ctx3;
					gpu_instances.init_desc = {};
					gpu_instances.init_desc.Usage = D3D11_USAGE_DEFAULT;
					gpu_instances.init_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				}

				gpu_instances.resize(set.instances.capacity());

				for (ModifiedMeshInstance& modified_instance : set.modified_instances) {
					
					MeshInstance& instance = set.instances[modified_instance.idx];

					GPU_MeshInstance gpu_inst;
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

					gpu_instances.update(modified_instance.idx, gpu_inst);
				}

				set.modified_instances.clear();
			}
		}
	}
}


//						glm::vec3& min = octree.aabb.min;
//						glm::vec3& max = octree.aabb.max;
//
//						// Forward (Classic winding)
//						gpu_aabbs_positions[0] = dxConvert(glm::vec3{ min.x, max.y, max.z });  // top left
//						gpu_aabbs_positions[1] = dxConvert(glm::vec3{ max.x, max.y, max.z });  // top right
//						gpu_aabbs_positions[2] = dxConvert(glm::vec3{ max.x, min.y, max.z });  // bot right
//						gpu_aabbs_positions[3] = dxConvert(glm::vec3{ min.x, min.y, max.z });  // bot left
//
//						// Backward
//						gpu_aabbs_positions[4] = dxConvert(glm::vec3{ min.x, max.y, min.z });  // top left
//						gpu_aabbs_positions[5] = dxConvert(glm::vec3{ max.x, max.y, min.z });  // top right
//						gpu_aabbs_positions[6] = dxConvert(glm::vec3{ max.x, min.y, min.z });  // bot right
//						gpu_aabbs_positions[7] = dxConvert(glm::vec3{ min.x, min.y, min.z });  // bot left
//
//						// Front Face
//						gpu_verts[0].pos = gpu_aabbs_positions[0];
//						gpu_verts[1].pos = gpu_aabbs_positions[1];
//						gpu_verts[2].pos = gpu_aabbs_positions[3];
//
//						gpu_verts[3].pos = gpu_aabbs_positions[1];
//						gpu_verts[4].pos = gpu_aabbs_positions[2];
//						gpu_verts[5].pos = gpu_aabbs_positions[3];
//
//						// Right Face
//						gpu_verts[6].pos = gpu_aabbs_positions[1];
//						gpu_verts[7].pos = gpu_aabbs_positions[5];
//						gpu_verts[8].pos = gpu_aabbs_positions[2];
//
//						gpu_verts[9].pos = gpu_aabbs_positions[5];
//						gpu_verts[10].pos = gpu_aabbs_positions[6];
//						gpu_verts[11].pos = gpu_aabbs_positions[2];
//
//						// Back Face
//						gpu_verts[12].pos = gpu_aabbs_positions[5];
//						gpu_verts[13].pos = gpu_aabbs_positions[4];
//						gpu_verts[14].pos = gpu_aabbs_positions[6];
//
//						gpu_verts[15].pos = gpu_aabbs_positions[4];
//						gpu_verts[16].pos = gpu_aabbs_positions[7];
//						gpu_verts[17].pos = gpu_aabbs_positions[6];
//
//						// Left Face
//						gpu_verts[18].pos = gpu_aabbs_positions[4];
//						gpu_verts[19].pos = gpu_aabbs_positions[0];
//						gpu_verts[20].pos = gpu_aabbs_positions[7];
//
//						gpu_verts[21].pos = gpu_aabbs_positions[0];
//						gpu_verts[22].pos = gpu_aabbs_positions[3];
//						gpu_verts[23].pos = gpu_aabbs_positions[7];
//
//						// Top Face
//						gpu_verts[24].pos = gpu_aabbs_positions[4];
//						gpu_verts[25].pos = gpu_aabbs_positions[5];
//						gpu_verts[26].pos = gpu_aabbs_positions[0];
//
//						gpu_verts[27].pos = gpu_aabbs_positions[5];
//						gpu_verts[28].pos = gpu_aabbs_positions[1];
//						gpu_verts[29].pos = gpu_aabbs_positions[0];
//
//						// Back Face
//						gpu_verts[30].pos = gpu_aabbs_positions[6];
//						gpu_verts[31].pos = gpu_aabbs_positions[7];
//						gpu_verts[32].pos = gpu_aabbs_positions[2];
//
//						gpu_verts[33].pos = gpu_aabbs_positions[7];
//						gpu_verts[34].pos = gpu_aabbs_positions[3];
//						gpu_verts[35].pos = gpu_aabbs_positions[2];
//
//						if (octree._debug_show_tesselation) {
//							for (GPU_MeshVertex& gpu_vert : gpu_verts) {
//								gpu_vert.tess_edge = 0;
//							}
//						}
//						else {
//							// Front Face
//							gpu_verts[1].tess_edge = 1;
//							gpu_verts[2].tess_edge = 1;
//
//							gpu_verts[3].tess_edge = 1;
//							gpu_verts[5].tess_edge = 1;
//
//							// Right Face
//							gpu_verts[7].tess_edge = 1;
//							gpu_verts[8].tess_edge = 1;
//
//							gpu_verts[9].tess_edge = 1;
//							gpu_verts[11].tess_edge = 1;
//
//							// Back Face
//							gpu_verts[13].tess_edge = 1;
//							gpu_verts[14].tess_edge = 1;
//
//							gpu_verts[15].tess_edge = 1;
//							gpu_verts[17].tess_edge = 1;
//
//							// Left Face
//							gpu_verts[19].tess_edge = 1;
//							gpu_verts[20].tess_edge = 1;
//
//							gpu_verts[21].tess_edge = 1;
//							gpu_verts[23].tess_edge = 1;
//
//							// Top Face
//							gpu_verts[25].tess_edge = 1;
//							gpu_verts[26].tess_edge = 1;
//
//							gpu_verts[27].tess_edge = 1;
//							gpu_verts[29].tess_edge = 1;
//
//							// Bot Face
//							gpu_verts[31].tess_edge = 1;
//							gpu_verts[32].tess_edge = 1;
//
//							gpu_verts[27].tess_edge = 1;
//							gpu_verts[35].tess_edge = 1;
//						}
//
//						std::memcpy(((GPU_MeshVertex*)octree_vbuff_mem) + octree_vi,
//							gpu_verts, 36 * sizeof(GPU_MeshVertex));
//
//						octree_vi += 36;
//					}
//
//					mesh_in_buff->aabbs_vertex_count = octree_vi - mesh_in_buff->aabbs_vertex_start;
//				}
//			}

void MeshRenderer::loadUniform()
{
	GPU_MeshUniform uniform;
	uniform.camera_pos = dxConvert(application.camera_pos);
	uniform.camera_quat = dxConvert(application.camera_quat_inv);
	uniform.camera_forward = dxConvert(application.camera_forward);

	uniform.perspective_matrix = DirectX::XMMatrixPerspectiveFovRH(
		toRad(application.camera_field_of_view),
		(float)viewport_width / (float)viewport_height,
		application.camera_z_near, application.camera_z_far);

	uniform.z_near = application.camera_z_near;
	uniform.z_far = application.camera_z_far;

	uint32_t i = 0;
	for (CameraLight& light : application.lights) {

		// Rotate the light normal to be relative to camera orientation
		glm::vec3 normal = light.normal * application.camera_quat_inv;
		normal = glm::normalize(normal);

		GPU_CameraLight& gpu_light = uniform.lights[i];
		gpu_light.normal = dxConvert(light.normal);  // change back to relative
		gpu_light.color = dxConvert(light.color);
		gpu_light.intensity = light.intensity;
		
		i++;
	}

	uniform.ambient_intensity = application.ambient_intensity;

	uniform.shading_normal = application.shading_normal;

	void* uniform_mem;
	frame_ubuff.beginLoad(sizeof(GPU_MeshUniform), uniform_mem);
	{
		std::memcpy(uniform_mem, &uniform, sizeof(GPU_MeshUniform));
	}
	frame_ubuff.endLoad();
}

void MeshRenderer::setWireframeDepthBias(int depth_bias)
{
	wire_bias_rs.setDepthBias(depth_bias);
	wire_none_bias_rs.setDepthBias(depth_bias);
	wire_none_rs.setDepthBias(depth_bias);
}

void lazyUnbind(ID3D11DeviceContext3* ctx3)
{
	std::array<ID3D11ShaderResourceView*, 2> srvs = {
		nullptr, nullptr
	};
	ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

	std::array<ID3D11RenderTargetView*, 2> rtvs = {
		nullptr, nullptr
	};
	ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), nullptr);
}

void MeshRenderer::draw(nui::SurfaceEvent& event)
{
	this->viewport_width = (float)event.viewport_size.x;
	this->viewport_height = (float)event.viewport_size.y;

	if (dev5 == nullptr) {

		dev5 = event.dev5;
		im_ctx3 = event.im_ctx3;

		render_target_width = 0;
		render_target_height = 0;
		
		load_uniform = true;

		// Scene Depth Resource
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = event.render_target_width;
			desc.Height = event.render_target_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_D32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			scene_dtex.create(dev5, im_ctx3, desc);

			D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			scene_dtex.createDepthStencilView(view_desc);
		}

		// Mesh Depth Mash Resource
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = event.render_target_width;
			desc.Height = event.render_target_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			mesh_mask_dtex.create(dev5, im_ctx3, desc);

			D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
			rtv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtv_desc.Texture2D.MipSlice = 0;

			mesh_mask_dtex.createRenderTargetView(rtv_desc);

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;

			mesh_mask_dtex.createShaderResourceView(srv_desc);
		}

		// Wireframe Depth Texture
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = event.render_target_width;
			desc.Height = event.render_target_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_D32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			wireframe_dtex.create(dev5, im_ctx3, desc);

			D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			wireframe_dtex.createDepthStencilView(view_desc);
		}

		// Frame Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			frame_ubuff.create(dev5, im_ctx3, desc);
		}

		// Octree Vertex Buffer
		/*{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			octree_vbuff.create(dev5, im_ctx3, desc);
		}*/

		// PBR Mesh Vertex Shader
		{
			dx11::createVertexShaderFromPath("Sculpt/CompiledShaders/MeshVS.cso", dev5,
				mesh_vs.GetAddressOf(), &shader_cso);

			// Vertex Input Layout
			auto vertex_elems = GPU_MeshVertex::getInputLayout();
			auto instance_elems = GPU_MeshInstance::getInputLayout();

			std::vector<D3D11_INPUT_ELEMENT_DESC> elems;
			elems.resize(vertex_elems.size() + instance_elems.size());

			uint32_t idx = 0;
			for (D3D11_INPUT_ELEMENT_DESC& elem : vertex_elems) {
				elems[idx++] = elem;
			}

			for (D3D11_INPUT_ELEMENT_DESC& elem : instance_elems) {
				elems[idx++] = elem;
			}

			throwDX11(dev5->CreateInputLayout(elems.data(), elems.size(),
				shader_cso.data(), shader_cso.size(), mesh_il.GetAddressOf()),
				"failed to create mesh vertex input layout");
		}

		// PBR Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshPS.cso", dev5,
			mesh_ps.GetAddressOf(), &shader_cso);

		// PBR Pixel Shader with depth output
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshOutputDepthPS.cso", dev5,
			mesh_output_depth_ps.GetAddressOf(), &shader_cso);

		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshDepthOnlyPS.cso", dev5,
			mesh_depth_only_ps.GetAddressOf(), &shader_cso);

		// Front Wireframe Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/FrontWirePS.cso", dev5,
			front_wire_ps.GetAddressOf(), &shader_cso);

		// Front Wireframe With Tesselation Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/FrontWireTessPS.cso", dev5,
			front_wire_tess_ps.GetAddressOf(), &shader_cso);

		// See Thru Wireframe Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/SeeThruWirePS.cso", dev5,
			see_thru_wire_ps.GetAddressOf(), &shader_cso);

		// See Thru Wireframe With Tesselation Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/SeeThruWireTessPS.cso", dev5,
			see_thru_wire_tess_ps.GetAddressOf(), &shader_cso);

		// Wireframe Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/WirePS.cso", dev5,
			wire_ps.GetAddressOf(), &shader_cso);

		// Wireframe With Tesselation Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/WireTessPS.cso", dev5,
			wire_tess_ps.GetAddressOf(), &shader_cso);

		// Mesh Rasterization States
		{
			int32_t depth_bias = -1000;

			D3D11_RASTERIZER_DESC desc = {};
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_BACK;
			desc.FrontCounterClockwise = false;
			desc.DepthBias = 0;
			desc.DepthBiasClamp = 0;
			desc.SlopeScaledDepthBias = 0;
			desc.DepthClipEnable = true;
			desc.ScissorEnable = true;
			desc.MultisampleEnable = false;
			desc.AntialiasedLineEnable = false;

			mesh_rs.create(dev5, desc);

			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.DepthBias = 0;
			mesh_none_rs.create(dev5, desc);

			desc.FillMode = D3D11_FILL_WIREFRAME;
			desc.CullMode = D3D11_CULL_BACK;
			desc.DepthBias = depth_bias;
			wire_bias_rs.create(dev5, desc);

			desc.FillMode = D3D11_FILL_WIREFRAME;
			desc.CullMode = D3D11_CULL_NONE;
			desc.DepthBias = depth_bias;
			wire_none_bias_rs.create(dev5, desc);

			desc.FillMode = D3D11_FILL_WIREFRAME;
			desc.CullMode = D3D11_CULL_NONE;
			desc.DepthBias = 0;
			wire_none_rs.create(dev5, desc);
		}

		// Blend
		{
			D3D11_BLEND_DESC desc = {};
			desc.AlphaToCoverageEnable = false;
			desc.IndependentBlendEnable = true;

			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			desc.RenderTarget[1].BlendEnable = false;
			desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			desc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			desc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			desc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			desc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			desc.RenderTarget[6].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			desc.RenderTarget[7].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			throwDX11(dev5->CreateBlendState(&desc, blend_target_0_bs.GetAddressOf()));

			for (uint8_t i = 0; i < 8; i++) {
				desc.RenderTarget[i].BlendEnable = false;
				desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			}

			throwDX11(dev5->CreateBlendState(&desc, blendless_bs.GetAddressOf()));
		}

		// Mesh Depth Stencil
		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_LESS;
			desc.StencilEnable = false;

			throwDX11(dev5->CreateDepthStencilState(&desc, depth_stencil.GetAddressOf()),
				"failed to create mesh depth stencil state");
		}
	}

	loadVertices();
	loadUniform();

	if (render_target_width != event.render_target_width ||
		render_target_height != event.render_target_height)
	{
		scene_dtex.resize(event.render_target_width, event.render_target_height);
		mesh_mask_dtex.resize(event.render_target_width, event.render_target_height);
		wireframe_dtex.resize(event.render_target_width, event.render_target_height);

		loadUniform();

		this->render_target_width = event.render_target_width;
		this->render_target_height = event.render_target_height;
	}

	// Rendering Commands ///////////////////////////////////////////////////////////////////////////////////

	im_ctx3->ClearDepthStencilView(scene_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);

	// Input Assembly
	{
		im_ctx3->IASetInputLayout(mesh_il.Get());
		im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Vertex Shader
	im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());

	// Rasterizer Viewport
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = (float)event.viewport_pos.x;
		viewport.TopLeftY = (float)event.viewport_pos.y;
		viewport.Width = (float)event.viewport_size.x;
		viewport.Height = (float)event.viewport_size.y;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		im_ctx3->RSSetViewports(1, &viewport);

		D3D11_RECT sccissor;
		sccissor.left = event.viewport_pos.x;
		sccissor.top = event.viewport_pos.y;
		sccissor.right = event.viewport_pos.x + event.viewport_size.x;
		sccissor.bottom = event.viewport_pos.y + event.viewport_size.y;

		im_ctx3->RSSetScissorRects(1, &sccissor);
	}

	// Pixel Shader
	{
		std::array<ID3D11Buffer*, 1> buffers = {
			frame_ubuff.buff.Get()
		};
		im_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());
	}

	for (Mesh& mesh : application.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		lazyUnbind(im_ctx3);

		// Input Assembly
		im_ctx3->IASetIndexBuffer(sculpt_mesh.gpu_indexes.get(), DXGI_FORMAT_R32_UINT, 0);

		for (MeshInstanceSet& set : mesh.sets) {
			
			MeshDrawcall* drawcall = set.drawcall;

			switch (drawcall->display_mode) {
			case DisplayMode::SOLID: {

				// Overview
				// - Draw the meshes using the PBR mesh shader

				// Input Assembly
				{
					std::array<ID3D11Buffer*, 2> buffs = {
						sculpt_mesh.gpu_verts.get(),
						set.gpu_instances.get()
					};

					std::array<uint32_t, 2> strides = {
						sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
					};

					std::array<uint32_t, 2> offsets = {
						0, 0
					};

					im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(),
						strides.data(), offsets.data());
				}

				// Vertex Shader
				im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);

				// Rasterization
				if (drawcall->is_back_culled) {
					im_ctx3->RSSetState(mesh_rs.get());
				}
				else {
					im_ctx3->RSSetState(mesh_none_rs.get());
				}

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						sculpt_mesh.gpu_triangles_srv.Get()
					};
					im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 1> rtvs = {
						event.compose_rtv,
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx3->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.size(), set.gpu_instances.size(),
					0, 0, 0);
				break;
			}
			}
		}
	}

	//for (MeshDrawcall& drawcall : application.drawcalls) {

	//	lazyUnbind(im_ctx3);

	//	// skip empty drawcalls
	//	if (!drawcall.mesh_instance_sets.size()) {
	//		continue;
	//	}

	//	switch (drawcall.display_mode) {
	//	case DisplayMode::SOLID: {

	//		/* Overview
	//		- Draw the meshes using the PBR mesh shader*/

	//		lazyUnbind(im_ctx3);

	//		// Vertex Shader
	//		im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);

	//		// Rasterization
	//		if (drawcall.is_back_culled) {
	//			im_ctx3->RSSetState(mesh_rs.get());
	//		}
	//		else {
	//			im_ctx3->RSSetState(mesh_none_rs.get());
	//		}

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11Buffer*, 1> buffers = {
	//				frame_ubuff.buff.Get()
	//			};
	//			im_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());

	//			im_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 2> rtvs = {
	//				event.compose_rtv, instance_poly_id_mask_tex.getRTV()
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance_set : drawcall.mesh_instance_sets) {

	//			scme::SculptMesh& mesh = instance_set.mesh->mesh;

	//			// Input Layout
	//			{
	//				ID3D11Buffer* idxbuff = mesh.gpu_indexes.get();
	//				ID3D11Buffer* vbuff = mesh.gpu_verts.get();
	//				ID3D11Buffer* instabuff = mesh.gpu_instances.get();

	//				std::array<ID3D11Buffer*, 2> buffers = {
	//					vbuff, instabuff.buff.Get()
	//				};
	//				std::array<uint32_t, 2> strides = {
	//					sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
	//				};
	//				std::array<uint32_t, 2> offsets = {
	//					0, 0
	//				};
	//				im_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
	//			}

	//			im_ctx3->DrawIndexedInstanced(instance.mesh->mesh_index_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_index_count, 0, instance.mesh_insta_start);
	//		}
	//		break;
	//	}
	//	
	//	case DisplayMode::SOLID_WITH_WIREFRAME_FRONT: {

	//		/* Overview
	//		- Draw the meshes using the PBR mesh shader
	//		- Draw the wireframe on top offset by depth bias */

	//		lazyUnbind(im_ctx3);

	//		// Input Assembly
	//		{
	//			im_ctx3->IASetInputLayout(mesh_il.Get());
	//			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//			std::array<ID3D11Buffer*, 2> buffers = {
	//				vbuff.buff.Get(), instabuff.buff.Get()
	//			};
	//			std::array<uint32_t, 2> strides = {
	//				sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
	//			};
	//			std::array<uint32_t, 2> offsets = {
	//				0, 0
	//			};
	//			im_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
	//		}

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}

	//		// Rasterization
	//		if (drawcall.is_back_culled) {
	//			im_ctx3->RSSetState(mesh_rs.get());
	//		}
	//		else {
	//			im_ctx3->RSSetState(mesh_none_rs.get());
	//		}

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11Buffer*, 1> buffers = {
	//				frame_ubuff.buff.Get()
	//			};
	//			im_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());

	//			im_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 2> rtvs = {
	//				event.compose_rtv, instance_poly_id_mask_tex.getRTV()
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}

	//		// Draw the wireframe on top offset by depth bias

	//		lazyUnbind(im_ctx3);

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}

	//		// Rasterization
	//		im_ctx3->RSSetState(wire_bias_rs.get());

	//		// Pixel Shader
	//		{
	//			switch (application.shading_normal) {
	//			case ShadingNormal::VERTEX:
	//			case ShadingNormal::POLY: {
	//				im_ctx3->PSSetShader(front_wire_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			case ShadingNormal::TESSELATION: {
	//				im_ctx3->PSSetShader(front_wire_tess_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			}
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 1> rtvs = {
	//				event.compose_rtv
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			// Input Layout
	//			{
	//				ID3D11Buffer* vbuff = instance.mesh->mesh.gpu_verts.get();

	//				std::array<ID3D11Buffer*, 2> buffers = {
	//					vbuff, instabuff.buff.Get()
	//				};
	//				std::array<uint32_t, 2> strides = {
	//					sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
	//				};
	//				std::array<uint32_t, 2> offsets = {
	//					0, 0
	//				};
	//				im_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
	//			}

	//			im_ctx3->DrawIndexedInstanced(instance.mesh->mesh_index_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_index_count, 0, instance.mesh_insta_start);
	//		}

	//		break;
	//	}

	//	case DisplayMode::SOLID_WITH_WIREFRAME_NONE: {

	//		/* Overview
	//		- clear the mesh depth tex
	//		- draw the solid mesh but record the resulting depth in mesh depth tex */

	//		lazyUnbind(im_ctx3);

	//		// Clear Mesh Mask depth texture
	//		{
	//			float clear[4] = {
	//				1.f, 0.f, 0.f, 0.f
	//			};
	//			im_ctx3->ClearRenderTargetView(mesh_mask_dtex.getRTV(), clear);
	//		}

	//		// Input Assembly
	//		im_ctx3->IASetInputLayout(mesh_il.Get());
	//		im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//		{
	//			std::array<ID3D11Buffer*, 2> buffs = {
	//				vbuff.buff.Get(), instabuff.buff.Get()
	//			};
	//			std::array<uint32_t, 2> strides = {
	//				sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
	//			};
	//			std::array<uint32_t, 2> offsets = {
	//				0, 0
	//			};
	//			im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(), strides.data(), offsets.data());
	//		}

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}

	//		// Rasterization
	//		im_ctx3->RSSetState(mesh_none_rs.get());

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11Buffer*, 1> buffs = {
	//				frame_ubuff.buff.Get()
	//			};
	//			im_ctx3->PSSetConstantBuffers(0, buffs.size(), buffs.data());

	//			im_ctx3->PSSetShader(mesh_output_depth_ps.Get(), nullptr, 0);
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 3> rtvs = {
	//				event.compose_rtv, mesh_mask_dtex.getRTV(), instance_poly_id_mask_tex.getRTV()
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}

	//		// Draw the wireframe ONLY for the mesh mask depth tex

	//		lazyUnbind(im_ctx3);

	//		im_ctx3->ClearDepthStencilView(wireframe_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}		

	//		// Rasterization
	//		im_ctx3->RSSetState(wire_none_bias_rs.get());

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11ShaderResourceView*, 1> srvs = {
	//				mesh_mask_dtex.getSRV()
	//			};
	//			im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

	//			switch (application.shading_normal) {
	//			case ShadingNormal::VERTEX:
	//			case ShadingNormal::POLY: {
	//				im_ctx3->PSSetShader(see_thru_wire_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			case ShadingNormal::TESSELATION: {
	//				im_ctx3->PSSetShader(see_thru_wire_tess_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			}
	//		}

	//		// Output Merger
	//		{
	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			std::array<ID3D11RenderTargetView*, 1> rtvs = {
	//				event.compose_rtv
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), wireframe_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}

	//		break;
	//	}

	//	case DisplayMode::WIREFRANE: {

	//		lazyUnbind(im_ctx3);

	//		// Clear Mesh Mask depth texture
	//		{
	//			float clear[4] = {
	//				1.f, 0.f, 0.f, 0.f
	//			};
	//			im_ctx3->ClearRenderTargetView(mesh_mask_dtex.getRTV(), clear);
	//		}

	//		configureMeshInputAssembly();
	//		configureMeshVertexShader();

	//		// Rasterization
	//		im_ctx3->RSSetState(mesh_none_rs.get());

	//		// Pixel Shader
	//		im_ctx3->PSSetShader(mesh_depth_only_ps.Get(), nullptr, 0);

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blendless_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 2> rtvs = {
	//				mesh_mask_dtex.getRTV(), instance_poly_id_mask_tex.getRTV()
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}

	//		// Draw the wireframe ONLY for the mesh mask depth tex

	//		lazyUnbind(im_ctx3);
	//		im_ctx3->ClearDepthStencilView(wireframe_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}

	//		// Rasterization
	//		im_ctx3->RSSetState(wire_none_bias_rs.get());

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11ShaderResourceView*, 1> srvs = {
	//				mesh_mask_dtex.getSRV()
	//			};
	//			im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

	//			switch (application.shading_normal) {
	//			case ShadingNormal::VERTEX:
	//			case ShadingNormal::POLY: {
	//				im_ctx3->PSSetShader(see_thru_wire_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			case ShadingNormal::TESSELATION: {
	//				im_ctx3->PSSetShader(see_thru_wire_tess_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			}
	//		}

	//		// Output Merger
	//		{
	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			std::array<ID3D11RenderTargetView*, 1> rtvs = {
	//				event.compose_rtv
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), wireframe_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}
	//		break;
	//	}

	//	case DisplayMode::WIREFRAME_PURE: {

	//		lazyUnbind(im_ctx3);

	//		configureMeshInputAssembly();
	//		configureMeshVertexShader();

	//		// Rasterization
	//		im_ctx3->RSSetState(wire_none_rs.get());

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11Buffer*, 1> buffers = {
	//				frame_ubuff.buff.Get()
	//			};
	//			im_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());

	//			switch (application.shading_normal) {
	//			case ShadingNormal::VERTEX:
	//			case ShadingNormal::POLY: {
	//				im_ctx3->PSSetShader(wire_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			case ShadingNormal::TESSELATION: {
	//				im_ctx3->PSSetShader(wire_tess_ps.Get(), nullptr, 0);
	//				break;
	//			}
	//			}
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 2> rtvs = {
	//				event.compose_rtv, instance_poly_id_mask_tex.getRTV()
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
	//		}
	//		break;
	//	}
	//	}

	//	 Draw Octree
	//	if (drawcall._debug_show_octree) {

	//		lazyUnbind(im_ctx3);

	//		// Input Assembly
	//		{
	//			im_ctx3->IASetInputLayout(mesh_il.Get());
	//			im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//			std::array<ID3D11Buffer*, 2> buffers = {
	//				octree_vbuff.buff.Get(), instabuff.buff.Get()
	//			};
	//			std::array<uint32_t, 2> strides = {
	//				sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
	//			};
	//			std::array<uint32_t, 2> offsets = {
	//				0, 0
	//			};
	//			im_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
	//		}

	//		// Vertex Shader
	//		{
	//			im_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
	//			im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
	//		}

	//		// Rasterization
	//		im_ctx3->RSSetState(wire_none_rs.get());

	//		// Pixel Shader
	//		{
	//			std::array<ID3D11Buffer*, 1> buffers = {
	//				frame_ubuff.buff.Get()
	//			};
	//			im_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());

	//			im_ctx3->PSSetShader(wire_ps.Get(), nullptr, 0);
	//		}

	//		// Output Merger
	//		{
	//			float blend_factor[4] = { 1, 1, 1, 1 };
	//			im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

	//			im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

	//			std::array<ID3D11RenderTargetView*, 1> rtvs = {
	//				event.compose_rtv
	//			};
	//			im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
	//		}

	//		for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

	//			im_ctx3->DrawInstanced(
	//				instance.mesh->aabbs_vertex_count, instance.mesh_insta_count,
	//				instance.mesh->aabbs_vertex_start, instance.mesh_insta_start);
	//		}
	//	}
	//}
}

void geometryDraw(nui::Window* window, nui::StoredElement* source, nui::SurfaceEvent& event, void* user_data)
{
	application.renderer.draw(event);
}