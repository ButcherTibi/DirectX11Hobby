
export default class Vector3InputInl {
	instance: HTMLElement | null = null;
	_x: number = 0;
	_y: number = 0;
	_z: number = 0;

	get x() {
		return this._x;
	}

	get y() {
		return this._y;
	}

	get z() {
		return this._z;
	}

	set x(value) {
		this._x = value;

		let x_input: HTMLInputElement = this.instance!.querySelector(".x-axis input")!;
		x_input.value = this._x.toFixed(2);
	}

	set y(value) {
		this._y = value;

		let input: HTMLInputElement = this.instance!.querySelector(".y-axis input")!;
		input.value = this._y.toFixed(2);
	}

	set z(value) {
		this._z = value;

		let input: HTMLInputElement = this.instance!.querySelector(".z-axis input")!;
		input.value = this._z.toFixed(2);
	}

	getValue(idx: number){
		switch(idx) {
			case 0: {
				return this.x;
			}
			case 1: {
				return this.y;
			}
			case 2: {
				return this.z;
			}
			default: {
				throw "invalid index";
			}
		}
	}

	setValue(idx: number, value: number) {
		switch(idx) {
			case 0: {
				this.x = value;
				break;
			}
			case 1: {
				this.y = value;
				break;
			}
			case 2: {
				this.z = value;
				break;
			}
			default: {
				throw "invalid index";
			}
		}
	}

	create = (root_id: string) =>
	{
		let root = document.getElementById(root_id);
		if (root === null) {
			console.trace();
			return;
		}

		let vector3_template = document.getElementById("vec3_input") as HTMLTemplateElement;
		let node = vector3_template.content.cloneNode(true);

		root.append(node);
		this.instance = root.lastElementChild as HTMLElement;

		let sub_btns: HTMLButtonElement[] = [
			this.instance.querySelector(".x-axis .sub")!,
			this.instance.querySelector(".y-axis .sub")!,
			this.instance.querySelector(".z-axis .sub")!
		];
		sub_btns.forEach((sub_btn, idx) => {
			sub_btn.onclick = () => {		
				this.setValue(idx, this.getValue(idx) - 0.1);
			};
		});

		let add_btns: HTMLButtonElement[] = [
			this.instance.querySelector(".x-axis .add")!,
			this.instance.querySelector(".y-axis .add")!,
			this.instance.querySelector(".z-axis .add")!,
		];
		add_btns.forEach((add_btn, idx) => {
			add_btn.onclick = () => {		
				this.setValue(idx, this.getValue(idx) + 0.1);
			};
		});

		let field_x = this.instance.querySelector(".x-axis .field") as HTMLSpanElement;
		let input_x = field_x.querySelector("input");

	}
}