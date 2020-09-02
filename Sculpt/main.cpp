
#include "NuiLibrary.h"


int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	nui::ErrStack err_stack{};

	nui::Instance instance;
	err_stack = instance.create();
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::WindowCrateInfo info = {};
	nui::Window* window;
	err_stack = instance.createWindow(info, window);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::Root* root = window->getRoot();

	nui::Wrap* wrap = root->addWrap();
	wrap->pos = { 0, 0 };
	wrap->width = 100;
	wrap->height = 100;
	// wrap->overflow = nui::Overflow::CLIP;
	wrap->background_color = { 1, 0, 0, 1 };

	nui::Text* text = wrap->addText();
	text->text = U"this text will be clipped by the parent if too large";
	text->pos = { 0, 20 };

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
