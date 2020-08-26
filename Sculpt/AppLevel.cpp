
#include "NuiLibrary.h"

// Header
#include "AppLevel.h"


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
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
	nui::Text* text = root->addText();

	for (char c = '!'; c <= '~'; c++) {
		text->text.push_back(c);
	}
	text->pos = {0, 500};
	text->size = 20;

	while (!window->close) {

		err_stack = instance.update();
		if (err_stack.isBad()) {
			err_stack.debugPrint();
			return 1;
		}
	}

	return 0;
}
