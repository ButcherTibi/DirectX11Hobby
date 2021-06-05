#pragma once


// Forward declarations
template<typename T>
class SparseVector;

// I did something wrong but what ?
//template<typename T>
//class LiteVector;
//
//
//template<typename T>
//struct LiteVectorIterator {
//	LiteVector<T>* parent;
//	uint32_t current_idx;
//
//	LiteVectorIterator() {};
//
//	bool operator!=(LiteVectorIterator& iter_right)
//	{
//		return parent != iter_right.parent || current_idx != iter_right.current_idx;
//	}
//
//	T& get()
//	{
//		return *(parent->data + current_idx);
//	}
//
//	inline uint32_t index()
//	{
//		return current_idx;
//	}
//
//	void next()
//	{
//		current_idx++;
//	}
//};
//
//
//template<typename T>
//class LiteVector {
//public:
//	uint32_t size;
//	uint32_t capacity;
//	T* data;
//
//	LiteVector()
//	{
//		size = 0;
//		capacity = 0;
//	}
//
//	void resize(uint32_t new_size)
//	{
//		if (capacity == 0) {
//			data = new T[new_size];
//			capacity = new_size;
//		}
//		else if (capacity < new_size) {
//
//			T* new_data = new T[new_size];
//
//			std::memcpy(new_data, data, capacity * sizeof(T));
//
//			delete[] data;
//
//			data = new_data;
//			capacity = new_size;
//		}
//
//		size = new_size;
//	}
//
//	void reserve(uint32_t new_capacity)
//	{
//		if (capacity == 0) {
//			data = new T[new_capacity];
//			capacity = new_capacity;
//		}
//		else if (new_capacity > capacity) {
//
//			T* new_data = new T[new_capacity];
//
//			std::memcpy(new_data, data, new_capacity * sizeof(T));
//
//			delete[] data;
//
//			data = new_data;
//			capacity = new_capacity;
//
//			// size unchanged
//		}
//	}
//
//	void clear()
//	{
//		size = 0;
//	}
//
//	T& operator[](uint32_t idx)
//	{
//		assert_cond(idx <= capacity);
//		return data[idx];
//	}
//
//	T& emplace_back()
//	{
//		this->resize(size + 1);
//		return data[size - 1];
//	}
//
//	LiteVectorIterator<T> begin()
//	{
//		LiteVectorIterator<T> iter;
//		iter.parent= this;
//		iter.current_idx = 0;
//
//		return iter;
//	}
//
//	LiteVectorIterator<T> end()
//	{
//		LiteVectorIterator<T> iter;
//		iter.parent = this;
//		iter.current_idx = size;
//
//		return iter;
//	}
//
//	~LiteVector()
//	{
//		if (capacity) {
//			delete[] data;
//		}
//	}
//};


template<typename T>
struct DeferredVectorNode {
	bool is_deleted;
	T elem;
};


template<typename T>
class DeferredVectorIterator {
public:
	SparseVector<T>* _parent_vector;
	uint32_t _current_idx;

public:
	DeferredVectorIterator() {};

	bool operator!=(DeferredVectorIterator& iter_right)
	{
		return _parent_vector != iter_right._parent_vector || _current_idx != iter_right._current_idx;
	}

	T& get()
	{
		return _parent_vector->nodes[_current_idx].elem;
	}

	inline uint32_t index()
	{
		return _current_idx;
	}

	void next()
	{
		auto& nodes = _parent_vector->nodes;

		_current_idx++;

		while (true) {
			if (_current_idx > _parent_vector->_last_index) {
				return;  // reached the end
			}

			if (nodes[_current_idx].is_deleted == false) {
				return;  // element found
			}

			_current_idx++;
		}
	}

	// WARNING: not fully reversible
	void prev()
	{
		auto& nodes = _parent_vector->nodes;

		_current_idx--;

		while (true) {
			if (_current_idx > _parent_vector->_first_index) {
				return;  // reached the begin
			}

			if (nodes[_current_idx].is_deleted == false) {
				return;  // element found
			}

			_current_idx++;
		}
	}
};


// made specifically for holding primitive data
// deletions are cheap as they don't have to reallocate the whole vector
// preserves indexes
template<typename T>
class SparseVector {
public:
	uint32_t _size;  // current used size without deleted elements

	// first element in vector may be deleted so keep track so you don't have to skip on each begin
	uint32_t _first_index;
	uint32_t _last_index;

	std::vector<DeferredVectorNode<T>> nodes;
	std::vector<uint32_t> deleted;

public:
	SparseVector()
	{
		_size = 0;
		_first_index = 0;
		_last_index = 0;

		//elems();
		//deleted();
	}

	void resize(uint32_t new_size)
	{
		nodes.resize(new_size);

		_size = new_size;
		_last_index = new_size - 1;
	}

	void reserve(uint32_t new_capacity)
	{
		nodes.reserve(new_capacity);
	}

	T& emplace(uint32_t& r_index)
	{
		// try reuse deleted
		for (uint32_t& deleted_idx : deleted) {

			if (deleted_idx != 0xFFFF'FFFF) {

				// mark node as available
				DeferredVectorNode<T>& recycled_node = nodes[deleted_idx];
				recycled_node.is_deleted = false;

				// bounds update
				if (deleted_idx < _first_index) {
					_first_index = deleted_idx;
				}
				else if (deleted_idx > _last_index) {
					_last_index = deleted_idx;
				}

				// remove from deleted list
				deleted_idx = 0xFFFF'FFFF;

				_size++;

				return recycled_node.elem;
			}
		}

		// Create new node
		_last_index = _size;
		_size++;

		r_index = nodes.size();

		DeferredVectorNode<T>& new_node = nodes.emplace_back();
		return new_node.elem;
	}

	void erase(uint32_t index)
	{
		DeferredVectorNode<T>& node = nodes[index];
		if (node.is_deleted == false) {

			node.is_deleted = true;

			if (nodes.size() > 1) {

				// seek forward new first index
				if (index == _first_index) {

					uint32_t i = index + 1;

					while (true) {
						if (i == _last_index || nodes[i].is_deleted == false) {
							_first_index = i;
							break;
						}
						i++;
					}
				}
				// seek backward new last index
				else if (index == _last_index && index > 1) {

					uint32_t i = index - 1;

					while (true) {
						if (i == _first_index || nodes[i].is_deleted == false) {
							_last_index = i;
							break;
						}
						i--;
					}
				}
			}	

			_size--;

			// add to delete list
			for (uint32_t& deleted_idx : deleted) {
				if (deleted_idx == 0xFFFF'FFFF) {
					deleted_idx = index;
					return;
				}
			}

			uint32_t& new_deleted_index = deleted.emplace_back();
			new_deleted_index = index;
		}
	}

	void clear()
	{
		nodes.clear();
		deleted.clear();

		_size = 0;
		_first_index = 0;
		_last_index = 0;
	}

	bool isDeleted(uint32_t idx)
	{
		return nodes[idx].is_deleted;
	}

	T& operator[](uint32_t idx)
	{	
		assert_cond(_first_index <= idx && idx <= _last_index, "out of bounds access");
		assert_cond(nodes[idx].is_deleted == false, "accessed element marked as deleted");
		return nodes[idx].elem;
	}

	T& front()
	{
		return nodes[_first_index].elem;
	}

	T& back()
	{
		return nodes[_last_index].elem;
	}

	DeferredVectorIterator<T> begin()
	{
		DeferredVectorIterator<T> iter;
		iter._parent_vector = this;
		iter._current_idx = _first_index;

		return iter;
	}

	// skips first N elements
	DeferredVectorIterator<T> begin(uint32_t skip_count)
	{
		DeferredVectorIterator<T> iter;
		iter._parent_vector = this;
		iter._current_idx = _first_index;

		for (uint32_t i = 0; i < skip_count; i++) {
			iter.next();
		}

		return iter;
	}

	DeferredVectorIterator<T> end()
	{
		DeferredVectorIterator<T> iter;
		iter._parent_vector = this;
		iter._current_idx = _last_index + 1;

		return iter;
	}

	// ease of refactoring
	inline uint32_t size()
	{
		return _size;
	}

	inline uint32_t firstIndex()
	{
		return _first_index;
	}

	inline uint32_t lastIndex()
	{
		return _last_index;
	}

	inline uint32_t capacity()
	{
		return nodes.capacity();
	}
};
