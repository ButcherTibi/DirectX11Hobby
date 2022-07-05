
export class AccordionItem {
	name: string = "";
	content?: DocumentFragment;
}

export default class Accordion {
	create = (root_id: string, items: AccordionItem[]) => {
		let root = document.getElementById(root_id);
		if (root === null) {
			console.trace();
			return;
		}

		let ul = document.createElement("ul");
		ul.classList.add("accordion");

		items.forEach(item => {
			let label = document.createElement("label");
			label.textContent = item.name;  
			
			let summary = document.createElement("summary");
			summary.appendChild(label);

			let li = document.createElement("li");
			li.appendChild(summary);

			if (item.content !== undefined) {

				let arrow = document.createElement("i");
				arrow.textContent = "⯈";
				summary.insertBefore(arrow, label);

				summary.onclick = (e) => {
					let li = e.currentTarget as HTMLElement;
					let content =  li.parentElement!.children[1] as HTMLDivElement;

					if (content.style.height === "0px" || content.style.height === "") {
						content.style.height = content.scrollHeight + "px";
					}
					else {
						content.style.height = "0px";
					}
				};

				let content_wrap = document.createElement("div");
				content_wrap.classList.add("content-wrap");

				let content = document.createElement("div");
				content.classList.add("content");
				content.appendChild(item.content.cloneNode(true));

				content_wrap.appendChild(content);

				li.appendChild(content_wrap);
			}

			ul.appendChild(li);
		});

		root.appendChild(ul);
	}
}