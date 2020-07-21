#pragma once

// DX11
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"DXGI.lib")

// Mine
#include "DX11ObjectTypes.h"


namespace nui_int {

	struct DrawInfo {
		uint32_t count;
		uint32_t start;
	};

	struct ElementRendering {
		DrawInfo padding_rect;
		DrawInfo padding_tl_circle;
	};

	class DX11Renderer {
	public:
		uint32_t width;
		uint32_t height;

		std::vector<GPU_Vertex> vertices;
		VectorBuffer<GPU_Vertex> vertex_buff;

		std::vector<GPU_RectProps> rect_props;
		VectorBuffer<GPU_RectProps> rect_props_buff;
		ComPtr<ID3D11ShaderResourceView> rect_props_view = nullptr;

		std::vector<GPU_CircleProps> circle_props;
		VectorBuffer<GPU_CircleProps> circle_props_buff;
		ComPtr<ID3D11ShaderResourceView> circle_props_view = nullptr;

		GPU_CommonStuff common_stuff;
		FixedBuffer<GPU_CommonStuff> common_buff;
		ComPtr<ID3D11ShaderResourceView> common_buff_view;

		std::vector<ElementRendering> elements;

		// DX11
		ComPtr<ID3D11Device> device;
		ComPtr<IDXGISwapChain> swapchain;
		ComPtr<ID3D11DeviceContext> imediate_ctx;
		ComPtr<ID3D11Debug> dx11_debug;
	
		ComPtr<ID3D11DeviceContext> deferred_ctx;
		ComPtr<ID3D11CommandList> command_list;

		ComPtr<ID3D11RenderTargetView> swapchain_view;

		VertexShader rect_VS;
		ComPtr<ID3D11InputLayout> vertex_input;

		ComPtr<ID3D11RasterizerState> rasterizer_state;

		PixelShader rect_PS;
		PixelShader circle_PS;

	public:
		
	};
}
