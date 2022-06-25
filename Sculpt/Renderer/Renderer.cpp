#include "./Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>

// Mine
#include <App/Application.hpp>
#include "RenderDocIntegration.hpp"


MeshRenderer renderer;


void checkDX11(HRESULT result)
{
	if (result != S_OK) {
		__debugbreak();
	}
}


// vertex buffer has one extra vertex, which will be the deleted vertex
// cpu vertex index == gpu vertex index + 1
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

		// Update Mesh Data
		{
			if (sculpt_mesh.dirty_vertex_list) {
				sculpt_mesh.uploadVertexAddsRemoves();
			}

			if (sculpt_mesh.dirty_vertex_pos) {
				sculpt_mesh.uploadVertexPositions();
			}

			if (sculpt_mesh.dirty_index_buff) {
				sculpt_mesh.uploadIndexBufferChanges();
			}

			if (sculpt_mesh.dirty_tess_tris) {
				sculpt_mesh.uploadTesselationTriangles(scme::TesselationModificationBasis::MODIFIED_POLYS);
			}

			if (app.shading_normal == GPU_ShadingNormal::VERTEX &&
				sculpt_mesh.dirty_vertex_normals)
			{
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

void MeshRenderer::init(bool enable_render_doc)
{
	// RenderDoc
	auto init_render_doc = [&]() {

		HMODULE module = GetModuleHandleA("renderdoc.dll");
		if (module == nullptr) {
			printf("renderdoc.dll could not be located");
			return;
		}

		pRENDERDOC_GetAPI getApi = (pRENDERDOC_GetAPI)GetProcAddress(module, "RENDERDOC_GetAPI");
		if (getApi == nullptr) {
			printf("Could not find GetAPI procedure in the RenderDoc DLL \n");
			return;
		}

		if (!getApi(eRENDERDOC_API_Version_1_4_1, (void**)&render_doc)) {
			printf("Could not get RenderDoc API \n");
			return;
		}

		int32_t major;
		int32_t minor;
		int32_t patch;
		render_doc->GetAPIVersion(&major, &minor, &patch);
		printf("RenderDoc Integration Active API Version %d.%d.%d \n", major, minor, patch);
	};

	if (enable_render_doc) {
		init_render_doc();
	}

	// Factory
	{
		if (CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory_2.GetAddressOf())) != S_OK) {
			__debugbreak();
		}

		if (factory_2->QueryInterface(factory.GetAddressOf()) != S_OK) {
			__debugbreak();
		}
	}

	// Adapter
	{
		IDXGIAdapter* found_adapter;

		while (factory->EnumAdapterByGpuPreference(
			(uint32_t)adapters.size(), DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&found_adapter)) == S_OK) {

			auto& adapter = adapters.emplace_back();
			adapter.index = (uint32_t)adapters.size() - 1;
			adapter.adapter = found_adapter;

			found_adapter->GetDesc(&adapter.desc);
		}

		std::sort(adapters.begin(), adapters.end(), [](Adapter& left, Adapter& right) -> bool {
			return left.desc.DedicatedVideoMemory > right.desc.DedicatedVideoMemory;
			});
	}
	auto& adapter = adapters.front().adapter;

	// Device and Context
	{
		std::array<D3D_FEATURE_LEVEL, 1> features = {
			D3D_FEATURE_LEVEL_11_1
		};

		checkDX11(D3D11CreateDevice(adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,  // driver type chosen by adapter
			NULL,  // no software adapter
			D3D11_CREATE_DEVICE_DEBUG,
			features.data(), (uint32_t)features.size(),
			D3D11_SDK_VERSION,
			dev1.GetAddressOf(),
			NULL,  // dont care feature level
			im_ctx1.GetAddressOf()));

		checkDX11(dev1->QueryInterface<ID3D11Device5>(dev.GetAddressOf()));
		checkDX11(im_ctx1->QueryInterface<ID3D11DeviceContext3>(im_ctx.GetAddressOf()));
	}

	// Swapchain
	{
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.Stereo = false;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.Scaling = DXGI_SCALING_NONE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		desc.Flags = 0;

		checkDX11(factory->CreateSwapChainForHwnd(
			dev.Get(),
			app.window.hwnd,
			&desc,
			nullptr,
			nullptr,
			swapchain1.GetAddressOf()
		));

		swapchain_backbuffer_index = 0;
	}

	// Scene Depth Resource
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		scene_dtex.create(dev.Get(), im_ctx.Get(), desc);

		D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
		view_desc.Format = DXGI_FORMAT_D32_FLOAT;
		view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipSlice = 0;

		scene_dtex.createDepthStencilView(view_desc);
	}

	// Mesh Depth Mash Resource
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		mesh_mask_dtex.create(dev.Get(), im_ctx.Get(), desc);

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
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		wireframe_dtex.create(dev.Get(), im_ctx.Get(), desc);

		D3D11_DEPTH_STENCIL_VIEW_DESC view_desc = {};
		view_desc.Format = DXGI_FORMAT_D32_FLOAT;
		view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipSlice = 0;

		wireframe_dtex.createDepthStencilView(view_desc);
	}

	// World Position Textures
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		world_pos_tex.create(dev.Get(), im_ctx.Get(), desc);

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

		world_pos_cputex.create(dev.Get(), im_ctx.Get(), desc);
	}

	// Staging Buffer
	{
		staging_buff.create(dev.Get(), im_ctx.Get());
	}

	// Frame Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		frame_ubuff.create(dev.Get(), im_ctx.Get(), desc);
	}

	// PBR Mesh Vertex Shader
	{
		dx11::createVertexShaderFromPath("Sculpt/CompiledShaders/MeshVS.cso", dev.Get(),
			mesh_vs.GetAddressOf(), &shader_cso);
	}

	// Mesh Geometry Shader
	{
		ErrStack err_stack = io::readLocalFile("Sculpt/CompiledShaders/MeshGS.cso", shader_cso);
		if (err_stack.isBad()) {
			throw std::exception("geometry shader code not found");
		}

		throwDX11(dev->CreateGeometryShader(shader_cso.data(), shader_cso.size(),
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

		mesh_rs.create(dev.Get(), desc);

		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthBias = 0;
		mesh_none_rs.create(dev.Get(), desc);

		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_BACK;
		desc.DepthBias = depth_bias;
		wire_bias_rs.create(dev.Get(), desc);

		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthBias = depth_bias;
		wire_none_bias_rs.create(dev.Get(), desc);
	}

	// Pixel Shaders
	{
		// PBR Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshPS.cso", dev.Get(),
			mesh_ps.GetAddressOf(), &shader_cso);

		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/MeshDepthOnlyPS.cso", dev.Get(),
			mesh_depth_only_ps.GetAddressOf(), &shader_cso);

		// Wireframe Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/WirePS.cso", dev.Get(),
			wire_ps.GetAddressOf(), &shader_cso);

		// See Thru Wireframe Pixel Shader
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/SeeThruWirePS.cso", dev.Get(),
			see_thru_wire_ps.GetAddressOf(), &shader_cso);

		// AABB
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/AABB_PS.cso", dev.Get(),
			aabb_ps.GetAddressOf(), &shader_cso);

		// Debug
		dx11::createPixelShaderFromPath("Sculpt/CompiledShaders/DebugPS.cso", dev.Get(),
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

		throwDX11(dev->CreateBlendState(&desc, blend_target_0_bs.GetAddressOf()));

		for (uint8_t i = 0; i < 8; i++) {
			desc.RenderTarget[i].BlendEnable = false;
			desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		throwDX11(dev->CreateBlendState(&desc, blendless_bs.GetAddressOf()));
	}

	// Mesh Depth Stencil
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = false;

		throwDX11(dev->CreateDepthStencilState(&desc, depth_stencil.GetAddressOf()),
			"failed to create mesh depth stencil state");
	}

	// Compute Shaders
	{
		dx11::createComputeShaderFromPath("Sculpt/CompiledShaders/DistributeAABB_VertsCS.cso", dev.Get(),
			distribute_AABB_verts_cs.GetAddressOf(), &shader_cso);

		dx11::createComputeShaderFromPath("Sculpt/CompiledShaders/UpdateVertexPositionsCS.cso", dev.Get(),
			update_vertex_positions_cs.GetAddressOf(), &shader_cso);

		dx11::createComputeShaderFromPath("Sculpt/CompiledShaders/UpdateVertexNormalsCS.cso", dev.Get(),
			update_vertex_normals_cs.GetAddressOf(), &shader_cso);

		dx11::createComputeShaderFromPath("Sculpt/CompiledShaders/UpdateTesselationTriangles.cso", dev.Get(),
			update_tesselation_triangles.GetAddressOf(), &shader_cso);
	}

	// Mesh Compute Data Constant Buffer
	{
		mesh_aabb_graph.create(dev.Get(), im_ctx.Get(), D3D11_USAGE_DYNAMIC, D3D10_CPU_ACCESS_WRITE);
		mesh_aabb_graph.addUint();
		mesh_aabb_graph.addUint();
	}

	// Vertex AABB placements
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_unplaced_verts.create(dev.Get(), im_ctx.Get(), desc);

		//
		desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_placed_verts.create(dev.Get(), im_ctx.Get(), desc);
	}

	// Vertex Position Update Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_vert_pos_updates.create(dev.Get(), im_ctx.Get(), desc);
	}

	// Vertex Normal Update Buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_vert_normal_updates.create(dev.Get(), im_ctx.Get(), desc);
	}

	// Poly Normal Update Buffers
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_poly_normal_updates.create(dev.Get(), im_ctx.Get(), desc);

		desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		gpu_r_poly_normal_updates.create(dev.Get(), im_ctx.Get(), desc);
	}
}

void MeshRenderer::loadUniform()
{
	GPU_MeshUniform uniform;
	uniform.camera_pos = dxConvert(app.camera.pos);
	uniform.camera_quat = dxConvert(app.camera.quat_inv);
	uniform.camera_forward = dxConvert(app.camera.forward);

	uniform.perspective_matrix = DirectX::XMMatrixPerspectiveFovRH(
		toRad(app.camera.field_of_view),
		(float)render_target_width / (float)render_target_height,
		app.camera.z_near, app.camera.z_far);

	uniform.z_near = app.camera.z_near;
	uniform.z_far = app.camera.z_far;

	uint32_t i = 0;
	for (CameraLight& light : app.lights) {

		// Rotate the light normal to be relative to camera orientation
		glm::vec3 normal = light.normal * app.camera.quat_inv;
		normal = glm::normalize(normal);

		GPU_CameraLight& gpu_light = uniform.lights[i];
		gpu_light.normal = dxConvert(normal);
		gpu_light.color = dxConvert(light.color);
		gpu_light.intensity = light.intensity;
		
		i++;
	}

	uniform.ambient_intensity = app.ambient_intensity;

	uniform.shading_normal = app.shading_normal;

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
	if (world_pos_cputex.tex == nullptr) {
		return;
	}

	D3D11_BOX box = {};
	box.top = y;
	box.bottom = box.top + 1;
	box.left = x;
	box.right = box.left + 1;
	box.front = 0;
	box.back = 1;

	im_ctx->CopySubresourceRegion(world_pos_cputex.get(), 0,
		0, 0, 0,
		world_pos_tex.get(), 0,
		&box
	);

	std::array<uint32_t, 4> rgba;
	world_pos_cputex.readPixel(0, 0, rgba);
	
	// copy only RGB and ignore A
	std::memcpy(&r_world_pos, rgba.data(), sizeof(uint32_t) * 3);
}

void MeshRenderer::render()
{
	loadVertices();
	loadUniform();

	if (render_target_width != app.viewport.width ||
		render_target_height != app.viewport.height)
	{
		// Swapchain
		{
			swapchain_texs[0] = nullptr;
			swapchain_rtvs[0] = nullptr;

			checkDX11(swapchain1->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

			checkDX11(swapchain1->GetBuffer(
				0, IID_PPV_ARGS(swapchain_texs[0].GetAddressOf()))
			);
			checkDX11(dev->CreateRenderTargetView(
				swapchain_texs[0].Get(), NULL, swapchain_rtvs[0].GetAddressOf())
			);
		}

		scene_dtex.resize(app.viewport.width, app.viewport.height);
		mesh_mask_dtex.resize(app.viewport.width, app.viewport.height);
		wireframe_dtex.resize(app.viewport.width, app.viewport.height);
		world_pos_tex.resize(app.viewport.width, app.viewport.height);

		loadUniform();

		this->render_target_width = app.viewport.width;
		this->render_target_height = app.viewport.height;
	}

	// Choose swapchain backbuffer
	swapchain_backbuffer_index = (swapchain_backbuffer_index + 1) % 1;
	present_rtv = swapchain_rtvs[swapchain_backbuffer_index].Get();

	// Rendering Commands ///////////////////////////////////////////////////////////////////////////////////
	im_ctx->ClearState();

	float clear_transparent[4] = {0, 0, 0, 0};
	im_ctx->ClearRenderTargetView(present_rtv, clear_transparent);

	im_ctx->ClearDepthStencilView(scene_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);
	{
		float clear_pos[4] = {
			FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX
		};
		im_ctx->ClearRenderTargetView(world_pos_tex.getRTV(), clear_pos);

		float clear_mesh_mask[4] = {
			1.f, 0.f, 0.f, 0.f
		};
		im_ctx->ClearRenderTargetView(mesh_mask_dtex.getRTV(), clear_mesh_mask);
	}

	// Input Assembly
	im_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Vertex Shader
	{
		std::array<ID3D11Buffer*, 1> buffers = {
			frame_ubuff.buff.Get()
		};
		im_ctx->VSSetConstantBuffers(0, buffers.size(), buffers.data());
	}

	// Rasterizer Viewport
	{
		D3D11_VIEWPORT viewport = {};
		viewport.Width = (float)render_target_width;
		viewport.Height = (float)render_target_height;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		im_ctx->RSSetViewports(1, &viewport);

		D3D11_RECT sccissor = {};
		sccissor.right = sccissor.left + render_target_width;
		sccissor.bottom = sccissor.top + render_target_height;

		im_ctx->RSSetScissorRects(1, &sccissor);
	}

	// Pixel Shader
	{
		std::array<ID3D11Buffer*, 1> buffers = {
			frame_ubuff.buff.Get()
		};
		im_ctx->PSSetConstantBuffers(0, buffers.size(), buffers.data());
	}

	// Solid and Wireframe Overlay
	for (Mesh& mesh : app.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Input Assembly (mesh is the same for all instances)
		im_ctx->IASetIndexBuffer(sculpt_mesh.gpu_indexes.get(), DXGI_FORMAT_R32_UINT, 0);

		for (MeshInstanceSet& set : mesh.sets) {
			
			MeshDrawcall* drawcall = set.drawcall;

			// Clear
			{
				// Geometry
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						nullptr, nullptr
					};

					im_ctx->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->GSSetShader(nullptr, nullptr, 0);
				}
			}

			// Just draw a solid mesh
			auto draw_solid_mesh = [&]() {

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_verts.getSRV(),
						set.gpu_instances_srv.Get()
					};
					im_ctx->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterization
				if (drawcall->is_back_culled) {
					im_ctx->RSSetState(mesh_rs.get());
				}
				else {
					im_ctx->RSSetState(mesh_none_rs.get());
				}

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_triangles.getSRV(),
						set.gpu_instances_srv.Get()
					};
					im_ctx->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->PSSetShader(mesh_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						present_rtv, world_pos_tex.getRTV()
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.capacity(), set.gpu_instances.capacity(),
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
						sculpt_mesh.gpu_triangles.getSRV(),
					};
					im_ctx->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->GSSetShader(mesh_gs.Get(), nullptr, 0);
				}

				// Rasterization
				if (drawcall->is_back_culled) {
					im_ctx->RSSetState(wire_bias_rs.get());
				}
				else {
					im_ctx->RSSetState(wire_none_bias_rs.get());
				}

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						set.gpu_instances_srv.Get(), nullptr
					};
					im_ctx->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->PSSetShader(wire_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						present_rtv, nullptr
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.capacity(), set.gpu_instances.capacity(),
					0, 0, 0);
				break;
			}

			case DisplayMode::WIREFRANE: {
 
				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_verts.getSRV(),
						set.gpu_instances_srv.Get()
					};
					im_ctx->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterization
				im_ctx->RSSetState(mesh_none_rs.get());

				// Pixel Shader
				im_ctx->PSSetShader(mesh_depth_only_ps.Get(), nullptr, 0);

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx->OMSetBlendState(blendless_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						mesh_mask_dtex.getRTV(),
						world_pos_tex.getRTV()
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());  // scene_dtex.getDSV()
				}

				im_ctx->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.capacity(), set.gpu_instances.capacity(),
					0, 0, 0);

				break;
			}
			}

			// Render AABBs 
			if (mesh.aabb_render_mode != AABB_RenderMode::NO_RENDER) {

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_aabb_verts.getSRV(),
						set.gpu_instances_srv.Get()
					};
					im_ctx->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Rasterizer
				im_ctx->RSSetState(wire_none_bias_rs.get());

				// Pixel Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						set.gpu_instances_srv.Get()
					};
					im_ctx->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->PSSetShader(aabb_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					im_ctx->OMSetDepthStencilState(depth_stencil.Get(), 1);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						present_rtv, nullptr
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), scene_dtex.getDSV());
				}

				im_ctx->DrawInstanced(sculpt_mesh.aabb_verts.size(), set.gpu_instances.capacity(),
					0, 0);
			}
		}
	}

	// Wireframe Drawcalls
	im_ctx->ClearDepthStencilView(wireframe_dtex.getDSV(), D3D11_CLEAR_DEPTH, 1, 0);

	for (Mesh& mesh : app.meshes) {

		scme::SculptMesh& sculpt_mesh = mesh.mesh;

		// Input Assembly (mesh is the same for all instances)
		im_ctx->IASetIndexBuffer(sculpt_mesh.gpu_indexes.get(), DXGI_FORMAT_R32_UINT, 0);

		for (MeshInstanceSet& set : mesh.sets) {

			MeshDrawcall* drawcall = set.drawcall;

			// Clear
			{
				// Geometry
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						nullptr, nullptr
					};

					im_ctx->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->GSSetShader(nullptr, nullptr, 0);
				}
			}

			switch (drawcall->display_mode) {
			case DisplayMode::WIREFRANE: {

				// Same Input Assembly

				// Vertex Shader
				{
					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						sculpt_mesh.gpu_verts.getSRV(),
						set.gpu_instances_srv.Get()
					};
					im_ctx->VSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->VSSetShader(mesh_vs.Get(), nullptr, 0);
				}

				// Geometry Shader
				{
					std::array<ID3D11ShaderResourceView*, 1> srvs = {
						sculpt_mesh.gpu_triangles.getSRV(),
					};
					im_ctx->GSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->GSSetShader(mesh_gs.Get(), nullptr, 0);
				}

				// Rasterization
				im_ctx->RSSetState(wire_none_bias_rs.get());

				// Pixel Shader
				{
					std::array<ID3D11RenderTargetView*, 1> rtvs = {
						nullptr
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), nullptr);

					std::array<ID3D11ShaderResourceView*, 2> srvs = {
						mesh_mask_dtex.getSRV(), set.gpu_instances_srv.Get()
					};
					im_ctx->PSSetShaderResources(0, srvs.size(), srvs.data());

					im_ctx->PSSetShader(see_thru_wire_ps.Get(), nullptr, 0);
				}

				// Output Merger
				{
					im_ctx->OMSetDepthStencilState(depth_stencil.Get(), 1);

					float blend_factor[4] = { 1, 1, 1, 1 };
					im_ctx->OMSetBlendState(blend_target_0_bs.Get(), blend_factor, 0xFFFF'FFFF);

					std::array<ID3D11RenderTargetView*, 2> rtvs = {
						present_rtv, nullptr
					};
					im_ctx->OMSetRenderTargets(rtvs.size(), rtvs.data(), wireframe_dtex.getDSV());
				}

				im_ctx->DrawIndexedInstanced(sculpt_mesh.gpu_indexes.capacity(), set.gpu_instances.capacity(),
					0, 0, 0);
				break;
			}
			}
		}
	}

	swapchain1->Present(0, 0);

	// Clear previous cached pixel world position
	_cached_pixel_world_pos.clear();
}