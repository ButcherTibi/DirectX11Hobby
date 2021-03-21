
// Header
#include "RenderDocIntegration.hpp"


RenderDocIntegration render_doc;


void RenderDocIntegration::init()
{
	this->api = nullptr;

	HMODULE module = GetModuleHandleA("renderdoc.dll");
	if (module == nullptr) {
		// printf("renderdoc.dll could not be located");
		return;
	}

	pRENDERDOC_GetAPI getApi = (pRENDERDOC_GetAPI)GetProcAddress(module, "RENDERDOC_GetAPI");
	if (getApi == nullptr) {
		printf("Could not find GetAPI procedure in the RenderDoc DLL \n");
		return;
	}

	if (!getApi(eRENDERDOC_API_Version_1_4_1, (void**)&api)) {
		printf("Could not get RenderDoc API \n");
		return;
	}

	int32_t major;
	int32_t minor;
	int32_t patch;
	api->GetAPIVersion(&major, &minor, &patch);
	printf("RenderDoc Integration Active API Version %d.%d.%d \n", major, minor, patch);
}

void RenderDocIntegration::startCapture()
{
	if (api == nullptr) {
		return;
	}

	api->StartFrameCapture(nullptr, nullptr);

	printf("RenderDoc frame capture started \n");
}

void RenderDocIntegration::endCapture()
{
	if (api == nullptr) {
		return;
	}

	api->EndFrameCapture(nullptr, nullptr);

	printf("RenderDoc frame capture ended \n");

}

void RenderDocIntegration::triggerCapture()
{
	if (api == nullptr) {
		return;
	}

	api->TriggerCapture();

	printf("RenderDoc trigger capture \n");
}

bool RenderDocIntegration::isFrameCapturing()
{
	if (api == nullptr) {
		return false;
	}

	uint32_t is_frame_capturing = api->IsFrameCapturing();
	
	printf("RenderDoc is frame capturing = %d \n", is_frame_capturing);
	
	return is_frame_capturing;
}
