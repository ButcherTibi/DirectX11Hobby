#pragma once

// Standard
#include <variant>
#include <forward_list>

#include "FilePath.hpp"
#include "ErrorStack.hpp"


// TODO maybe: store line and column, idx of the text file in json::Value for error messages of consuming uses
namespace json {

	// Forward declaration
	struct Value;

	/* move i to next character */
	void _skipWhiteSpace(uint64_t& i, std::vector<uint8_t>& text);

	/* return true or false if symbol has been found */
	bool _seekSymbol(uint64_t& i, std::vector<uint8_t>& text, char symbol);

	/* checks if next char sequence == keyword */
	ErrStack _confirmKeyword(uint64_t& i, std::vector<uint8_t>& text, std::string keyword);

	/* assumes i after '"' */
	ErrStack _parseString(uint64_t& i, std::vector<uint8_t>& text, std::string& out);


	struct Field {
		std::string name;
		Value* value;
	};

	struct Value {
		std::variant<bool,
			double,
			std::string,
			std::vector<Value*>,  // array
			std::vector<Field>,  // object
			nullptr_t>
			value;
	};


	class Graph {
	public:
		std::forward_list<Value> values;
		Value* root;

		// Caching
		std::vector<char> integer_chars;
		std::vector<char> frac_chars;
		std::string temp_string;

	public:
		ErrStack _parseValue(uint64_t& i, std::vector<uint8_t>& text, Value& json_value);

		/* assumes i is at first digit of number */
		ErrStack _parseNumber(uint64_t& i, std::vector<uint8_t>& text, Value& json_value);

		/* assumes i after '[' */
		ErrStack _parseArray(uint64_t& i, std::vector<uint8_t>& text, Value& json_value);

		/* assumes i after '{' */
		ErrStack _parseObject(uint64_t& i, std::vector<uint8_t>& text, Value& json_value);

	public:
		ErrStack importJSON(std::vector<uint8_t>& text);
	};
}
