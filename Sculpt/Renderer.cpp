
// Header
#include "Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"


void MeshRenderer::loadVertices()
{
	DirectX::XMFLOAT3 gpu_aabbs_positions[8];

	for (Mesh& mesh : application.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Polygons
		// polygons normals need to be generated before
		// so they can be used to generate vertex normal
		{
			if (sculpt_mesh.modified_polys.size() > 0) {

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

				for (scme::ModifiedPoly& modified_poly : sculpt_mesh.modified_polys) {

					scme::Poly& poly = sculpt_mesh.polys[modified_poly.idx];

					switch (modified_poly.state) {
					case scme::ModifiedPolyState::UPDATE: {

						GPU_MeshTriangle tris;

						if (poly.is_tris) {

							uint32_t vs_idx[3];
							scme::Vertex* vs[3];
							sculpt_mesh.getTrisPrimitives(&poly, vs_idx, vs);

							gpu_indexs.update(6 * modified_poly.idx + 0, vs_idx[0]);
							gpu_indexs.update(6 * modified_poly.idx + 1, vs_idx[1]);
							gpu_indexs.update(6 * modified_poly.idx + 2, vs_idx[2]);

							uint32_t zero = 0;
							gpu_indexs.update(6 * modified_poly.idx + 3, zero);
							gpu_indexs.update(6 * modified_poly.idx + 4, zero);
							gpu_indexs.update(6 * modified_poly.idx + 5, zero);

							// Triangle Normal
							poly.normal = sculpt_mesh.calcWindingNormal(vs[0], vs[1], vs[2]);

							// GPU Triangle
							tris.poly_normal = dxConvert(poly.normal);
							tris.tess_normal = dxConvert(poly.normal);
							tris.tess_vertex_0 = 0;
							tris.tess_vertex_1 = 0;
							gpu_triangles.update(2 * modified_poly.idx, tris);
						}
						else {
							uint32_t vs_idx[4];
							scme::Vertex* vs[4];
							sculpt_mesh.getQuadPrimitives(&poly, vs_idx, vs);

							// Tesselation and Normals
							if (glm::distance(vs[0]->pos, vs[2]->pos) < glm::distance(vs[1]->pos, vs[3]->pos)) {

								poly.tesselation_type = 0;
								poly.tess_normals[0] = sculpt_mesh.calcWindingNormal(vs[0], vs[1], vs[2]);
								poly.tess_normals[1] = sculpt_mesh.calcWindingNormal(vs[0], vs[2], vs[3]);

								gpu_indexs.update(6 * modified_poly.idx + 0, vs_idx[0]);
								gpu_indexs.update(6 * modified_poly.idx + 1, vs_idx[2]);
								gpu_indexs.update(6 * modified_poly.idx + 2, vs_idx[3]);

								gpu_indexs.update(6 * modified_poly.idx + 3, vs_idx[0]);
								gpu_indexs.update(6 * modified_poly.idx + 4, vs_idx[1]);
								gpu_indexs.update(6 * modified_poly.idx + 5, vs_idx[2]);

								tris.tess_vertex_0 = vs_idx[0];
								tris.tess_vertex_1 = vs_idx[2];
							}
							else {
								poly.tesselation_type = 1;
								poly.tess_normals[0] = sculpt_mesh.calcWindingNormal(vs[0], vs[1], vs[3]);
								poly.tess_normals[1] = sculpt_mesh.calcWindingNormal(vs[1], vs[2], vs[3]);

								gpu_indexs.update(6 * modified_poly.idx + 0, vs_idx[0]);
								gpu_indexs.update(6 * modified_poly.idx + 1, vs_idx[1]);
								gpu_indexs.update(6 * modified_poly.idx + 2, vs_idx[3]);

								gpu_indexs.update(6 * modified_poly.idx + 3, vs_idx[1]);
								gpu_indexs.update(6 * modified_poly.idx + 4, vs_idx[2]);
								gpu_indexs.update(6 * modified_poly.idx + 5, vs_idx[3]);

								tris.tess_vertex_0 = vs_idx[1];
								tris.tess_vertex_1 = vs_idx[3];
							}
							poly.normal = glm::normalize((poly.tess_normals[0] + poly.tess_normals[1]) / 2.f);

							// GPU Triangles
							tris.poly_normal = dxConvert(poly.normal);
							tris.tess_normal = dxConvert(poly.tess_normals[0]);
							gpu_triangles.update(2 * modified_poly.idx, tris);

							tris.tess_normal = dxConvert(poly.tess_normals[1]);
							gpu_triangles.update(2 * modified_poly.idx + 1, tris);
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

				sculpt_mesh.modified_polys.clear();

				// Shader Resource View
				sculpt_mesh.gpu_triangles_srv = nullptr;

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

		// AABBs
		if (mesh.render_aabbs &&
			(sculpt_mesh.modified_verts.size() || sculpt_mesh.aabb_vbuff.buff == nullptr))
		{
			auto& gpu_aabb_verts = sculpt_mesh.gpu_aabb_verts;
			gpu_aabb_verts.resize(sculpt_mesh.aabbs.size() * 36);

			uint32_t vertex_idx = 0;
			for (scme::VertexBoundingBox& aabb : sculpt_mesh.aabbs) {

				glm::vec3& min = aabb.aabb.min;
				glm::vec3& max = aabb.aabb.max;

				// Forward (Classic winding)
				gpu_aabbs_positions[0] = dxConvert(glm::vec3{ min.x, max.y, max.z });  // top left
				gpu_aabbs_positions[1] = dxConvert(glm::vec3{ max.x, max.y, max.z });  // top right
				gpu_aabbs_positions[2] = dxConvert(glm::vec3{ max.x, min.y, max.z });  // bot right
				gpu_aabbs_positions[3] = dxConvert(glm::vec3{ min.x, min.y, max.z });  // bot left

				// Backward
				gpu_aabbs_positions[4] = dxConvert(glm::vec3{ min.x, max.y, min.z });  // top left
				gpu_aabbs_positions[5] = dxConvert(glm::vec3{ max.x, max.y, min.z });  // top right
				gpu_aabbs_positions[6] = dxConvert(glm::vec3{ max.x, min.y, min.z });  // bot right
				gpu_aabbs_positions[7] = dxConvert(glm::vec3{ min.x, min.y, min.z });  // bot left

				// Front Face
				gpu_aabb_verts[vertex_idx + 0].pos = gpu_aabbs_positions[0];
				gpu_aabb_verts[vertex_idx + 1].pos = gpu_aabbs_positions[1];
				gpu_aabb_verts[vertex_idx + 2].pos = gpu_aabbs_positions[3];

				gpu_aabb_verts[vertex_idx + 3].pos = gpu_aabbs_positions[1];
				gpu_aabb_verts[vertex_idx + 4].pos = gpu_aabbs_positions[2];
				gpu_aabb_verts[vertex_idx + 5].pos = gpu_aabbs_positions[3];

				// Right Face
				gpu_aabb_verts[vertex_idx + 6].pos = gpu_aabbs_positions[1];
				gpu_aabb_verts[vertex_idx + 7].pos = gpu_aabbs_positions[5];
				gpu_aabb_verts[vertex_idx + 8].pos = gpu_aabbs_positions[2];

				gpu_aabb_verts[vertex_idx + 9].pos = gpu_aabbs_positions[5];
				gpu_aabb_verts[vertex_idx + 10].pos = gpu_aabbs_positions[6];
				gpu_aabb_verts[vertex_idx + 11].pos = gpu_aabbs_positions[2];

				// Back Face
				gpu_aabb_verts[vertex_idx + 12].pos = gpu_aabbs_positions[5];
				gpu_aabb_verts[vertex_idx + 13].pos = gpu_aabbs_positions[4];
				gpu_aabb_verts[vertex_idx + 14].pos = gpu_aabbs_positions[6];

				gpu_aabb_verts[vertex_idx + 15].pos = gpu_aabbs_positions[4];
				gpu_aabb_verts[vertex_idx + 16].pos = gpu_aabbs_positions[7];
				gpu_aabb_verts[vertex_idx + 17].pos = gpu_aabbs_positions[6];

				// Left Face
				gpu_aabb_verts[vertex_idx + 18].pos = gpu_aabbs_positions[4];
				gpu_aabb_verts[vertex_idx + 19].pos = gpu_aabbs_positions[0];
				gpu_aabb_verts[vertex_idx + 20].pos = gpu_aabbs_positions[7];

				gpu_aabb_verts[vertex_idx + 21].pos = gpu_aabbs_positions[0];
				gpu_aabb_verts[vertex_idx + 22].pos = gpu_aabbs_positions[3];
				gpu_aabb_verts[vertex_idx + 23].pos = gpu_aabbs_positions[7];

				// Top Face	
				gpu_aabb_verts[vertex_idx + 24].pos = gpu_aabbs_positions[4];
				gpu_aabb_verts[vertex_idx + 25].pos = gpu_aabbs_positions[5];
				gpu_aabb_verts[vertex_idx + 26].pos = gpu_aabbs_positions[0];

				gpu_aabb_verts[vertex_idx + 27].pos = gpu_aabbs_positions[5];
				gpu_aabb_verts[vertex_idx + 28].pos = gpu_aabbs_positions[1];
				gpu_aabb_verts[vertex_idx + 29].pos = gpu_aabbs_positions[0];

				// Back Face
				gpu_aabb_verts[vertex_idx + 30].pos = gpu_aabbs_positions[6];
				gpu_aabb_verts[vertex_idx + 31].pos = gpu_aabbs_positions[7];
				gpu_aabb_verts[vertex_idx + 32].pos = gpu_aabbs_positions[2];

				gpu_aabb_verts[vertex_idx + 33].pos = gpu_aabbs_positions[7];
				gpu_aabb_verts[vertex_idx + 34].pos = gpu_aabbs_positions[3];
				gpu_aabb_verts[vertex_idx + 35].pos = gpu_aabbs_positions[2];

				// Front Face
				gpu_aabb_verts[vertex_idx + 1].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 2].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 3].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 5].normal.x = 1;

				// Right Face
				gpu_aabb_verts[vertex_idx + 7].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 8].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 9].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 11].normal.x = 1;

				// Back Face
				gpu_aabb_verts[vertex_idx + 13].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 14].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 15].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 17].normal.x = 1;

				// Left Face
				gpu_aabb_verts[vertex_idx + 19].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 20].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 21].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 23].normal.x = 1;

				// Top Face
				gpu_aabb_verts[vertex_idx + 25].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 26].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 27].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 29].normal.x = 1;

				// Bot Face
				gpu_aabb_verts[vertex_idx + 31].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 32].normal.x = 1;

				gpu_aabb_verts[vertex_idx + 27].normal.x = 1;
				gpu_aabb_verts[vertex_idx + 35].normal.x = 1;

				// normal.xy is unused

				vertex_idx += 36;
			}

			if (sculpt_mesh.aabb_vbuff.buff == nullptr) {

				D3D11_BUFFER_DESC desc = {};
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				sculpt_mesh.aabb_vbuff.create(dev5, im_ctx3, desc);
			}

			sculpt_mesh.aabb_vbuff.load(
				sculpt_mesh.gpu_aabb_verts.data(),
				sculpt_mesh.gpu_aabb_verts.size() * sizeof(GPU_MeshVertex));
		}
		// Free GPU Memory
		else if (mesh.render_aabbs == false && sculpt_mesh.aabb_vbuff.buff != nullptr) {
			sculpt_mesh.aabb_vbuff.buff = nullptr;
		}

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

						sculpt_mesh.calcVertexNormal(modified_v.idx);

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

				// Clear
				sculpt_mesh.modified_verts.clear();
			}
		}

		// Instances
		{
			for (MeshInstanceSet& set : mesh.sets) {

				if (set.modified_instances.size()) {

					auto& gpu_instances = set.gpu_instances;

					if (gpu_instances.buff == nullptr) {
						gpu_instances.dev = this->dev5;
						gpu_instances.ctx3 = this->im_ctx3;
						gpu_instances.init_desc = {};
						gpu_instances.init_desc.Usage = D3D11_USAGE_DEFAULT;
						gpu_instances.init_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						gpu_instances.init_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
						gpu_instances.init_desc.StructureByteStride = sizeof(GPU_MeshInstance);
					}

					gpu_instances.resize(set.instances.capacity());

					for (ModifiedMeshInstance& modified_instance : set.modified_instances) {

						MeshInstance& instance = set.instances[modified_instance.idx];
						GPU_MeshInstance gpu_inst;

						switch (modified_instance.type) {
						case ModifiedInstanceType::UPDATE: {
						
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
							break;
						}

						case ModifiedInstanceType::DELETED: {
							gpu_inst.rot.w = 2.f;
							gpu_instances.update(modified_instance.idx, gpu_inst);
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
					desc.Buffer.NumElements = set.gpu_instances.size();

					throwDX11(dev5->CreateShaderResourceView(
						set.gpu_instances.get(), &desc, set.gpu_instances_srv.GetAddressOf()));
				}
			}
		}
	}
}

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

void MeshRenderer::setWireframeDepthBias(int32_t depth_bias)
{
	wire_bias_rs.setDepthBias(depth_bias);
	wire_none_bias_rs.setDepthBias(depth_bias);
}

void MeshRenderer::getPixelWorldPosition(int32_t x, int32_t y, glm::vec3& r_world_pos)
{
	D3D11_BOX box = {};
	box.top = y;
	box.bottom = box.top + 1;
	box.left = x;
	box.right = box.left + 1;
	box.front = 0;
	box.back = 1;

	im_ctx3->CopySubresourceRegion(world_pos_cputex.get(), 0,
		0, 0, 0,
		world_pos_tex.get(), 0,
		&box
	);

	std::array<uint32_t, 4> rgba;
	world_pos_cputex.readPixel(0, 0, rgba);
	
	// copy only RGB and ignore A
	std::memcpy(&r_world_pos, rgba.data(), sizeof(uint32_t) * 3);
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

		// World Position Textures
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = event.render_target_width;
			desc.Height = event.render_target_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			world_pos_tex.create(dev5, im_ctx3, desc);

			D3D11_RENDER_TARGET_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipSlice = 0;

			world_pos_tex.createRenderTargetView(view_desc);

			// CPU side
			desc = {};
			desc.Width = 1;
			desc.Height = 1;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			world_pos_cputex.create(dev5, im_ctx3, desc);
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

			std::vector<D3D11_INPUT_ELEMENT_DESC> nodes;
			nodes.resize(vertex_elems.size());

			uint32_t idx = 0;
			for (D3D11_INPUT_ELEMENT_DESC& elem : vertex_elems) {
				nodes[idx++] = elem;
			}

			throwDX11(dev5->CreateInputLayout(nodes.data(), nodes.size(),
				shader_cso.data(), shader_cso.size(), mesh_il.GetAddressOf()),
				"failed to create mesh vertex input layout");
		}

		// Mesh Geometry Shader
		{
			ErrStack err_stack = io::readLocalFile("Sculpt/CompiledShaders/MeshGS.cso", shader_cso);
			if (err_stack.isBad()) {
				throw std::exception("geometry shader code not found");
			}

			throwDX11(dev5->CreateGeometryShader(shader_cso.data(), shader_cso.size(),
				nullptr, mesh_gs.GetAddressOf()));
		}

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
		}

		// Pixel Shaders
		{
			// PBR Pixel Shader
			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshPS.cso", dev5,
				mesh_ps.GetAddressOf(), &shader_cso);

			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshDepthOnlyPS.cso", dev5,
				mesh_depth_only_ps.GetAddressOf(), &shader_cso);

			// Wireframe Pixel Shader
			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/WirePS.cso", dev5,
				wire_ps.GetAddressOf(), &shader_cso);

			// See Thru Wireframe Pixel Shader
			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/SeeThruWirePS.cso", dev5,
				see_thru_wire_ps.GetAddressOf(), &shader_cso);

			// AABB
			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/AABB_PS.cso", dev5,
				aabb_ps.GetAddressOf(), &shader_cso);

			// Debug
			dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/DebugPS.cso", dev5,
				debug_ps.GetAddressOf(), &shader_cso);
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
		world_pos_tex.resize(event.render_target_width, event.render_target_height);

		loadUniform();

		this->render_target_width = event.render_target_width;
		this->render_target_height = event.render_target_height;
	}

	// Rendering Commands ///////////////////////////////////////////////////////////////////////////////////

	im_ctx3->ClearDepthStencilView(scene_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);
	{
		float clear_pos[4] = {
			FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX
		};
		im_ctx3->ClearRenderTargetView(world_pos_tex.getRTV(), clear_pos);

		float clear_mesh_mask[4] = {
			1.f, 0.f, 0.f, 0.f
		};
		im_ctx3->ClearRenderTargetView(mesh_mask_dtex.getRTV(), clear_mesh_mask);
	}

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

	// Solid and Wireframe Overlay
	for (Mesh& mesh : application.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Input Assembly (mesh is the same for all instances)
		{
			std::array<ID3D11Buffer*, 1> buffs = {
				sculpt_mesh.gpu_verts.get()
			};

			std::array<uint32_t, 1> strides = {
				sizeof(GPU_MeshVertex)
			};

			std::array<uint32_t, 2> offsets = {
				0
			};

			im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(),
				strides.data(), offsets.data());

			im_ctx3->IASetIndexBuffer(sculpt_mesh.gpu_indexes.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		for (MeshInstanceSet& set : mesh.sets) {
			
			MeshDrawcall* drawcall = set.drawcall;

			// Clear
			{
				// Geometry
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						nullptr, nullptr
					};

					im_ctx3->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->GSSetShader(nullptr, nullptr, 0);
				}
			}

			// Just draw a solid mesh
			auto draw_solid_mesh = [&]() {

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterization
				if (drawcall->is_back_culled) {
					im_ctx3->RSSetState(mesh_rs.get());
				}
				else {
					im_ctx3->RSSetState(mesh_none_rs.get());
				}

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_triangles_srv.Get(), set.gpu_instances_srv.Get()
					};
					im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						event.compose_rtv, world_pos_tex.getRTV()
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx3->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.size(), set.gpu_instances.size(),
					0, 0, 0);
			};

			switch (drawcall->display_mode) {
			case DisplayMode::SOLID: {
				draw_solid_mesh();
				break;
			}

			case DisplayMode::WIREFRAME_OVERLAY: {
				// Overview
				// - Draw the meshes using the PBR mesh shader
				// - Draw the wireframe on top offset by depth bias

				draw_solid_mesh();

				/////////////////////////////////////////////////////////////////////////////////////
				// Draw the wireframe on top offset by depth bias

				// Same Input Assembly
				// Same Vertex Shader

				// Geometry Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						sculpt_mesh.gpu_triangles_srv.Get()
					};
					im_ctx3->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->GSSetShader(mesh_gs.Get(), nullptr, 0);
				}

				// Rasterization
				if (drawcall->is_back_culled) {
					im_ctx3->RSSetState(wire_bias_rs.get());
				}
				else {
					im_ctx3->RSSetState(wire_none_bias_rs.get());
				}

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						set.gpu_instances_srv.Get(), nullptr
					};
					im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->PSSetShader(wire_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						event.compose_rtv, nullptr
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx3->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.size(), set.gpu_instances.size(),
					0, 0, 0);
				break;
			}

			case DisplayMode::WIREFRANE: {
 
				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterization
				im_ctx3->RSSetState(mesh_none_rs.get());

				// Pixel Shader
				im_ctx3->PSSetShader(mesh_depth_only_ps.Get(), nullptr, 0);

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blendless_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						mesh_mask_dtex.getRTV(),
						world_pos_tex.getRTV()
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());  // scene_dtex.getDSV()
				}

				im_ctx3->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.size(), set.gpu_instances.size(),
					0, 0, 0);

				break;
			}
			}

			// Render AABBs
			if (mesh.render_aabbs) {

				// Input Assembly
				{
					std::array<ID3D11Buffer*, 1> buffs = {
						sculpt_mesh.aabb_vbuff.get()
					};

					std::array<uint32_t, 1> strides = {
						sizeof(GPU_MeshVertex)
					};

					std::array<uint32_t, 2> offsets = {
						0
					};

					im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(),
						strides.data(), offsets.data());

					// Index buffer unused
				}

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterizer
				im_ctx3->RSSetState(wire_none_bias_rs.get());

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->PSSetShader(aabb_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						event.compose_rtv, nullptr
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx3->DrawInstanced(sculpt_mesh.gpu_aabb_verts.size(), set.gpu_instances.size(),
					0, 0);
			}
		}
	}

	// Wireframe Drawcalls
	im_ctx3->ClearDepthStencilView(wireframe_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);

	for (Mesh& mesh : application.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Input Assembly (mesh is the same for all instances)
		{
			std::array<ID3D11Buffer*, 1> buffs = {
				sculpt_mesh.gpu_verts.get()
			};

			std::array<uint32_t, 1> strides = {
				sizeof(GPU_MeshVertex)
			};

			std::array<uint32_t, 2> offsets = {
				0
			};

			im_ctx3->IASetVertexBuffers(0, buffs.size(), buffs.data(),
				strides.data(), offsets.data());

			im_ctx3->IASetIndexBuffer(sculpt_mesh.gpu_indexes.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		for (MeshInstanceSet& set : mesh.sets) {

			MeshDrawcall* drawcall = set.drawcall;

			// Clear
			{
				// Geometry
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						nullptr, nullptr
					};

					im_ctx3->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->GSSetShader(nullptr, nullptr, 0);
				}
			}

			switch (drawcall->display_mode) {
			case DisplayMode::WIREFRANE: {

				// Same Input Assembly

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx3->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Geometry Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						sculpt_mesh.gpu_triangles_srv.Get()
					};
					im_ctx3->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->GSSetShader(mesh_gs.Get(), nullptr, 0);
				}

				// Rasterization
				im_ctx3->RSSetState(wire_none_bias_rs.get());

				// Pixel Shader
				{
					std::array<ID3D11RenderTargetView*, 1> rtvs = {
						nullptr
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), nullptr);

					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						mesh_mask_dtex.getSRV(), set.gpu_instances_srv.Get()
					};
					im_ctx3->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx3->PSSetShader(see_thru_wire_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					im_ctx3->OMSetDepthStencilState(depth_stencil.Get(), 1);

					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx3->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						event.compose_rtv, nullptr
					};
					im_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), wireframe_dtex.getDSV());
				}

				im_ctx3->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.size(), set.gpu_instances.size(),
					0, 0, 0);
				break;
			}
			}
		}
	}
}

void geometryDraw(nui::Window*, nui::StoredElement*, nui::SurfaceEvent& event, void*)
{
	application.renderer.draw(event);
}