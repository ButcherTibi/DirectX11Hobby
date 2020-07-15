
#include "FileIO.h"

// header
#include "DX11ObjectTypes.h"


using namespace nui_int;


ErrStack VertexShader::create(ID3D11Device* device, std::string path)
{
	ErrStack err_stack{};
	HRESULT hr{};

	FilePath file_path = {};
	checkErrStack1(file_path.recreateRelativeToSolution(path));

	checkErrStack(file_path.read(code), "failed to read vertex shader code");

	checkHResult(device->CreateVertexShader(code.data(), code.size(), NULL, this->shader.GetAddressOf()),
		"failed to create vertex shader");

	return err_stack;
}

ErrStack PixelShader::create(ID3D11Device* device, std::string path)
{
	ErrStack err_stack{};
	HRESULT hr{};

	FilePath file_path{};
	checkErrStack1(file_path.recreateRelativeToSolution(path));

	checkErrStack(file_path.read(code), "failed to read pixel shader code");

	checkHResult(device->CreatePixelShader(code.data(), code.size(), NULL, this->shader.GetAddressOf()),
		"failed to create pixel shader");

	return err_stack;
}
