#include <App/Application.hpp>


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// Configure Console
	SetConsoleOutputCP(CP_UTF8);

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

	app.init(true);

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
