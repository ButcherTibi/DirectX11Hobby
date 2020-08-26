
// Standard
#include <numeric>

// Header
#include "Importer.h"


std::string msgWithPos(std::string msg, uint64_t i, std::vector<char>& text)
{
	uint64_t line = 1;
	uint64_t col = 1;

	for (uint64_t idx = 0; idx <= i; idx++) {

		char& c = text[idx];

		if (c == '\n') {
			line++;
			col = 1;
			continue;
		}
		col++;
	}

	return msg +
		" at ln= " + std::to_string(line) + " col= " + std::to_string(col - 1);
}

void skipWhiteSpace(uint64_t& i, std::vector<char>& text)
{
	for (; i < text.size(); i++) {

		char& c = text[i];

		// skip anything below whitespace
		if (c < 0x21) {
			continue;
		}
		return;
	}
}

bool seekSymbol(uint64_t& i, std::vector<char>& text, char symbol)
{
	for (; i < text.size(); i++) {

		skipWhiteSpace(i, text);

		char& c = text[i];

		if (c == symbol) {
			i++;
			return true;
		}
	}
	return false;
}

ErrStack confirmKeyword(uint64_t& i, std::vector<char>& text, std::string keyword)
{
	uint64_t idx = 0;

	for (; i < text.size(); i++, idx++) {

		skipWhiteSpace(i, text);

		if (idx < keyword.length()) {

			if (text[i] != keyword[idx]) {

				return ErrStack(code_location,
					msgWithPos("failed to parse keyword " + keyword,
					i, text));
			}
		}
		else {
			return ErrStack();
		}
	}
	return ErrStack(code_location,
		msgWithPos("premature end of characters in file when parsing keyword " + keyword,
		i, text));
}

/* assumes i after '"' */
ErrStack parseString(uint64_t& i, std::vector<char>& text, std::string& out)
{
	for (; i < text.size(); i++) {

		char& c = text[i];

		// skip newline and carriage return
		if (c == '\n' || c == '\r') {
			continue;
		}
		else if (c == '"') {
			i++;
			return ErrStack();
		}
		else {
			out.push_back(c);
		}
	}
	return ErrStack(code_location,
		msgWithPos("missing ending '\"' or premature end of characters in file when parsing string",
		i, text));
}

enum class NumberParseMode {
	INTEGER,
	FRACTION
};

#define isUTF8Number(c) \
	c > 0x2f && c < 0x3a

void convertCharsToInt(std::vector<char>& integer_chars, int64_t& int_num)
{
	uint64_t m = 1;

	int32_t count = (int32_t)integer_chars.size() - 1;
	for (int32_t idx = count; idx != -1; idx--) {

		int64_t digit = (int64_t)(integer_chars[idx] - '0');
		int64_t a = digit * m;

		int_num += a;
		m *= 10;
	}
}

ErrStack JSONGraph::parseNumber(uint64_t& i, std::vector<char>& text, JSONValue& number)
{
	this->integer_chars.clear();
	this->frac_chars.clear();

	NumberParseMode mode = NumberParseMode::INTEGER;

	int64_t int_sign = 1;
	int64_t int_part = 0;

	for (; i < text.size(); i++) {

		char& c = text[i];

		switch (mode) {
		case NumberParseMode::INTEGER:

			if (c == '+') {
				int_sign = 1;
			}
			else if (c == '-') {
				int_sign = -1;
			}
			else if (isUTF8Number(c)) {
				integer_chars.push_back(c);
			}
			else {
				convertCharsToInt(integer_chars, int_part);
				int_part *= int_sign;

				if (c == '.') {
					mode = NumberParseMode::FRACTION;
				}
				else {
					number.value = (int64_t)int_part;
					return ErrStack();
				}			
			}
			break;

		case NumberParseMode::FRACTION:

			if (isUTF8Number(c)) {
				frac_chars.push_back(c);
			}
			else {
				// Extract 0.1234
				int64_t frac_part;
				convertCharsToInt(frac_chars, frac_part);
				double frac_part_dbl = (double)frac_part / std::pow(10.0, (double)frac_chars.size());

				number.value = (double)int_part + frac_part_dbl;
				return ErrStack();
			}
			break;;
		}	
	}
	return ErrStack(code_location,
		msgWithPos("premature end of characters in file when parsing number",
		i, text));
}

ErrStack JSONGraph::parseValue(uint64_t& i, std::vector<char>& text, JSONValue& json_value)
{
	ErrStack err_stack;

	skipWhiteSpace(i, text);
	char& c = text[i];

	if (isUTF8Number(c) || c == '+' || c == '-') {
		err_stack = parseNumber(i, text, json_value);
	}
	else {
		i++;

		// string
		if (c == '"') {
			temp_string.clear();
			err_stack = parseString(i, text, temp_string);
			json_value.value = temp_string;
		}
		else if (c == 't') {
			err_stack = confirmKeyword(i, text, "rue");
			json_value.value = true;
		}
		// false
		else if (c == 'f') {
			err_stack = confirmKeyword(i, text, "alse");
			json_value.value = false;
		}
		// null
		else if (c == 'n') {
			err_stack = confirmKeyword(i, text, "ull");
			json_value.value = nullptr;
		}
		// Array
		else if (c == '[') {
			err_stack = parseArray(i, text, json_value);
		}
		// Object
		else if (c == '{') {
			err_stack = parseObject(i, text, json_value);
		}
		else {
			return ErrStack(code_location,
				msgWithPos("expected value but got " + asIs(c) + std::string(" instead"),
					i, text));
		}
	}
	
	checkErrStack(err_stack, "failed to parse value");
	return ErrStack();
}

ErrStack JSONGraph::parseArray(uint64_t& i, std::vector<char>& text, JSONValue& json_value)
{
	std::vector<JSONValue*> array_vals;

	// empty array
	if (text[i] == ']') {
		json_value.value = array_vals;
		i++;
		return ErrStack();
	}

	ErrStack err_stack;

	for (; i < text.size(); i++) {

		skipWhiteSpace(i, text);

		// Value
		JSONValue* val = &this->values.emplace_front();
		checkErrStack(parseValue(i, text, *val), "");

		array_vals.push_back(val);

		skipWhiteSpace(i, text);

		// Next
		if (text[i] == ',') {
			continue;
		}
		// End of Array
		else if (text[i] == ']') {
			json_value.value = array_vals;
			i++;
			return ErrStack();
		}
		else {
			return ErrStack(code_location, msgWithPos(
				"expected separator ',' between array values or array end symbol ']'", i, text));
		}
	}
	return ErrStack(code_location,
		msgWithPos("premature end of characters in file when parsing array",
		i, text));
}

ErrStack JSONGraph::parseObject(uint64_t& i, std::vector<char>& text, JSONValue& json_value)
{
	ErrStack err_stack;

	std::vector<JSONField> fields;

	// Empty Object
	skipWhiteSpace(i, text);
	if (text[i] == '}') {
		json_value.value = fields;
		i++;
		return ErrStack();
	}
	
	JSONField* new_field = nullptr;

	for (; i < text.size(); i++) {

		skipWhiteSpace(i, text);

		if (text[i] == '"') {

			// Parse Field Name
			i++;
			new_field = &fields.emplace_back();
			checkErrStack(parseString(i, text, new_field->name), 
				"failed to parse field name");

			if (new_field->name.empty()) {
				return ErrStack(code_location,
					msgWithPos("expected field name is blank",
						i - 1, text));
			}

			// Parse Value
			skipWhiteSpace(i, text);
			if (text[i] == ':') {
			
				i++;
				new_field->value = &this->values.emplace_front();
				checkErrStack(parseValue(i, text, *new_field->value), 
					"failed to parse field value");

				skipWhiteSpace(i, text);

				// Field separator
				if (text[i] == ',') {
					continue;
				}
				// End of Object
				else if (text[i] == '}') {
					json_value.value = fields;
					i++;
					return ErrStack();
				}
				else {
					return ErrStack(code_location, msgWithPos(
						"expected separator ',' between fields or object end symbol '}'", i, text));
				}
			}
			else {
				return ErrStack(code_location, msgWithPos(
					"expected separator ':' between field name and value", i, text));
			}
		}
		else {
			return ErrStack(code_location, msgWithPos(
				"expected field name", i, text));
		}
	}
	return ErrStack(code_location, msgWithPos(
		"premature end of characters in file when parsing object",
		i, text));
}

ErrStack parseJSON(std::vector<char>& text, uint64_t offset,
	JSONGraph& json)
{
	uint64_t i = offset;

	if (!seekSymbol(i, text, '{')) {

		return ErrStack(code_location,
			msgWithPos("expected object begin symbol '{' not found",
			i, text));
	}

	ErrStack err = json.parseObject(i, text, json.values.emplace_front());
	if (err.isBad()) {
		return err;
	}

	// root is the first element added thus last in forward_list
	std::forward_list<JSONValue>::iterator last_it = json.values.before_begin();

	for (auto it = json.values.begin(); it != json.values.end(); ++last_it, ++it) {}

	json.root = &(*last_it);

	return ErrStack();
} 
