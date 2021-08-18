
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
	instance->_loadCharacterAtlasToTexture();

	// Constant buffer data
	{
		cbuff.setUint(GPU_ConstantsFields::SCREEN_WIDTH, surface_width);
		cbuff.setUint(GPU_ConstantsFields::SCREEN_HEIGHT, surface_height);
	}

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

	// Viewport
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)surface_width;
		viewport.Height = (float)surface_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}

	// Draw
	im_ctx3->ClearState();

	for (auto& stack : draw_stacks) {
		for (Element* elem : stack.second) {

			elem->_draw();
		}
	}

#if _DEBUG
	throwDX11(swapchain1->Present(0, 0));
#else
	// go fuck yourself DXGI, just ignore my presentation attempt if you can't handle it
	// NOTE: never rely on presentation being correct or incorrect as the context of presenting and it's
	// meaning can vary depending on Windows's behaviour
	// Example:
	// if you minimize the app via buttons or context menu it works
	// if press the taskbar To Desktop button it crashes, apparently that was not a minimization
	swapchain1->Present(0, 0);
#endif
}
