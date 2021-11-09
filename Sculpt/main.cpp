
#include "RenderDocIntegration.hpp"
import UserInterface;
#include "Application.hpp"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Memory Debugging
#if 0
	{
		int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value.
		_CrtSetDbgFlag(tmpFlag);
	}
#endif

	// GPU Debugging using Render Doc
#if _DEBUG
	render_doc.init();
#endif

	// Init User Interface Library
	{
		nui::Instance& instance = application.ui_instance;
		instance.create();

		// Create Window
		nui::Window* window;
		{
			nui::Window::CreateInfo info;
			info.width = 1027;
			info.height = 720;

			application.main_window = instance.createWindow(info);
			window = application.main_window;
		}
	}

	while (application.main_window->win_messages.should_close == false) {

		application.main_window->update([](nui::Window* win, void*) {

			nui::Flex* flex;
			{
				nui::FlexCreateInfo info;
				info.id = "flex_id";
				info.size[0] = 100.f;
				info.size[1] = 100.f;

				flex = win->createFlex(info);
			}
			
			{
				nui::DirectX11_Viewport::CreateInfo info;
				info.id = "viewport_id";
				info.size[0] = 100.f;
				info.size[1] = 100.f;
				info.callback = geometryDraw;

				flex->createDirectX11_Viewport(info);
			}
		});
	}

	return 0;
}

int main(int, char**)
{
	// Disabled compiler warnings:
	// - 4201 nameless struct/union = needed by GLM
	// - 4239 default reference parameter
	// - 4267 uint32_t size = vector.size()
	// - 4701 potentially uninitialized local variable used
	// - 4703 potentially uninitialized local pointer variable used

	return WinMain(nullptr, nullptr, nullptr, 0);
}
