
// Header
#include "Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>


using namespace scme;


void MeshRenderer::generateVertices()
{
	SculptMesh& mesh = application.mesh;

	// Vertices
	uint32_t vertex_count = 0;
	for (VertexBoundingBox& aabb : mesh.aabbs) {
		for (GPU_Vertex& gv : aabb.gpu_vs) {
			vertex_count++;
		}
	}
	vertices.resize(vertex_count);

	uint32_t vi = 0;
	for (VertexBoundingBox& aabb : mesh.aabbs) {
		for (GPU_Vertex& gv : aabb.gpu_vs) {
			vertices[vi].pos = dxConvert(gv.pos);
			vi++;
		}
	}

	// Instances
	instances.resize(1);
	instances[0].pos = dxConvert(mesh.pos);
	instances[0].rot = dxConvert(mesh.rot);
}

void MeshRenderer::generateUniform()
{
	uniform.camera_pos = dxConvert(application.camera_pos);

	uniform.camera_quat = dxConvert(application.camera_quat);

	uniform.perspective_matrix = dxConvert(glm::perspectiveFovRH_ZO(toRad(application.field_of_view),
		(float)viewport_width, (float)viewport_height, application.z_near, application.z_far));
}

ErrStack MeshRenderer::loadVertices()
{
	ErrStack err_stack;

	checkErrStack(vbuff.load(vertices.data(), vertices.size() * sizeof(GPU_MeshVertex)),
		"failed to load mesh vertices");

	checkErrStack(instabuff.load(instances.data(), instances.size() * sizeof(GPU_MeshInstance)),
		"failed to load mesh instances");

	return err_stack;
}

ErrStack MeshRenderer::loadUniform()
{
	ErrStack err_stack;

	checkErrStack(ubuff.load(&uniform, sizeof(GPU_MeshUniform)),
		"failed to load uniform buffer");

	return err_stack;
}

ErrStack MeshRenderer::draw(nui::SurfaceEvent& event)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	// Generate CPU Data
	{
		generateVertices();
		generateUniform();
	}

	if (dev5 == nullptr) {

		dev5 = event.dev5;
		de_ctx3 = event.de_ctx3;

		viewport_width = event.surface_width;
		viewport_height = event.surface_height;

		// Vertex Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			vbuff.create(dev5, de_ctx3, desc);
		}

		// Instance Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			instabuff.create(dev5, de_ctx3, desc);
		}

		// Uniform Buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			ubuff.create(dev5, de_ctx3, desc);
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
			desc.DepthClipEnable = false;
			desc.ScissorEnable = true;
			desc.MultisampleEnable = false;
			desc.AntialiasedLineEnable = false;

			checkHResult(dev5->CreateRasterizerState(&desc, mesh_rs.GetAddressOf()),
				"failed to create mesh rasterized state");
		}

		// Mesh Blend States
		{
			D3D11_BLEND_DESC desc = {};
			desc.AlphaToCoverageEnable = false;
			desc.IndependentBlendEnable = true;
			std::memset(desc.RenderTarget, 0, 8);

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

		checkErrStack1(loadVertices());
		checkErrStack1(loadUniform());
	}

	// Command List
	de_ctx3->ClearState();

	// Input Assembly
	de_ctx3->IASetInputLayout(mesh_il.Get());
	de_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	{
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
	de_ctx3->VSSetConstantBuffers(0, 1, ubuff.buff.GetAddressOf());
	de_ctx3->VSSetShader(mesh_vs.Get(), nullptr, 0);

	// Rasterization
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = (float)event.pos.x;
		viewport.TopLeftY = (float)event.pos.y;
		viewport.Width = (float)event.surface_width;
		viewport.Height = (float)event.surface_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		de_ctx3->RSSetViewports(1, &viewport);

		D3D11_RECT sccissor;
		sccissor.left = event.pos.x;
		sccissor.top = event.pos.y;
		sccissor.right = event.pos.x + event.surface_width;
		sccissor.bottom = event.pos.y + event.surface_height;

		de_ctx3->RSSetScissorRects(1, &sccissor);

		de_ctx3->RSSetState(mesh_rs.Get());
	}

	// Pixel Shader
	de_ctx3->PSSetShader(mesh_ps.Get(), nullptr, 0);

	// Output Merger
	{
		float blend_factor[4] = { 1, 1, 1, 1 };
		de_ctx3->OMSetBlendState(mesh_bs.Get(), blend_factor, 0xFFFF'FFFF);

		std::array<ID3D11RenderTargetView*, 2> rtvs = {
			event.compose_rtv, event.next_parents_clip_mask_rtv
		};

		de_ctx3->OMSetRenderTargets(rtvs.size(), rtvs.data(), nullptr);
	}

	// Draw
	de_ctx3->DrawInstanced(vertices.size(), 1, 0, 0);

	return err_stack;
}

ErrStack geometryDraw(nui::SurfaceEvent& event)
{
	MeshRenderer* r = (MeshRenderer*)event.user_data;
	return r->draw(event);;
}