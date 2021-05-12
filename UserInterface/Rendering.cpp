
// Header
#include "NuiLibrary.hpp"


using namespace nui;


void Window::_render()
{
	// don't render if display too small or window is not visible
	if (win_messages.is_minimized ||
		surface_width < 10 || surface_height < 10)
	{
		return;
	}

	auto& im_ctx3 = instance->im_ctx3;
	auto& dev5 = instance->dev5;

	// Load Character Atlas
	instance->loadCharacterAtlasToTexture();

	// Load Constant Buffer
	cbuff.load(&gpu_constants, sizeof(GPU_Constants));

	// Resize Attachments
	{
		DXGI_SWAP_CHAIN_DESC desc;
		swapchain1->GetDesc(&desc);

		if (desc.BufferDesc.Width != surface_width || desc.BufferDesc.Height != surface_height) {

			present_rtv->Release();
			present_img->Release();

			throwDX11(swapchain1->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc.Flags));

			throwDX11(swapchain1->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())));

			throwDX11(dev5->CreateRenderTargetView(present_img.Get(), NULL, present_rtv.GetAddressOf()));
		}
	}

	// Draw
	im_ctx3->ClearState();

	std::array<float, 4> clear_color = { 0, 0, 0, 0 };
	im_ctx3->ClearRenderTargetView(present_rtv.Get(), clear_color.data());

	for (auto& stack : draw_stacks) {
		for (Element* elem : stack.second) {

			elem->_draw();
		}
	}

	throwDX11(swapchain1->Present(0, 0));
}
