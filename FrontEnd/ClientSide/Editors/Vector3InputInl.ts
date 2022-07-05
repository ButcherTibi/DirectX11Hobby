
export default class Vector3InputInl {
	private instance: HTMLElement | null = null;
	private _x: number = 0;
	private _y: number = 0;
	private _z: number = 0;
	onchange?: (x: number, y: number, z: number) => void;

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

		if (this.onchange !== undefined) {
			this.onchange(this._x, this._y, this._z);
		}
	}

	set y(value) {
		this._y = value;

		let input: HTMLInputElement = this.instance!.querySelector(".y-axis input")!;
		input.value = this._y.toFixed(2);

		if (this.onchange !== undefined) {
			this.onchange(this._x, this._y, this._z);
		}
	}

	set z(value) {
		this._z = value;

		let input: HTMLInputElement = this.instance!.querySelector(".z-axis input")!;
		input.value = this._z.toFixed(2);

		if (this.onchange !== undefined) {
			this.onchange(this._x, this._y, this._z);
		}
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
			throw `root_id ${root_id} not found`;
		}

		let vector3_template = document.getElementById("vec3_input_inl") as HTMLTemplateElement;
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

		let inputs: HTMLInputElement[] = [
			this.instance.querySelector(".x-axis input") as HTMLInputElement,
			this.instance.querySelector(".y-axis input") as HTMLInputElement,
			this.instance.querySelector(".z-axis input") as HTMLInputElement
		];
		inputs.forEach((input, idx) => {
			input.onchange = (e) => {
				let input_elem = e.target as HTMLInputElement;
				let new_value = parseFloat(input_elem.value);

				if (isNaN(new_value) === false) {
					this.setValue(idx, new_value);
				}
				else {
					this.setValue(idx, this.getValue(idx));
				}
			};
		});
	}
}