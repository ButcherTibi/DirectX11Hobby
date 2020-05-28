#pragma once

// Standard
#include <variant>
#include <forward_list>
#include <map>
#include <optional>

// Mine
#include "ErrorStuff.h"
#include "FileIO.h"
#include "Meshes.h"


/* is case of demand for large file do:
- use std::future and/or promise (can't remember which one is which)
- give file completion rate using file index @param i
- call destructor explicit when variable not needed to reduce menory
 */


/* move i to next character */
void skipWhiteSpace(uint64_t& i, std::vector<char>& text);

/* return true or false if symbol has been found */
bool seekSymbol(uint64_t& i, std::vector<char>& text, char symbol);

/* checks if next char sequence == keyword,
 * @param allow_white_space = should whitespaces be ignored in keyword */
ErrStack confirmKeyword(uint64_t& i, std::vector<char>& text, std::string keyword);

/* assumes i after '"' */
ErrStack parseString(uint64_t& i, std::vector<char>& text, std::string& out);

// ErrStack parseNumber(uint64_t& i, std::vector<char>& text, std::variant<int64_t, double>& out);


struct JSONValue;

struct JSONField {
	std::string name;
	JSONValue* value;
};

struct JSONValue {
	std::variant<bool,
		int64_t,
		double,
		std::string,
		std::vector<JSONValue*>,  // array
		std::vector<JSONField>,  // object
		nullptr_t>
		value;
};

class JSONGraph {
public:
	/* Since a JSON value can be an array or object thus recursive,
	 * someone must store where those JSON values point to */
	std::forward_list<JSONValue> values;

	JSONValue* root = nullptr;

private:
	/* Stored here for heap reuse in number parsing */
	std::vector<char> integer_chars;
	std::vector<char> frac_chars;

	/* Stored here for heap reuse in string parsing */
	std::string temp_string;

public:

	ErrStack parseValue(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i is at first digit of number */
	ErrStack parseNumber(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i after '[' */
	ErrStack parseArray(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i after '{' */
	ErrStack parseObject(uint64_t& i, std::vector<char>& text, JSONValue& json_value);
};

/* use @param offset to skip start of file if desired */
ErrStack parseJSON(std::vector<char>& text, uint64_t offset, JSONGraph& json);

ErrStack importGLTFMeshes(FileSysPath& path, std::vector<LinkageMesh>& meshes);
