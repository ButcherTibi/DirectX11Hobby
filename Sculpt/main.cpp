
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

	auto w0 = root->addWrap();
	w0->width = 50.0f;
	w0->height = 50.0f;
	w0->background_color = nui::Color::red();
	w0->overflow = nui::Overflow::CLIP;

	auto w1 = w0->addWrap();
	w1->width = 50.0f;
	w1->height = 100.0f;
	w1->background_color = nui::Color::green();

	auto w2 = w1->addWrap();
	w2->width = 50.0f;
	w2->height = 100.0f;
	w2->background_color = nui::Color::blue();

	auto t = w2->addText();
	t->text = U"This text should overflow past blue into green and red but clipped by red";

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
