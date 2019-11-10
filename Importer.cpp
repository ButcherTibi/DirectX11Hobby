
// Standard

// Header
#include "Importer.h"


void JSONValue::initInteger(int32_t int_num)
{
	this->type = JSONType::INTEGER;
	this->value = int_num;
}

void JSONValue::initFloat(float float_num)
{
	this->type = JSONType::FLOAT;
	this->value = float_num;
}

void JSONValue::initString(const std::string& string_jt)
{
	this->type = JSONType::STRING;
	this->value = string_jt;
}

void JSONValue::initBool(bool val)
{
	this->type = JSONType::BOOL;
	this->value = val;
}

void JSONValue::initNull()
{
	this->type = JSONType::NOTHING;
	this->value = nullptr;
}

/* move i to next character */
void skipWhiteSpace(uint64_t& i, std::vector<char>& text)
{
	for (; i < text.size(); i++) {

		char& c = text[i];

		if (c == '\n' || c == '\r' || c == ' ') {
			continue;
		}
		return;
	}
}

/* return true or false if symbol has been found */
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

/* assumes i after '"' */
ErrorStack parseString(uint64_t& i, std::vector<char>& text, std::string& out)
{
	for (; i < text.size(); i++) {

		skipWhiteSpace(i, text);

		char& c = text[i];

		if (c == '"') {

			i++;
			return ErrorStack();
		}
		out.push_back(c);
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
		"missing ending '\"' or premature end of characters in file when parsing string",
		i, text);
}

enum class NumberParseMode {
	INTEGER,
	FRACTION,
	EXPONENT
};

#define isUTF8Number(c) \
	c > 0x2f && c < 0x3a

uint32_t convertCharsToInt(std::vector<char>& integer_part)
{
	uint32_t m = 1;
	uint32_t int_num = 0;

	for (size_t idx = integer_part.size() - 1; idx > 0; idx--) {

		char& num = integer_part[idx];

		int_num += num * m;
		m *= 10;
	}
	return int_num;
}

float convertCharsToFrac(std::vector<char>& frac_part)
{
	return convertCharsToInt(frac_part) / (10.0f * (float)frac_part.size());
}

// TODO: guards against overflow
ErrorStack JSONGraph::parseNumber(uint64_t& i, std::vector<char>& text, JSONValue& number)
{
	NumberParseMode mode = NumberParseMode::INTEGER;

	int32_t int_sign = 1;
	float exp_sign = 1;

	this->integer_part.clear();
	this->frac_part.clear();
	this->exp_part.clear();

	for (; i < text.size(); i++) {

		skipWhiteSpace(i, text);

		char& c = text[i];

		switch (mode) {
		case NumberParseMode::INTEGER:

			if (c == '+') {
				int_sign = 1;
			}
			else if (c == '-') {
				int_sign = -1;
			}
			// if number in UTF-8 format
			else if (isUTF8Number(c)) {
				integer_part.push_back(c);
			}
			else if (c == '.') {
				mode = NumberParseMode::FRACTION;
			}
			else {
				number.initInteger(convertCharsToInt(integer_part) * int_sign);
				return ErrorStack();
			}
			break;

		case NumberParseMode::FRACTION:

			if (isUTF8Number(c)) {
				frac_part.push_back(c);
			}
			else if (c == 'e' || c == 'E') {
				mode = NumberParseMode::EXPONENT;
			}
			else {
				float float_num = (float)convertCharsToInt(integer_part) * int_sign + convertCharsToFrac(frac_part);
				number.initFloat(float_num);
				return ErrorStack();
			}
			break;

		case NumberParseMode::EXPONENT:

			if (c == '+') {
				exp_sign = 1;
			}
			else if (c == '-') {
				exp_sign = -1;
			}
			// if number in UTF-8 format
			else if (isUTF8Number(c)) {
				exp_part.push_back(c);
			}
			else {
				float float_num = (float)convertCharsToInt(integer_part) + convertCharsToFrac(frac_part);
				float_num *= std::powf(10, convertCharsToFrac(exp_part) * exp_sign);
				number.initFloat(float_num);
				return ErrorStack();
			}
			break;
		}	
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
		"premature end of characters in file when parsing number",
		i, text);
}

/* checks if next char sequence == keyword */
ErrorStack confirmKeyword(uint64_t& i, std::vector<char>& text, std::string keyword)
{
	uint8_t idx = 0;

	for (; i < text.size(); i++, idx++) {

		skipWhiteSpace(i, text);

		if (idx < keyword.length()) {

			if (text[i] != keyword[idx]) {

				return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
					"failed to parse keyword " + keyword,
					i, text);
			}
		}
		else {
			return ErrorStack();
		}
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
		"premature end of characters in file when parsing keyword " + keyword,
		i, text);
}

ErrorStack JSONGraph::parseArray(uint64_t& i, std::vector<char>& text, JSONValue& json_value)
{
	ErrorStack err;

	std::vector<JSONValue*> values;

	for (; i < text.size(); ) {

		skipWhiteSpace(i, text);
		char& c = text[i];

		// skip to next value
		if (c == ',') {

			i++;
			continue;
		}
		else if (c == ']') {

			json_value.type = JSONType::ARRAY;
			json_value.value = values;

			i++;
			return ErrorStack();
		}

		JSONValue* val = &this->json_values.emplace_back();

		// number
		if (isUTF8Number(c)) {

			err = parseNumber(i, text, *val);

			if (val->type == JSONType::INTEGER) {
				printf("value = %d \n", std::get<int32_t>(val->value));
			}
			else if (val->type == JSONType::FLOAT) {
				printf("value = %f \n", std::get<float>(val->value));
			}
		}
		else {
			i++;

			// string
			if (c == '"') {

				this->temp_string = "";
				err = parseString(i, text, this->temp_string);
				val->initString(temp_string);

				printf("value = %s \n", temp_string.c_str());
			}
			else if (c == 't') {

				err = confirmKeyword(i, text, "rue");
				val->initBool(true);

				printf("value = true \n");
			}
			// false
			else if (c == 'f') {

				err = confirmKeyword(i, text, "alse");
				val->initBool(false);

				printf("value = false \n");
			}
			// null
			else if (c == 'n') {

				err = confirmKeyword(i, text, "ull");
				val->initNull();

				printf("value = null \n");
			}
			// Array
			else if (c == '[') {

				printf("value = array \n");

				err = parseArray(i, text, *val);
			}
			// Object
			else if (c == '{') {

				printf("value = object \n");

				err = parseObject(i, text, *val);
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
					"expected value but got " + asIs(c) + std::string(" instead"),
					i - 1, text);
			}
		}	

		if (err.isBad()) {
			err.report(code_location, "failed to parse array");
			return err;
		}

		// add value to array
		values.push_back(val);
		// skip ahead not required, already done by parser functions
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
		"premature end of characters in file when parsing array",
		i, text);
}

enum class ObjectParseMode {
	FIELD_NAME,
	VALUE
};

ErrorStack JSONGraph::parseObject(uint64_t& i, std::vector<char>& text, JSONValue& json_value)
{
	ErrorStack err;

	ObjectParseMode mode = ObjectParseMode::FIELD_NAME;

	std::vector<JSONField> fields;

	JSONField* new_field = nullptr;

	for (; i < text.size(); ) {

		skipWhiteSpace(i, text);
		char& c = text[i];

		switch (mode)
		{
		case ObjectParseMode::FIELD_NAME:
			if (c == '"') {

				// create new field
				new_field = &fields.emplace_back();

				i++;
				err = parseString(i, text, new_field->name);

				printf("field = %s \n", new_field->name.c_str());

				if (err.isBad()) {

					err.report(code_location, "failed to parse object");
					return err;
				}
				else if (new_field->name.empty()) {

					return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location, "expected field name is blank",
						i - 1, text);
				}

				mode = ObjectParseMode::VALUE;
				i++;
			}
			// another field after so skip
			else if (c == ',') {
				i++;
				continue;
			}
			// no more fields or object is empty
			else if (c == '}') {

				json_value.type = JSONType::OBJECT;
				json_value.value = fields;

				i++;
				return ErrorStack();
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
					"expected field name or object end symbol '}' but got " + asIs(c) + std::string(" instead"),
					i, text);
			}
			break;

		case ObjectParseMode::VALUE:
			if (c == ':') {

				i++;
				skipWhiteSpace(i, text);
				c = text[i];

				JSONValue& field_value = this->json_values.emplace_back();

				// Number
				if (isUTF8Number(c)) {

					err = parseNumber(i, text, field_value);
				}
				else {
					i++;

					// String
					if (c == '"') {

						this->temp_string = "";
						err = parseString(i, text, temp_string);
						field_value.initString(temp_string);

						printf("value = %s \n", temp_string.c_str());
					}
					// true
					else if (c == 't') {

						err = confirmKeyword(i, text, "rue");
						field_value.initBool(true);

						printf("value = true \n");
					}
					// false
					else if (c == 'f') {

						err = confirmKeyword(i, text, "alse");
						field_value.initBool(false);

						printf("value = false \n");
					}
					// null
					else if (c == 'n') {

						err = confirmKeyword(i, text, "ull");
						field_value.initNull();

						printf("value = null \n");
					}
					// Array
					else if (c == '[') {

						printf("value = array \n");

						err = parseArray(i, text, field_value);
					}
					// Object
					else if (c == '{') {

						printf("value = object \n");

						err = parseObject(i, text, field_value);
					}
					else {
						return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
							"expected value but got " + asIs(c) + std::string(" instead"),
							i - 1, text);
					}
				}

				if (err.isBad()) {
					err.report(code_location, "failed to parse object");
					return err;
				}

				// add found value to field
				new_field->value = &field_value;
				mode = ObjectParseMode::FIELD_NAME;
				// skip ahead not required, already done by parser functions
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
					"expected separator ':' between field name and value but got " + asIs(c) + std::string(" instead"),
					i, text);
			}
		}
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
		"premature end of characters in file when parsing object",
		i, text);
}

ErrorStack importGLTFMesh(Path path, LinkageMesh& mesh)
{
	ErrorStack err;

	std::vector<char> text;
	err = path.read(text);

	if (err.isBad()) {
		return err;
	}

	JSONGraph json_graph;

	uint64_t i = 0;
	if (!seekSymbol(i, text, '{')) {

		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"expected object begin symbol '{' not found",
			i, text);
	}

	err = json_graph.parseObject(i, text, json_graph.json_values.emplace_back());

	return err;
}