import MainMenu, { MenuItem } from "./MainMenu/MainMenu.js"
import Vector3InputInl from "./Editors/Vector3InputInl.js";
import Accordion, { AccordionItem } from "./Accordion/Accordion.js";


class Globals {
    static main_menu: MainMenu;
	static vec3: Vector3InputInl;
	static accordion: Accordion;
}


function main() {
    Globals.main_menu = new MainMenu();

	let items: MenuItem[] = [
		{
			name: "Scene",
			children: [
				{
					name: "New",
					children: [
						{ name: "Empty Scene" },
						{ name: "Triangle Example" },
						{ name: "Quad Example" },
						{ name: "Cube Example" },
						{ name: "Cylinder Example" },
						{ name: "UV Sphere Example" },
						{ name: "ISO Sphere Example" },
						{ name: "Character Example" }
					]
				},
				{
					name: "Open"
				},
				{
					name: "Open Recent",
					children: []
				},
				{
					name: "Save",
					children: []
				},
				{
					name: "Save As",
					children: []
				},
				{
					name: "Import",
					children: [
						{ name: "Autodesk's FBX" },
						{ name: "glTF 2.0" },
						{ name: "OBJ" }
					]
				},
				{
					name: "Export",
					children: [
						{ name: "Autodesk's FBX" },
						{ name: "glTF 2.0" },
						{ name: "OBJ" }
					]
				},
				{
					name: "Quit",
					children: []
				}
			]
		},
		{
			name: "Mesh",
			children: [
				{ name: "Copy" },
				{ name: "Edit" },
				{ name: "Delete" }
			]
		},
		{
			name: "Instance",
			children: [
				{
					name: "Create",
					children: [
						{ name: "Triangle" },
						{ name: "Quad" },
						{ name: "Cube" },
						{ name: "Cylinder" },
						{ name: "UV Sphere" },
						{ name: "ISO Sphere" }
					]
				},
				{ name: "Copy" },
				{ name: "Edit" },
				{ name: "Delete" }
			]
		},
		{
			name: "View",
			children: [
				{ name: "Default" },
				{ name: "Transparent" },
				{ name: "Wireframe" }
			]
		},
		{
			name: "Layers"
		},
		{
			name: "Learn",
			children: [
				{ name: "Progresive Tutorial" },
				{ name: "Features" }
			]
		}
	];
	Globals.main_menu.render("main_menu_root", items);

	// Vector3 Input
	Globals.vec3 = new Vector3InputInl();
	Globals.vec3.create("panel");
	Globals.vec3.x = 123.67;
	Globals.vec3.y = 0.981;
	Globals.vec3.z = 10.45;
	Globals.vec3.onchange = (x, y, z) => {
		console.log(`vector3 = {${x}, ${y}, ${z}}`);
	};
	
	Globals.vec3 = new Vector3InputInl();
	Globals.vec3.create("panel");

	// Accordion
	{
		let p = document.createElement("p");
		p.textContent = "Test content";

		let p2 = document.createElement("p");
		p2.textContent = "Another bit of content";

		let frag = document.createDocumentFragment();
		frag.append(p, p2);

		let accordion_items: AccordionItem[] = [
			{ name: "Item 1", content: frag },
			{ name: "Item 2" },
			{ name: "Item 3", content: frag }
		];
		Globals.accordion = new Accordion();
		Globals.accordion.create("panel", accordion_items);
	}
}

main();