
// Header
#include "Renderer.hpp"

// GLM
#include <glm/gtc/matrix_transform.hpp>


using namespace scme;


ErrStack MeshRenderer::loadVertices()
{
	ErrStack err_stack;

	SculptMesh& mesh = application.mesh;

	uint32_t vertex_count = 0;
	for (VertexBoundingBox& aabb : mesh.aabbs) {
		for (GPU_Vertex& gv : aabb.gpu_vs) {
			vertex_count++;
		}
	}
	vertices.resize(vertex_count);

	uint32_t vertex_idx = 0;
	for (VertexBoundingBox& aabb : mesh.aabbs) {
		for (GPU_Vertex& gv : aabb.gpu_vs) {
			vertices[vertex_idx].pos = dxConvert(gv.pos);
		}
	}

	instances.resize(1);
	instances[0].pos = dxConvert(mesh.pos);
	instances[0].rot = dxConvert(mesh.rot);

	checkErrStack(vbuff.load(vertices.data(), vertices.size() * sizeof(GPU_MeshVertex)),
		"failed to load mesh vertices");

	checkErrStack(instabuff.load(instances.data(), instances.size() * sizeof(GPU_MeshInstance)),
		"failed to load mesh instances");

	return err_stack;
}

ErrStack MeshRenderer::loadUniform()
{
	ErrStack err_stack;

	uniform.camera_quat = { 0, 0, 0, 1 };
	uniform.perspective_matrix = DirectX::XMMatrixPerspectiveFovRH(application.field_of_view,
		(float)render_width / (float)render_height, application.z_near, application.z_far);

	checkErrStack(ubuff.load(&uniform, sizeof(GPU_MeshUniform)),
		"failed to load uniform buffer");

	return err_stack;
}

ErrStack geometryDraw(nui::SurfaceEvent& event)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	MeshRenderer* r = (MeshRenderer*)event.user_data;

	if (r->dev5 == nullptr) {

		r->dev5 = event.dev5;
		r->de_ctx3 = event.de_ctx3;

		r->render_width = event.surface_width;
		r->render_height = event.surface_height;

		// Vertex Buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			r->vbuff.create(r->dev5, r->de_ctx3, desc);
		}
		
		// Instance Buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			r->instabuff.create(r->dev5, r->de_ctx3, desc);
		}

		// Uniform Buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;

			r->ubuff.create(r->dev5, r->de_ctx3, desc);
		}

		// Vertex Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/MeshVS.cso", r->mesh_vs_cso));
			
			checkHResult(r->dev5->CreateVertexShader(r->mesh_vs_cso.data(), r->mesh_vs_cso.size(), nullptr,
				r->mesh_vs.GetAddressOf()),
				"failed to create mesh vertex shader");
		}

		// Pixel Shader
		{
			checkErrStack1(io::readLocalFile("Sculpt/CompiledShaders/MeshPS.cso", r->mesh_ps_cso));

			checkHResult(r->dev5->CreatePixelShader(r->mesh_ps_cso.data(), r->mesh_ps_cso.size(), nullptr,
				r->mesh_ps.GetAddressOf()),
				"failed to create mesh pixel shader");
		}
	}

	return err_stack;
}