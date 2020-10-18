
#include "NuiLibrary.hpp"
#include "Renderer.hpp"


void onRedKeyDown(nui::KeyDownEvent& event)
{
	printf("Red key down \n");
}

void onBlueEnter(nui::MouseEnterEvent& event)
{
	printf("Blue entered \n");
}

void onBlueHover(nui::MouseHoverEvent& event)
{
	printf("Blue hover called for %f \n", event.duration);
}

void onBlueLeave(nui::MouseLeaveEvent& event)
{
	printf("Blue leave \n");
}

void onBlueMove(nui::MouseMoveEvent& event)
{
	printf("Blue move \n");
}

void onTextEnter(nui::MouseEnterEvent& event)
{
	printf("text entered \n");
}

void onTextHover(nui::MouseHoverEvent& event)
{
	printf("text hover called for %f \n", event.duration);
}

void onTextLeave(nui::MouseLeaveEvent& event)
{
	printf("text leave \n");
}

int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	ErrStack err_stack{};

	// Application
	{
		application.field_of_view = 25;
		application.z_near = 0.1f;
		application.z_far = 100;
		application.mesh.createAsTriangle(glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 0, 0 });
	}

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

	// UI code
	auto w0 = window->addWrap();
	w0->pos.x = 50;
	w0->width = 50.0f;
	w0->height = 50.0f;
	w0->background_color = nui::Color::red();
	w0->overflow = nui::Overflow::CLIP;
	w0->addKeyDownEvent(onRedKeyDown, nui::VirtualKey::A);

	auto green = w0->addWrap();
	green->width = 50.0f;
	green->height = 100.0f;
	green->background_color = nui::Color::green();

	auto blue = green->addWrap();
	blue->width = 50.0f;
	blue->height = 100.0f;
	blue->background_color = nui::Color::blue();
	blue->setOnMouseEnterEvent(onBlueEnter);
	blue->setOnMouseLeaveEvent(onBlueLeave);
	blue->setOnMouseMoveEvent(onBlueMove);

	auto t = blue->addText();
	t->text = U"This text should overflow past blue into green and red but clipped by red";
	t->pos.x = 100;
	t->setOnMouseEnterEvent(onTextEnter);
	t->setOnMouseLeaveEvent(onTextLeave);

	MeshRenderer renderer;
	nui::Surface* surface = window->addSurface();
	surface->callback = geometryDraw;
	surface->user_data = &renderer;

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
