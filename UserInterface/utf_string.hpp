#pragma once

// Standard
#include <string>
#include <vector>


// Forward
class utf8string;


/* Low level Utilities */

// looks at the byte and determines how many bytes are used for the character
// in the encoding
// returns 0 if invalid
uint32_t getUTF8_SequenceLength(uint8_t first_byte);

// get how many bytes are required to encode the code point
// returns 0 if invalid
uint32_t getUTF8_CodePointLength(uint32_t code_point);

// encode code point into bytes
// ensure you call the right one based on code point
void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2);

void encodeUTF8_CodePoint(uint32_t code_point,
	uint8_t& byte_0, uint8_t& byte_1, uint8_t& byte_2, uint8_t& byte_3);


// Upon analizing the memory cost of rendering a single character on the GPU
// The idea of using UTF-8 to save memory of rendered characters is absolutely
// hilarious and absurd (pretty easy to make a utf-8 string type)
// it takes around 120 bytes of memory of CPU and GPU to render a single character
class utf8string_iter {
public:
	utf8string* parent = nullptr;
	int32_t byte_index;

public:
	// void invalidate();
	// bool isValid();

	void prev();
	void next(uint32_t count = 1);

	uint32_t getCodePoint();
	utf8string get();
	uint8_t* data();

	bool isBeforeBegin();
	bool isAtNull();

	void swap(utf8string_iter& other);
};

bool operator==(utf8string_iter a, utf8string_iter b);
bool operator!=(utf8string_iter a, utf8string_iter b);
bool operator<(utf8string_iter a, utf8string_iter b);
bool operator>(utf8string_iter a, utf8string_iter b);
bool operator>=(utf8string_iter a, utf8string_iter b);

bool operator==(utf8string_iter a, const char8_t* utf8_string_literal);


class utf8string {
public:
	std::vector<uint8_t> bytes;

public:
	utf8string();
	utf8string(const char8_t* utf8_string_literal);
	utf8string(std::string& string);

	// return the number of code points (excludes \0)
	uint32_t length();


	/* Iterators */

	// return iterator with byte index of -1
	utf8string_iter beforeBegin();

	// return iterator with byte index of 0
	utf8string_iter begin();
	
	// return iterator with byte index of the last character which is \0
	utf8string_iter end();


	/* Insert */

	void push(uint32_t code_point);

	// writes characters from new_content to location
	void overwrite(utf8string_iter& location, utf8string& new_content);

	void overwrite(utf8string_iter& location,
		utf8string_iter& new_content_start, uint32_t new_content_length);

	void overwrite(utf8string_iter& location,
		utf8string_iter& new_content_start, utf8string_iter& new_content_end);

	// designed for use in text edit
	// deletes selection and inserts new content
	// method is faster than delete + insert
	void replaceSelection(utf8string_iter& selection_start, uint32_t selection_length,
		utf8string& new_content);


	/* Delete */
	// void erase(uint32_t start, uint32_t count);


	/* Output */

	// extract selection into a new utf8string
	utf8string extract(utf8string_iter& begin, utf8string_iter& end);
	utf8string extract(utf8string_iter& begin, uint32_t count);

	// fill other string with bytes content (excludes \0)
	void fill(std::string& other);

	// returns a C style null terminated string
	const char* c_str();

	// std::wstring w_str();
};

bool operator==(utf8string& a, const char8_t* utf8_string_literal);
bool operator!=(utf8string& a, const char8_t* utf8_string_literal);


// UTF-32 String Helpers

