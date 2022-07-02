
export function isElementInDescendants(parent: HTMLElement, target: HTMLElement):boolean {
	if (parent === target) {
		return true;
	}

	for (let child of parent.children) {
		if (isElementInDescendants(child as HTMLElement, target)) {
			return true;
		}
	}

	return false;
}