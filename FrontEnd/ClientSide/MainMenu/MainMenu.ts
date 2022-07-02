
import { isElementInDescendants } from "../Utils.js"


export class MenuItem {
	name: string = "";
	children?: MenuItem[] = [];
}

export default class MainMenu {
	private container_id: string = "";
	private items: MenuItem[] = [];
    private close_timer_id: number = Number.NaN;
	private outside_click_handler: any = null;

	// private static title_item_class = "title-item";
	// private static normal_item_class = "normal-item";


	private _hideDescedants = (menu: HTMLMenuElement, except: HTMLMenuElement | null) => {
		if (menu != except) {

			if (menu.classList.contains("hide") === false) {
				menu.classList.add("hide");
			}
			
			for (let item of menu.children) {
	
				let child_menu: HTMLMenuElement = item.children[1] as HTMLMenuElement;
				if (child_menu !== undefined) {
					this._hideDescedants(child_menu, except);
				}
			}
		}
	}

	private hideDescedants = (menu: HTMLMenuElement, except: HTMLMenuElement | null) => {
		for (let item of menu.children) {
	
			let child_menu: HTMLMenuElement = item.children[1] as HTMLMenuElement;
			if (child_menu !== undefined) {
				this._hideDescedants(child_menu, except);
			}
		}
	}
	
	showMenu(e: MouseEvent) {
		let li_elem: HTMLElement = e.target as HTMLElement;

		let menu: HTMLMenuElement = li_elem.children[1] as HTMLMenuElement;
		let parent_menu: HTMLMenuElement = li_elem.parentElement as HTMLMenuElement;
		clearTimeout(this.close_timer_id);

		if (menu !== undefined) {
			this.hideDescedants(parent_menu, menu);
			menu.classList.remove("hide");

			if (parent_menu.classList.contains("main-menu") === false) {
				let menu_width: number = parent_menu.getBoundingClientRect().width;
				menu.style.top = `-2px`;
				menu.style.left = `${menu_width - 4}px`;
			}
		}
		else {
			this.hideDescedants(parent_menu, null);
		}
	}

	scheduleHidingMenu(e: MouseEvent) {
		let li_elem: HTMLElement = e.target as HTMLElement;

		let menu: HTMLMenuElement = li_elem.children[1] as HTMLMenuElement;
		if (menu !== undefined) {

			let hideMenu = (menu: HTMLMenuElement) => {
				if (menu.classList.contains("hide") === false) {
					menu.classList.add("hide");
				}
			};

			// Reschedule hiding the menu
			clearTimeout(this.close_timer_id);
			this.close_timer_id = setTimeout(hideMenu, 1000, menu);
		}
	}

	private _render = (parent_elem: HTMLElement, item: MenuItem, level: number) => {
		let li_elem = document.createElement("li");
		li_elem.onmouseenter = (e) => this.showMenu(e);
		li_elem.onmouseleave = (e) => this.scheduleHidingMenu(e);

		if (level === 0) {
			li_elem.classList.add("title-item");
		}
		else {
			li_elem.classList.add("normal-item");
		}

		li_elem.classList.add("menu-item");

		let label = document.createElement("label");
		label.textContent = item.name;

		li_elem.appendChild(label);

		if (item.children && item.children.length > 0) {

			let menu_elem = document.createElement("menu");
			menu_elem.classList.add("hide");

			for (let child of item.children) {
				this._render(menu_elem, child, level + 1);
			}

			li_elem.appendChild(menu_elem);
		}

		parent_elem.appendChild(li_elem);
	}

	private closeMenuIfClickedOutside = (e: MouseEvent) => {
		let container_elem = document.getElementById(this.container_id) as HTMLElement;
		let menu = container_elem.children[0] as HTMLMenuElement;
		
		if (isElementInDescendants(menu, e.target as HTMLElement) === false) {
			this.hideDescedants(menu, null);
		}
	}

	render = (new_container_id: string, new_items: MenuItem[]) => {
		this.container_id = new_container_id;
		this.items = new_items;

		let menu_elem = document.createElement("menu");
		menu_elem.classList.add("main-menu");

		for (let item of this.items) {
			this._render(menu_elem, item, 0);
		}

		let root = document.getElementById(new_container_id) as HTMLElement;
		root.appendChild(menu_elem);

		document.removeEventListener("click", this.closeMenuIfClickedOutside);
		document.addEventListener("click", this.closeMenuIfClickedOutside);
	}
}
