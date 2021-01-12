
// Header
#include "Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"


using namespace scme;


ErrStack MeshRenderer::loadVertices()
{
	ErrStack err_stack;

	uint32_t mesh_vi = 0;
	uint32_t mesh_insta_idx = 0;
	uint32_t aabb_vi = 0;
	uint32_t aabb_idx = 0;

	// Count
	{
		for (MeshDrawcall& drawcall : application.drawcalls) {

			for (MeshInstanceSet& mesh_instances : drawcall.mesh_instance_sets) {

				SculptMesh& mesh = mesh_instances.mesh->mesh;

				for (Poly& poly : mesh.polys) {

					if (poly.is_tris) {
						mesh_vi += 3;
					}
					else {
						mesh_vi += 6;
					}
				}

				mesh_insta_idx += mesh_instances.instances.size();

				if (drawcall.show_aabbs) {
					aabb_vi += mesh.aabbs.size() * 8;
					aabb_idx += mesh.aabbs.size() * 36;
				}
			}
		}
	}

	void* vertex_buff_mem;
	void* mesh_instabuff_mem;
	if (mesh_vi) {
		checkErrStack1(vbuff.beginLoad(mesh_vi * sizeof(GPU_MeshVertex), 0, vertex_buff_mem));
		checkErrStack1(instabuff.beginLoad(mesh_insta_idx * sizeof(GPU_MeshInstance), 0, mesh_instabuff_mem));
	}
	
	void* aabb_vbuff_mem;
	void* aabb_ibuff_mem;
	if (aabb_vi) {
		checkErrStack1(aabbs_vbuff.beginLoad(aabb_vi * sizeof(GPU_AABB_Vertex), 0, aabb_vbuff_mem));
		checkErrStack1(aabbs_ibuff.beginLoad(aabb_idx * sizeof(uint32_t), 0, aabb_ibuff_mem));
	}

	mesh_vi = 0;
	mesh_insta_idx = 0;
	aabb_vi = 0;
	aabb_idx = 0;

	GPU_MeshVertex gpu_verts[6];
	GPU_AABB_Vertex gpu_aabb_verts[8];
	uint32_t gpu_aabb_indexes[36];
	GPU_MeshInstance gpu_inst;	

	for (MeshDrawcall& drawcall : application.drawcalls) {
		
		for (MeshInstanceSet& mesh_instances : drawcall.mesh_instance_sets) {

			MeshInBuffers* mesh_in_buff = mesh_instances.mesh;
			SculptMesh& mesh = mesh_in_buff->mesh;

			// Load mesh vertices
			if (mesh_in_buff->mesh_vertex_start == 0xFFFF'FFFF) {

				mesh_in_buff->mesh_vertex_start = mesh_vi;

				for (Poly& poly : mesh.polys) {

					Loop& l0 = mesh.loops[poly.inner_loop];
					Loop& l1 = mesh.loops[l0.poly_next_loop];
					Loop& l2 = mesh.loops[l1.poly_next_loop];

					if (poly.is_tris) {

						Vertex* v0 = &mesh.verts[l2.target_v];
						Vertex* v1 = &mesh.verts[l0.target_v];
						Vertex* v2 = &mesh.verts[l1.target_v];

						// Convert to GPU format
						gpu_verts[0].pos = dxConvert(v0->pos);
						gpu_verts[1].pos = dxConvert(v1->pos);
						gpu_verts[2].pos = dxConvert(v2->pos);

						// Normal
						switch (application.shading_normal) {
						case ShadingNormal::VERTEX: {
							gpu_verts[0].normal = dxConvert(v0->normal);
							gpu_verts[1].normal = dxConvert(v1->normal);
							gpu_verts[2].normal = dxConvert(v2->normal);
							break;
						}
						case ShadingNormal::POLY:
						case ShadingNormal::TESSELATION: {
							for (uint8_t i = 0; i < 3; i++) {
								gpu_verts[i].normal = dxConvert(poly.normal);
							}
							break;
						}
						}

						// Tesselation edge
						for (uint8_t i = 0; i < 3; i++) {
							gpu_verts[i].tess_edge = 0;
						}

						// Copy Data
						std::memcpy(((GPU_MeshVertex*)vertex_buff_mem) + mesh_vi,
							gpu_verts, 3 * sizeof(GPU_MeshVertex)
						);

						mesh_vi += 3;
					}
					else {
						Loop& l3 = mesh.loops[l2.poly_next_loop];

						Vertex* v0 = &mesh.verts[l3.target_v];
						Vertex* v1 = &mesh.verts[l0.target_v];
						Vertex* v2 = &mesh.verts[l1.target_v];
						Vertex* v3 = &mesh.verts[l2.target_v];

						std::array<Vertex*, 6> tess;

						if (poly.tesselation_type == 0) {

							tess[0] = v0;
							tess[1] = v2;
							tess[2] = v3;

							tess[3] = v0;
							tess[4] = v1;
							tess[5] = v2;

							// Tesselation Edge
							gpu_verts[0].tess_edge = 1;
							gpu_verts[1].tess_edge = 1;
							gpu_verts[2].tess_edge = 0;
							gpu_verts[3].tess_edge = 1;
							gpu_verts[4].tess_edge = 0;
							gpu_verts[5].tess_edge = 1;
							
							gpu_verts[0].tess_edge_dir = 1;
							gpu_verts[1].tess_edge_dir = 0;
							gpu_verts[2].tess_edge_dir = 0;
							gpu_verts[3].tess_edge_dir = 1;
							gpu_verts[4].tess_edge_dir = 0;
							gpu_verts[5].tess_edge_dir = 0;
						}
						else {
							tess[0] = v0;
							tess[1] = v1;
							tess[2] = v3;

							tess[3] = v1;
							tess[4] = v2;
							tess[5] = v3;

							// Tesselation Edge
							gpu_verts[0].tess_edge = 0;
							gpu_verts[1].tess_edge = 1;
							gpu_verts[2].tess_edge = 1;
							gpu_verts[3].tess_edge = 1;
							gpu_verts[4].tess_edge = 0;
							gpu_verts[5].tess_edge = 1;

							gpu_verts[0].tess_edge_dir = 0;
							gpu_verts[1].tess_edge_dir = 1;
							gpu_verts[2].tess_edge_dir = 0;
							gpu_verts[3].tess_edge_dir = 1;
							gpu_verts[4].tess_edge_dir = 0;
							gpu_verts[5].tess_edge_dir = 0;
						}

						// Convert to GPU format
						for (uint8_t i = 0; i < 6; i++) {
							gpu_verts[i].pos = dxConvert(tess[i]->pos);
						}

						switch (application.shading_normal) {
						case ShadingNormal::VERTEX: {
							for (uint8_t i = 0; i < 6; i++) {
								gpu_verts[i].normal = dxConvert(tess[i]->normal);
							}
							break;
						}
						case ShadingNormal::POLY: {
							for (uint8_t i = 0; i < 6; i++) {
								gpu_verts[i].normal = dxConvert(poly.normal);
							}
							break;
						}
						case ShadingNormal::TESSELATION: {
							gpu_verts[0].normal = dxConvert(poly.tess_normals[0]);
							gpu_verts[1].normal = dxConvert(poly.tess_normals[0]);
							gpu_verts[2].normal = dxConvert(poly.tess_normals[0]);
							gpu_verts[3].normal = dxConvert(poly.tess_normals[1]);
							gpu_verts[4].normal = dxConvert(poly.tess_normals[1]);
							gpu_verts[5].normal = dxConvert(poly.tess_normals[1]);
							break;
						}
						}

						// Copy Data
						std::memcpy(((GPU_MeshVertex*)vertex_buff_mem) + mesh_vi,
							gpu_verts, 6 * sizeof(GPU_MeshVertex)
						);

						mesh_vi += 6;
					}
				}

				mesh_in_buff->mesh_vertex_count = mesh_vi - mesh_in_buff->mesh_vertex_start;
			}

			// Load AABBs vertices
			if (drawcall.show_aabbs && mesh_in_buff->aabb_vertex_start == 0xFFFF'FFFF)
			{
				mesh_in_buff->aabb_vertex_start = aabb_vi;
				mesh_in_buff->aabb_index_start = aabb_idx;

				for (VertexBoundingBox& aabb : mesh.aabbs) {

					glm::vec3& min = aabb.aabb.min;
					glm::vec3& max = aabb.aabb.max;

					// Forward (Classic winding)
					gpu_aabb_verts[0].pos = dxConvert(glm::vec3{ min.x, max.y, max.z });  // top left
					gpu_aabb_verts[1].pos = dxConvert(glm::vec3{ max.x, max.y, max.z });  // top right
					gpu_aabb_verts[2].pos = dxConvert(glm::vec3{ max.x, min.y, max.z });  // bot right
					gpu_aabb_verts[3].pos = dxConvert(glm::vec3{ min.x, min.y, max.z });  // bot left

					// Backward
					gpu_aabb_verts[4].pos = dxConvert(glm::vec3{ min.x, max.y, min.z });  // top left
					gpu_aabb_verts[5].pos = dxConvert(glm::vec3{ max.x, max.y, min.z });  // top right
					gpu_aabb_verts[6].pos = dxConvert(glm::vec3{ max.x, min.y, min.z });  // bot right
					gpu_aabb_verts[7].pos = dxConvert(glm::vec3{ min.x, min.y, min.z });  // bot left

					std::memcpy(((GPU_AABB_Vertex*)vertex_buff_mem) + aabb_vi,
						gpu_aabb_verts, 8 * sizeof(GPU_AABB_Vertex));

					aabb_vi += 8;

					// Front Face
					gpu_aabb_indexes[0] = 0;
					gpu_aabb_indexes[1] = 1;
					gpu_aabb_indexes[2] = 3;

					gpu_aabb_indexes[3] = 1;
					gpu_aabb_indexes[4] = 2;
					gpu_aabb_indexes[5] = 3;

					// Right Face
					gpu_aabb_indexes[6] = 1;
					gpu_aabb_indexes[7] = 5;
					gpu_aabb_indexes[8] = 2;

					gpu_aabb_indexes[9] = 5;
					gpu_aabb_indexes[10] = 6;
					gpu_aabb_indexes[11] = 3;

					// Back Face
					gpu_aabb_indexes[12] = 5;
					gpu_aabb_indexes[13] = 4;
					gpu_aabb_indexes[14] = 6;

					gpu_aabb_indexes[15] = 4;
					gpu_aabb_indexes[16] = 7;
					gpu_aabb_indexes[17] = 6;

					// Left Face
					gpu_aabb_indexes[18] = 4;
					gpu_aabb_indexes[19] = 0;
					gpu_aabb_indexes[20] = 7;

					gpu_aabb_indexes[21] = 0;
					gpu_aabb_indexes[22] = 3;
					gpu_aabb_indexes[23] = 7;

					// Top Face
					gpu_aabb_indexes[24] = 4;
					gpu_aabb_indexes[25] = 5;
					gpu_aabb_indexes[26] = 0;

					gpu_aabb_indexes[27] = 5;
					gpu_aabb_indexes[28] = 6;
					gpu_aabb_indexes[29] = 0;

					// Back Face
					gpu_aabb_indexes[30] = 6;
					gpu_aabb_indexes[31] = 7;
					gpu_aabb_indexes[32] = 2;

					gpu_aabb_indexes[33] = 7;
					gpu_aabb_indexes[34] = 3;
					gpu_aabb_indexes[35] = 2;

					std::memcpy(((uint32_t*)aabb_ibuff_mem) + aabb_idx, gpu_aabb_indexes,
						36 * sizeof(uint32_t));

					aabb_idx += 36;
				}

				mesh_in_buff->aabb_vertex_count = aabb_vi - mesh_in_buff->aabb_vertex_start;
				mesh_in_buff->aabb_index_count = aabb_idx - mesh_in_buff->aabb_index_start;
			}

			// Load mesh instances
			if (mesh_instances.mesh_insta_start == 0xFFFF'FFFF) {

				mesh_instances.mesh_insta_start = mesh_insta_idx;

				for (MeshInstance* instance : mesh_instances.instances) {

					gpu_inst.pos = dxConvert(instance->pos);
					gpu_inst.rot = dxConvert(instance->rot);

					PhysicalBasedMaterial& pbr_mat = instance->pbr_material;
					gpu_inst.albedo_color = dxConvert(pbr_mat.albedo_color);
					gpu_inst.roughness = glm::clamp(pbr_mat.roughness, 0.05f, 1.f);
					gpu_inst.metallic = pbr_mat.metallic;
					gpu_inst.specular = pbr_mat.specular;

					MeshWireframeColors wire_colors = instance->wireframe_colors;
					gpu_inst.wireframe_front_color = dxConvert(wire_colors.front_color);
					gpu_inst.wireframe_back_color = dxConvert(wire_colors.back_color);
					gpu_inst.wireframe_tess_front_color = dxConvert(wire_colors.tesselation_front_color);
					gpu_inst.wireframe_tess_back_color = dxConvert(wire_colors.tesselation_back_color);
					gpu_inst.wireframe_tess_split_count = wire_colors.tesselation_split_count;
					gpu_inst.wireframe_tess_gap = wire_colors.tesselation_gap;

					std::memcpy(((GPU_MeshInstance*)mesh_instabuff_mem) + mesh_insta_idx,
						&gpu_inst, sizeof(GPU_MeshInstance)
					);

					mesh_insta_idx++;
				}

				mesh_instances.mesh_insta_count = mesh_insta_idx - mesh_instances.mesh_insta_start;
			}
		}
	}

	if (mesh_vi) {
		vbuff.endLoad();
		instabuff.endLoad();
	}

	if (aabb_vi) {
		aabbs_vbuff.endLoad();
		aabbs_ibuff.endLoad();
	}

	// TODO: unsignal stored meshes

	return err_stack;
}

ErrStack MeshRenderer::loadUniform()
{
	ErrStack err_stack;

	GPU_MeshUniform uniform;
	uniform.camera_pos = dxConvert(application.camera_pos);
	uniform.camera_quat = dxConvert(application.camera_quat_inv);
	uniform.camera_forward = dxConvert(application.camera_forward);

	if (false) {

		// teoretically this is the proper Reversed Z but of course, it does not work
		glm::mat4x4 persp = glm::perspectiveFovRH_ZO(toRad(application.camera_field_of_view),
			(float)viewport_width, (float)viewport_height,
			1.f, 0.1f);

		glm::mat4x4 z_reversal = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, -1, 1,
			0, 0, 0, 1
		};

		persp = persp * z_reversal;

		uniform.perspective_matrix = dxConvertMatrix(persp);
	}
	else {
		uniform.perspective_matrix = DirectX::XMMatrixPerspectiveFovRH(
			toRad(application.camera_field_of_view),
			(float)viewport_width / (float)viewport_height,
			application.camera_z_near, application.camera_z_far);
	}

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

	void* uniform_mem;
	checkErrStack1(frame_ubuff.beginLoad(sizeof(GPU_MeshUniform), 0, uniform_mem));
	{
		std::memcpy(uniform_mem, &uniform, sizeof(GPU_MeshUniform));
	}
	frame_ubuff.endLoad();

	return err_stack;
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

ErrStack MeshRenderer::draw(nui::SurfaceEvent& event)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	this->viewport_width = (float)event.viewport_size.x;
	this->viewport_height = (float)event.viewport_size.y;

	if (dev5 == nullptr) {

		dev5 = event.dev5;
		im_ctx3 = event.im_ctx3;
		de_ctx3 = event.de_ctx3;
		
		load_vertices = true;
		load_uniform = true;

		this->render_target_width = event.render_target_width;
		this->render_target_height = event.render_target_height;

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

			checkHResult1(dev5->CreateTexture2D(&desc, nullptr, scene_dtex.GetAddressOf()));

			D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			checkHResult1(dev5->CreateDepthStencilView(scene_dtex.Get(), &view_desc, scene_dsv.GetAddressOf()));
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

			checkHResult1(dev5->CreateTexture2D(&desc, nullptr, mesh_mask_dtex.GetAddressOf()));

			D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
			rtv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtv_desc.Texture2D.MipSlice = 0;

			checkHResult1(dev5->CreateRenderTargetView(mesh_mask_dtex.Get(), &rtv_desc, mesh_mask_rtv.GetAddressOf()));

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;

			checkHResult1(dev5->CreateShaderResourceView(mesh_mask_dtex.Get(), &srv_desc, mesh_mask_srv.GetAddressOf()));
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

			checkHResult1(dev5->CreateTexture2D(&desc, nullptr, wireframe_dtex.GetAddressOf()));

			D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			checkHResult1(dev5->CreateDepthStencilView(wireframe_dtex.Get(), &view_desc, wireframe_dsv.GetAddressOf()));
		}

		// Vertex Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			vbuff.create(dev5, im_ctx3, desc);
		}

		// Instance Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;  // TODO: maybe staging into default
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			instabuff.create(dev5, im_ctx3, desc);
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

		// Drawcall Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			drawcall_ubuff.create(dev5, de_ctx3, desc);
		}

		// Vertex Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/MeshVS.cso", mesh_vs_cso));

			checkHResult(dev5->CreateVertexShader(mesh_vs_cso.data(), mesh_vs_cso.size(), nullptr,
				mesh_vs.GetAddressOf()),
				"failed to create mesh vertex shader");
		}

		// Pixel Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/MeshPS.cso", mesh_ps_cso));

			checkHResult(dev5->CreatePixelShader(mesh_ps_cso.data(), mesh_ps_cso.size(), nullptr,
				mesh_ps.GetAddressOf()),
				"failed to create mesh pixel shader");
		}

		// Mesh Pixel Shader with depth output
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/MeshOutputDepthPS.cso", mesh_output_depth_ps_cso));

			checkHResult(dev5->CreatePixelShader(mesh_output_depth_ps_cso.data(), mesh_output_depth_ps_cso.size(), nullptr,
				mesh_output_depth_ps.GetAddressOf()),
				"failed to create mesh output depth pixel shader");
		}

		// Wireframe Pixel Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/DimWireframePS.cso", dim_wireframe_ps_cso));

			checkHResult(dev5->CreatePixelShader(dim_wireframe_ps_cso.data(), dim_wireframe_ps_cso.size(), nullptr,
				dim_wireframe_ps.GetAddressOf()),
				"failed to create wireframe pixel shader");
		}

		// Wireframe With Tesselation Pixel Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/WireframeWithTessellation.cso", wireframe_with_tessellation_ps_cso));

			checkHResult(dev5->CreatePixelShader(wireframe_with_tessellation_ps_cso.data(),wireframe_with_tessellation_ps_cso.size(), nullptr,
				wireframe_with_tesselation_ps.GetAddressOf()),
				"failed to create wireframe with tesselation pixel shader");
		}

		// Vertex Input Layout
		{
			auto vertex_elems = GPU_MeshVertex::getInputLayout();
			auto instance_elems = GPU_MeshInstance::getInputLayout();

			std::vector<D3D11_INPUT_ELEMENT_DESC> elems;
			elems.resize(vertex_elems.size() + instance_elems.size());
			
			uint32_t idx = 0;
			for (D3D11_INPUT_ELEMENT_DESC& elem: vertex_elems) {
				elems[idx++] = elem;
			}

			for (D3D11_INPUT_ELEMENT_DESC& elem : instance_elems) {
				elems[idx++] = elem;
			}

			checkHResult(dev5->CreateInputLayout(elems.data(), elems.size(),
				mesh_vs_cso.data(), mesh_vs_cso.size(), mesh_il.GetAddressOf()),
				"failed to create mesh vertex input layout");
		}

		// Mesh Rasterization State
		{
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

			checkHResult(dev5->CreateRasterizerState(&desc, fill_front_rs.GetAddressOf()),
				"failed to create raster state");

			desc.CullMode = D3D11_CULL_NONE;
			checkHResult1(dev5->CreateRasterizerState(&desc, fill_none_rs.GetAddressOf()));

			desc.FillMode = D3D11_FILL_WIREFRAME;
			desc.CullMode = D3D11_CULL_BACK;
			desc.DepthBias = 100'000;  // TODO: make this a setting
			checkHResult(dev5->CreateRasterizerState(&desc, wireframe_bias_rs.GetAddressOf()),
				"failed to create raster state");

			// Wireframe Rasterizer State
			desc.CullMode = D3D11_CULL_NONE;
			checkHResult(dev5->CreateRasterizerState(&desc, wireframe_none_bias_rs.GetAddressOf()),
				"failed to create raster state");
		}

		// Mesh Blend States
		{
			D3D11_BLEND_DESC desc = {};
			desc.AlphaToCoverageEnable = false;
			desc.IndependentBlendEnable = true;

			desc.RenderTarget[0].BlendEnable = false;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			desc.RenderTarget[1].BlendEnable = false;
			desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			checkHResult(dev5->CreateBlendState(&desc, mesh_bs.GetAddressOf()),
				"failed to create mesh blend state");
		}

		// Dim Wireframe Blend States
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

			checkHResult(dev5->CreateBlendState(&desc, blend_target_0_bs.GetAddressOf()),
				"failed to create mesh blend state");
		}

		// Mesh Depth Stencil
		{
			D3D11_DEPTH_STENCIL_DESC desc = {};
			desc.DepthEnable = true;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_GREATER;
			desc.StencilEnable = false;

			checkHResult(dev5->CreateDepthStencilState(&desc, greater_dss.GetAddressOf()),
				"failed to create mesh depth stencil state");
		}
	}

	if (load_vertices) {
		checkErrStack1(loadVertices());

		// checkErrStack1(loadAABBs());

		load_vertices = false;
	}

	if (load_uniform) {
		checkErrStack1(loadUniform());
		load_uniform = false;
	}

	/*if (render_target_width != event.render_target_width ||
		render_target_height != event.render_target_height)
	{
		mesh_depth_view->Release();
		wireframe_depth_view->Release();

		mesh_depth_srv->Release();

		checkErrStack1(dx11::resizeTexture2D(dev5, event.render_target_width, event.render_target_height, mesh_depth_tex));
		checkErrStack1(dx11::resizeTexture2D(dev5, event.render_target_width, event.render_target_height, wireframe_depth_tex));

		{
			D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_D32_FLOAT;
			view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			checkHResult(dev5->CreateDepthStencilView(mesh_depth_tex.Get(), &view_desc, mesh_depth_view.GetAddressOf()),
				"failed to rereate mesh depth view");

			checkHResult(dev5->CreateDepthStencilView(wireframe_depth_tex.Get(), &view_desc, wireframe_depth_view.GetAddressOf()),
				"failed to rereate wireframe depth view");
		}

		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = 1;

			checkHResult(dev5->CreateShaderResourceView(mesh_depth_tex.Get(), &srv_desc, mesh_depth_srv.GetAddressOf()),
				"failed to create mesh SRV");
		}

		checkErrStack1(loadUniform());
		load_uniform = false;
	}*/

	this->render_target_width = event.render_target_width;
	this->render_target_height = event.render_target_height;

	// Command List
	de_ctx3->ClearDepthStencilView(scene_dsv.Get(), D3D11_CLEAR_DEPTH, 0, 0);	

	for (MeshDrawcall& drawcall : application.drawcalls) {

		// skip empty drawcalls
		if (!drawcall.mesh_instance_sets.size()) {
			continue;
		}

		// Rasterizer Viewport
		{
			D3D11_VIEWPORT viewport;
			viewport.TopLeftX = (float)event.viewport_pos.x;
			viewport.TopLeftY = (float)event.viewport_pos.y;
			viewport.Width = (float)event.viewport_size.x;
			viewport.Height = (float)event.viewport_size.y;
			viewport.MinDepth = 0;
			viewport.MaxDepth = 1;

			de_ctx3->RSSetViewports(1, &viewport);

			D3D11_RECT sccissor;
			sccissor.left = event.viewport_pos.x;
			sccissor.top = event.viewport_pos.y;
			sccissor.right = event.viewport_pos.x + event.viewport_size.x;
			sccissor.bottom = event.viewport_pos.y + event.viewport_size.y;

			de_ctx3->RSSetScissorRects(1, &sccissor);
		}

		switch (drawcall.rasterizer_mode) {
		case RasterizerMode::SOLID: {

			/* Overview
			- Draw the meshes using the PBR mesh shader*/

			lazyUnbind(de_ctx3);

			// Input Assembly
			{
				de_ctx3->IASetInputLayout(mesh_il.Get());
				de_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				std::array<ID3D11Buffer*, 2> buffers = {
					vbuff.buff.Get(), instabuff.buff.Get()
				};
				std::array<uint32_t, 2> strides = {
					sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
				};
				std::array<uint32_t, 2> offsets = {
					0, 0
				};
				de_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
			}

			// Vertex Shader
			{
				de_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
				de_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
			}

			// Rasterization
			if (drawcall.is_back_culled) {
				de_ctx3->RSSetState(fill_front_rs.Get());
			}
			else {
				de_ctx3->RSSetState(fill_none_rs.Get());
			}

			// Pixel Shader
			{
				std::array<ID3D11Buffer*, 1> buffers = {
					frame_ubuff.buff.Get()
				};
				de_ctx3->PSSetConstantBuffers(0, buffers.size(), buffers.data());

				de_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);
			}

			// Output Merger
			{
				float blend_factor[4] = { 1, 1, 1, 1 };
				de_ctx3->OMSetBlendState(mesh_bs.Get(), blend_factor, 0xFFFF'FFFF);

				de_ctx3->OMSetDepthStencilState(greater_dss.Get(), 1);

				std::array<ID3D11RenderTargetView*, 1> rtvs = {
					event.compose_rtv
				};
				de_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dsv.Get());
			}

			for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

				de_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
					instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
			}
			break;
		}

		case RasterizerMode::SOLID_WITH_WIREFRAME_NONE: {

			/* Overview
			- clear the mesh depth tex
			- draw the solid mesh but run the pixel shader with early depth test and record the resulting depth
			in mesh depth tex */

			lazyUnbind(de_ctx3);

			// Clear Mesh Mask depth texture
			{
				float clear[4] = {
					0.f, 0.f, 0.f, 0.f
				};
				de_ctx3->ClearRenderTargetView(mesh_mask_rtv.Get(), clear);
			}

			de_ctx3->ClearDepthStencilView(wireframe_dsv.Get(), D3D11_CLEAR_DEPTH, 0, 0);

			// Input Assembly
			de_ctx3->IASetInputLayout(mesh_il.Get());
			de_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			{
				std::array<ID3D11Buffer*, 2> buffs = {
					vbuff.buff.Get(), instabuff.buff.Get()
				};
				std::array<uint32_t, 2> strides = {
					sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
				};
				std::array<uint32_t, 2> offsets = {
					0, 0
				};
				de_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(), strides.data(), offsets.data());
			}

			// Vertex Shader
			{
				de_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
				de_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
			}

			// Rasterization
			if (drawcall.is_back_culled) {
				de_ctx3->RSSetState(fill_front_rs.Get());
			}
			else {
				de_ctx3->RSSetState(fill_none_rs.Get());
			}

			// Pixel Shader
			{
				std::array<ID3D11Buffer*, 1> buffs = {
					frame_ubuff.buff.Get()
				};
				de_ctx3->PSSetConstantBuffers(0, buffs.size(), buffs.data());

				de_ctx3->PSSetShader(mesh_output_depth_ps.Get(), nullptr, 0);
			}

			// Output Merger
			{
				float blend_factor[4] = { 1, 1, 1, 1 };
				de_ctx3->OMSetBlendState(mesh_bs.Get(), blend_factor, 0xFFFF'FFFF);

				de_ctx3->OMSetDepthStencilState(greater_dss.Get(), 1);

				std::array<ID3D11RenderTargetView*, 2> rtvs = {
					event.compose_rtv, mesh_mask_rtv.Get()
				};
				de_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dsv.Get());
			}

			for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

				de_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
					instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
			}

			// Draw the wireframe ONLY for the mesh mask depth tex

			lazyUnbind(de_ctx3);

			// Input Assembly
			{
				de_ctx3->IASetInputLayout(mesh_il.Get());

				std::array<ID3D11Buffer*, 2> buffers = {
					vbuff.buff.Get(), instabuff.buff.Get()
				};
				std::array<uint32_t, 2> strides = {
					sizeof(GPU_MeshVertex), sizeof(GPU_MeshInstance)
				};
				std::array<uint32_t, 2> offsets = {
					0, 0
				};
				de_ctx3->IASetVertexBuffers(0, buffers.size(), buffers.data(), strides.data(), offsets.data());
			}

			// Vertex Shader
			{
				de_ctx3->VSSetConstantBuffers(0, 1, frame_ubuff.buff.GetAddressOf());
				de_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
			}		

			// Rasterization
			de_ctx3->RSSetState(wireframe_none_bias_rs.Get());

			// Pixel Shader
			{
				std::array<ID3D11Buffer*, 2> buffs = {
					frame_ubuff.buff.Get(), drawcall_ubuff.buff.Get()
				};
				de_ctx3->PSSetConstantBuffers(0, buffs.size(), buffs.data());

				std::array<ID3D11ShaderResourceView*, 1> srvs = {
					mesh_mask_srv.Get()
				};
				de_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

				switch (application.shading_normal) {
				case ShadingNormal::VERTEX:
				case ShadingNormal::POLY: {
					de_ctx3->PSSetShader(dim_wireframe_ps.Get(), nullptr, 0);
					break;
				}
				case ShadingNormal::TESSELATION: {
					de_ctx3->PSSetShader(wireframe_with_tesselation_ps.Get(), nullptr, 0);
					break;
				}
				}
			}

			// Output Merger
			{
				de_ctx3->OMSetDepthStencilState(greater_dss.Get(), 1);

				float blend_factor[4] = { 1, 1, 1, 1 };
				de_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 2> rtvs = {
					event.compose_rtv, nullptr
				};
				de_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), wireframe_dsv.Get());
			}

			for (MeshInstanceSet& instance : drawcall.mesh_instance_sets) {

				de_ctx3->DrawInstanced(instance.mesh->mesh_vertex_count, instance.mesh_insta_count,
					instance.mesh->mesh_vertex_start, instance.mesh_insta_start);
			}

			break;
		}
		}
	}

	return err_stack;
}

ErrStack geometryDraw(nui::SurfaceEvent& event)
{
	MeshRenderer* r = (MeshRenderer*)event.user_data;
	return r->draw(event);
}