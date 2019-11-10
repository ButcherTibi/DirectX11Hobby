#pragma once

// Standard
#include <variant>

// Mine
#include "ErrorStuff.h"
#include "FileIO.h"
#include "Meshes.h"


enum class JSONType {
	INTEGER,
	FLOAT,
	STRING,
	BOOL,
	NOTHING, // nullptr
	ARRAY,
	OBJECT
};


struct JSONValue;


struct JSONField {
	std::string name;
	JSONValue* value;
};


struct JSONValue {
	JSONType type;

	std::variant<int32_t, float, std::string, bool, nullptr_t,  // primitives
		std::vector<JSONValue*>,  // array
		std::vector<JSONField>>   // object
		value;

public:
	void initInteger(int32_t int_num);
	void initFloat(float float_num);
	void initString(const std::string& string_jt);
	void initBool(bool val);
	void initNull();
};


class JSONGraph {
public:
	/* Since a JSON value can be an array or object thus recursive,
	 * someone must store where those JSON values point to */
	std::vector<JSONValue> json_values;

	/* Stored here for heap reuse in number parsing */
	std::vector<char> integer_part;
	std::vector<char> frac_part;
	std::vector<char> exp_part;

	/* Stored here for heap reuse in object parsing */
	std::string temp_string;

public:
	/* assumes i is at number */
	ErrorStack parseNumber(uint64_t& i, std::vector<char>& text, JSONValue& number);

	/* assumes i after '[' */
	ErrorStack parseArray(uint64_t& i, std::vector<char>& text, JSONValue& json_value);

	/* assumes i after '{' */
	ErrorStack parseObject(uint64_t& i, std::vector<char>& text, JSONValue& json_value);
};


struct Asset
{
	float version;
};


struct GLTFStruct {

	Asset asset;
};

ErrorStack importGLTFMesh(Path path, LinkageMesh& mesh);