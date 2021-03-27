#pragma once

// intended for vector of pointers only
template<typename T>
void emplaceBackUnique(std::vector<T>& values, T new_value)
{
	for (T& value : values) {
		if (value == new_value) {
			return;
		}
	}

	values.push_back(new_value);
}
