import MainMenu, { MenuItem } from "./MainMenu/MainMenu.js"


class Globals {
    static main_menu: MainMenu;
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
}

main();