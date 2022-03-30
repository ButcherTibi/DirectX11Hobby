// Header
#include "Interface.h"

#include "DreamGraphicsAPI/DreamGraphicsAPI.hpp"
#include "ButchersToolbox/Filesys/Filesys.hpp"


uint32_t cppFunction()
{
	dga::Instance instance;
	instance.create();

	dga::Device device;
	instance.getBestDevice(device);

	dga::VertexShader vertex_shader;
	dga::PixelShader pixel_shader;
	{
		filesys::Path exe_path = filesys::Path::executablePath();
		exe_path.pop(8);

		filesys::Path project_path = exe_path;
		project_path.append("BackEnd/Shaders/");

		std::vector<char8_t> vertex_shader_spv;
		std::vector<char8_t> pixel_shader_spv;

		filesys::Path vertex_shader_path = project_path;
		vertex_shader_path.append("Triangle.spv");
		vertex_shader_path.readFile(vertex_shader_spv);

		filesys::Path pixel_shader_path = project_path;
		pixel_shader_path.append("SolidColor.spv");
		pixel_shader_path.readFile(pixel_shader_spv);

		device.createVertexShader(vertex_shader_spv, vertex_shader);
		device.createPixelShader(pixel_shader_spv, pixel_shader);
	}


	return 100;
}
