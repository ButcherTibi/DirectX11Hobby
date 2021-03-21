#pragma once

#include "renderdoc_app.h"


class RenderDocIntegration {
public:
	RENDERDOC_API_1_4_1* api;

public:
	void init();

	void startCapture();
	void endCapture();
	void triggerCapture();

	bool isFrameCapturing();
};

extern RenderDocIntegration render_doc;
