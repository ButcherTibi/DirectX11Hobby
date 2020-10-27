
#include "NuiLibrary.hpp"
#include "Renderer.hpp"


void onBlueEnter(nui::MouseEnterEvent& event)
{
	printf("Blue entered \n");

	// nui::Wrap* wrap = std::get_if<nui::Wrap>(&event.source->elem);
}

void onBlueLeave(nui::MouseLeaveEvent& event)
{
	printf("Blue leave \n");

	// nui::Wrap* wrap = std::get_if<nui::Wrap>(&event.source->elem);
}

void onBlueDeltaBegin(nui::MouseDeltaEvent& event)
{
	printf("Blue delta begin \n");
}

void onBlueDelta(nui::MouseDeltaEvent& event)
{
	printf("Blue delta for %d %d \n", event.delta_x, event.delta_y);
}

void onBlueDeltaEnd(nui::MouseDeltaEvent& event)
{
	printf("Blue delta end \n");
}

void onBlueKeyDown(nui::KeyDownEvent& event)
{
	printf("Blue key down \n");

	nui::Wrap* wrap = std::get_if<nui::Wrap>(&event.source->elem);
	wrap->beginMouseDelta();
}

void onBlueHeldKeyDown(nui::KeyHeldDownEvent& event)
{
	printf("Blue key held down for %f \n", event.duration);

	nui::Wrap* wrap = std::get_if<nui::Wrap>(&event.source->elem);
	// wrap->endMouseDelta();
}

void onBlueKeyUp(nui::KeyUpEvent& event)
{
	printf("Blue key up \n");

	nui::Wrap* wrap = std::get_if<nui::Wrap>(&event.source->elem);
	wrap->endMouseDelta();
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

void onCameraOrbitBtn(nui::KeyHeldDownEvent& event)
{
	printf("key held down for %f \n", event.duration);

	nui::Surface* surf = std::get_if<nui::Surface>(&event.source->elem);
	surf->NodeComp::window->setLocalMousePosition(0, 0);
}

int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
	ErrStack err_stack{};

	// Application
	{
		application.mesh.createAsTriangle(glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 0, 0 }, 1);

		application.field_of_view = 90;
		application.z_near = 0.1f;
		application.z_far = 100;
		application.camera_pos = { 0, 0, 10 };
		application.camera_quat = { 0, 0, 0, 1 };
	}

	nui::Instance instance;
	err_stack = instance.create();
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	nui::WindowCreateInfo info;
	info.width = 1027;
	info.height = 720;

	nui::Window* window;
	err_stack = instance.createWindow(info, window);
	if (err_stack.isBad()) {
		err_stack.debugPrint();
		return 1;
	}

	// UI code
	MeshRenderer renderer;
	nui::Surface* surface = window->addSurface();
	surface->gpu_callback = geometryDraw;
	surface->user_data = &renderer;
	surface->addKeyHeldDownEvent(onCameraOrbitBtn, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

	auto w0 = surface->addWrap();
	w0->pos.x = 50;
	w0->width = 50.0f;
	w0->height = 50.0f;
	w0->background_color = nui::Color::red();
	w0->overflow = nui::Overflow::CLIP;

	auto green = w0->addWrap();
	green->width = 50.0f;
	green->height = 100.0f;
	green->background_color = nui::Color::green();

	auto blue = green->addWrap();
	blue->width = 50.0f;
	blue->height = 100.0f;
	blue->background_color = nui::Color::blue();
	blue->setMouseEnterEvent(onBlueEnter);
	blue->setMouseLeaveEvent(onBlueLeave);

	blue->setMouseDeltaBeginEvent(onBlueDeltaBegin);
	blue->setMouseDeltaEvent(onBlueDelta);
	blue->setMouseDeltaEndEvent(onBlueDeltaEnd);

	blue->addKeyDownEvent(onBlueKeyDown, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);
	blue->addKeyHeldDownEvent(onBlueHeldKeyDown, nui::VirtualKeys::S);
	blue->addKeyUpEvent(onBlueKeyUp, nui::VirtualKeys::RIGHT_MOUSE_BUTTON);

	auto t = blue->addText();
	t->text = U"This text should overflow past blue into green and red but clipped by red";
	t->pos.x = 100;
	t->setMouseEnterEvent(onTextEnter);
	t->setMouseLeaveEvent(onTextLeave);

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
