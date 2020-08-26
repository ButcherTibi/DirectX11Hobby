#pragma once

#include "pch.h"


#include "VulkanWrapper.hpp"
#include "GPU_Types.hpp"
#include "FontRasterization.hpp"


namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Root;
	class Flex;
	class Text;
	class Node;


	class Root {
	public:
		Window* window;

		Node* node;

	public:
		Text* addText();
	};


	class Flex {
	public:
		

	public:
		//Text* addText();
	};


	struct CharacterDrawcall {
		Character* chara;	

		std::vector<GPU_CharacterInstance> instances;

		uint32_t instance_start_idx;
	};

	class Text {
	public:
		std::vector<uint32_t> text;
		glm::vec2 pos;
		float size = 14;
		glm::vec4 color = { 1, 1, 1, 1 };

		// Internal
		Window* window;
		Node* node;
		std::vector<CharacterDrawcall> drawcalls;

	public:
		void generateDrawcalls();

	public:
	};


	class Node {
	public:
		std::variant<Root, Flex, Text> elem;

		Node* parent;
		std::list<Node*> children;
	};


	struct WindowCrateInfo {
		uint32_t width = 1024;
		uint32_t height = 720;
	};

	class Window {
	public:
		Instance* instance;

		HINSTANCE hinstance;

		WNDCLASSEXA window_class;
		std::string class_name;

		std::string window_name;
		HWND hwnd = NULL;

		// Properties
		uint32_t width;
		uint32_t height;
		
		// Messages
		bool close;

		// UI
		std::list<Node> nodes;

		// Rendering
		bool rendering_configured = false;

		vkw::VulkanDevice dev;
	
		std::vector<GPU_CharacterVertex> char_verts;
		std::vector<uint32_t> char_idxs;
		std::vector<GPU_CharacterInstance> char_instances;
		GPU_TextUniform text_uniform;

		vkw::Buffer chars_vbuff;
		vkw::Buffer chars_idxbuff;
		vkw::Buffer chars_instabuff;
		vkw::Buffer text_ubuff;

		vkw::Image char_atlas_tex;
		vkw::ImageView char_atlas_view;
		vkw::Sampler sampler;

		vkw::Shader text_vs;
		vkw::Shader text_fs;

		vkw::Drawpass text_pass;

		vkw::CommandList cmd_list;

	public:
		ErrStack generateGPU_CharacterVertexData();

		ErrStack draw();

	public:
		Root* getRoot();

		~Window();
	};

	 
	extern std::vector<Window> windows;


	class Instance {
	public:
		vkw::Instance inst;

		CharacterAtlas char_atlas;

	public:
		ErrStack create();

		ErrStack createWindow(WindowCrateInfo& info, Window*& r_window);

		ErrStack update();

		~Instance();
	};
}
