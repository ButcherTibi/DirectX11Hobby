#pragma once

namespace nui {

	// Forward
	class Window;
	class Node;
	class Text;
	class Wrap;
	class Surface;


	class NodeComp {
	public:
		Window* window;
		Node* node;

	public:
		void _create(Window* window, Node* node);

		Text* addText();
		Wrap* addWrap();
		Surface* addSurface();
	};

}
