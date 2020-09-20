
#include "NuiLibrary.hpp"


int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	nui::ErrStack err_stack{};

	nui::Instance instance;
	err_stack = instance.create();
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::WindowCrateInfo info;
	info.width = 1027;
	info.height = 720;
	nui::Window* window;
	err_stack = instance.createWindow(info, window);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::Wrap* root = window->getRoot();

	nui::Wrap* w0 = root->addWrap();
	w0->width.setRelative(0.5);
	w0->height.setRelative(0.5);
	w0->background_color = { 1, 0, 0, 1 };
	w0->overflow = nui::Overflow::CLIP;

	nui::Text* t0 = w0->addText();
	t0->pos.x = 455;
	t0->text = U"DirectX 11";

	/*nui::Wrap* w1 = w0->addWrap();
	w1->pos.x = 400;
	w1->width.setRelative(0.5);
	w1->height.setRelative(0.5);
	w1->background_color = { 0, 1, 0, 1 };*/

	while (!window->close) {

		err_stack = instance.update();
		if (err_stack.isBad()) {
			err_stack.debugPrint();
			return 1;
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	return WinMain(GetModuleHandleA(NULL), NULL, "", 1);
}
