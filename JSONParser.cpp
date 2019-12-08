
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

		// skip anything above whitespace
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

ErrorStack confirmKeyword(uint64_t& i, std::vector<char>& text, std::string keyword)
{
	uint64_t idx = 0;

	for (; i < text.size(); i++, idx++) {

		skipWhiteSpace(i, text);

		if (idx < keyword.length()) {

			if (text[i] != keyword[idx]) {

				return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
					msgWithPos("failed to parse keyword " + keyword,
					i, text));
			}
		}
		else {
			return ErrorStack();
		}
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
		msgWithPos("premature end of characters in file when parsing keyword " + keyword,
		i, text));
}

bool checkForKeyword(uint64_t& i, std::vector<char>& text, std::string keyword)
{
	uint64_t idx = 0;

	for (; i < text.size(); i++, idx++) {

		skipWhiteSpace(i, text);

		if (idx < keyword.length()) {

			if (text[i] != keyword[idx]) {

				return false;
			}
		}
		else {
			return true;
		}
	}
	return false;
}

/* assumes i after '"' */
ErrorStack parseString(uint64_t& i, std::vector<char>& text, std::string& out)
{
	for (; i < text.size(); i++) {

		char& c = text[i];

		// skip newline and carriage return
		if (c == '\n' || c == '\r') {
			continue;
		}

		if (c == '"') {

			i++;
			return ErrorStack();
		}
		out.push_back(c);
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
		msgWithPos("missing ending '\"' or premature end of characters in file when parsing string",
		i, text));
}

enum class NumberParseMode {
	INTEGER,
	FRACTION,
	EXPONENT
};

#define isUTF8Number(c) \
	c > 0x2f && c < 0x3a

bool convertCharsToInt(std::vector<char>& integer_part, int64_t& int_num)
{
	uint64_t m = 1;

	int32_t count = (int32_t)integer_part.size() - 1;
	for (int32_t idx = count; idx != -1; idx--) {

		int64_t digit = (int64_t)(integer_part[idx] - '0');
		int64_t a = digit * m;

		if (std::numeric_limits<int64_t>::max() - a < int_num) {
			return false;
		}

		int_num += a;
		m *= 10;
	}

	return true;
}

/* converts 1234 to 0.1234 */
bool convertCharsToFrac(std::vector<char>& frac_part, double& dbl_num)
{
	int64_t int_num = 0;
	if (!convertCharsToInt(frac_part, int_num) ||
		frac_part.size() > 18)
	{
		return false;
	}

	dbl_num = (double)int_num / std::pow(10.0, (double)frac_part.size());
	return true;
}

bool isSafeAdd(double a, double b)
{
	if ((b > 0) && (a > std::numeric_limits<double>::max() - b) ||  // overflow
		(b < 0) && (a < std::numeric_limits<double>::min() - b))  // underflow
	{
		return false;
	}
	return true;
}

// not used but noted down how to
bool isSafeSub(double a, double b)
{
	if ((b < 0) && (a > std::numeric_limits<double>::max() + b) ||  // overflow
		(b > 0) && (a < std::numeric_limits<double>::min() + b))  // underflow
	{
		return false;
	}
	return true;
}

bool isSafeMul(double a, double b)
{
	if (a > std::numeric_limits<double>::max() / b ||  // overflow
		a < std::numeric_limits<double>::min() / b)  // underflow
	{
		return false;
	}
	return true;
}

bool doSafePow(double a, double b, double& val)
{
	val = std::pow(a, b);

	if (val == std::numeric_limits<double>::infinity() ||
		val == -std::numeric_limits<double>::infinity() || 
		std::isnan(val))
	{
		return false;
	}
	return true;
}

ErrorStack JSONGraph::parseNumber(uint64_t& i, std::vector<char>& text, bool use_64int, bool use_double,
	JSONValue& number)
{
	NumberParseMode mode = NumberParseMode::INTEGER;

	int64_t int_sign = 1;
	double exp_sign = 1;

	this->integer_part.clear();
	this->frac_part.clear();
	this->exp_part.clear();

	int64_t int_num = 0;
	double frac_num = 0;

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
			else if (isUTF8Number(c)) {
				integer_part.push_back(c);
			}
			else {
				if (!convertCharsToInt(integer_part, int_num)) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
						msgWithPos("integer part of number too large to represent on int64_t",
						i, text));
				}
				int_num *= int_sign;

				if (c == '.') {
					mode = NumberParseMode::FRACTION;
					continue;
				}

				// write value
				if (use_64int) {
					number.value = (int64_t)int_num;
				}
				else {
					number.value = (int32_t)(int_num);
				}
				return ErrorStack();
			}
			break;

		case NumberParseMode::FRACTION:

			if (isUTF8Number(c)) {
				frac_part.push_back(c);
			}
			else {
				if (!convertCharsToFrac(frac_part, frac_num)) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
						msgWithPos("fraction part of number has too many digits",
						i, text));
				}

				if (!isSafeAdd((double)int_num, frac_num)) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
						msgWithPos("number too large too represent",
						i, text));
				}
				frac_num += (double)int_num;

				if (c == 'e' || c == 'E') {
					mode = NumberParseMode::EXPONENT;
					continue;
				}

				// write value
				if (use_double) {
					number.value = (double)frac_num;
				}
				else {
					number.value = (float)frac_num;
				}
				return ErrorStack();
			}
			break;

		case NumberParseMode::EXPONENT:

			if (c == '+') {
				exp_sign = 1.0;
			}
			else if (c == '-') {
				exp_sign = -1.0;
			}
			else if (isUTF8Number(c)) {
				exp_part.push_back(c);
			}
			else {
				int64_t exp_int = 0;
				if (!convertCharsToInt(exp_part, exp_int)) {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
						msgWithPos("exponent part of number too large to represent on int64_t",
						i, text));
				}

				double exp_dbl;

				if (doSafePow(10, (double)exp_int * exp_sign, exp_dbl) && 
					isSafeMul(frac_num, exp_dbl)) 
				{
					if (use_double) {
						number.value = double(frac_num * exp_dbl);
					}
					else {
						number.value = (float)(frac_num * exp_dbl);
					}
				}
				else {
					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
						msgWithPos("number too large to represent on double",
						i, text));
				}
				return ErrorStack();
			}
			break;
		}	
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
		msgWithPos("premature end of characters in file when parsing number",
		i, text));
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

			json_value.value = values;

			i++;
			return ErrorStack();
		}

		JSONValue* val = &this->values.emplace_front();

		// number
		if (isUTF8Number(c) || c == '+' || c == '-') {

			err = parseNumber(i, text, this->use_64int, this->use_double, *val);
		}
		else {
			i++;

			// string
			if (c == '"') {

				this->temp_string.clear();
				err = parseString(i, text, this->temp_string);
				val->value = temp_string;
			}
			else if (c == 't') {

				err = confirmKeyword(i, text, "rue");
				val->value = true;
			}
			// false
			else if (c == 'f') {

				err = confirmKeyword(i, text, "alse");
				val->value = false;
			}
			// null
			else if (c == 'n') {

				err = confirmKeyword(i, text, "ull");
				val->value = nullptr;
			}
			// Array
			else if (c == '[') {
				err = parseArray(i, text, *val);
			}
			// Object
			else if (c == '{') {
				err = parseObject(i, text, *val);
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
					msgWithPos("expected value but got " + asIs(c) + std::string(" instead"),
					i - 1, text));
			}
		}	

		if (err.isBad()) {
			err.pushError(code_location, "failed to parse array");
			return err;
		}

		// add value to array
		values.push_back(val);
		// skip ahead not required, already done by parser functions
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
		msgWithPos("premature end of characters in file when parsing array",
		i, text));
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
				if (err.isBad()) {

					err.pushError(code_location, "failed to parse object");
					return err;
				}
				else if (new_field->name.empty()) {

					return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location, 
						msgWithPos("expected field name is blank",
						i - 1, text));
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

				json_value.value = fields;

				i++;
				return ErrorStack();
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
					msgWithPos("expected field name or object end symbol '}' but got " + asIs(c) + std::string(" instead"),
					i, text));
			}
			break;

		case ObjectParseMode::VALUE:
			if (c == ':') {

				i++;
				skipWhiteSpace(i, text);
				c = text[i];

				JSONValue& field_value = this->values.emplace_front();

				// Number
				if (isUTF8Number(c) || c == '+' || c == '-') {

					err = parseNumber(i, text, this->use_64int, this->use_double, field_value);
				}
				else {
					i++;

					// String
					if (c == '"') {

						this->temp_string.clear();
						err = parseString(i, text, temp_string);
						field_value.value = temp_string;
					}
					// true
					else if (c == 't') {

						err = confirmKeyword(i, text, "rue");
						field_value.value = true;
					}
					// false
					else if (c == 'f') {

						err = confirmKeyword(i, text, "alse");
						field_value.value = false;
					}
					// null
					else if (c == 'n') {

						err = confirmKeyword(i, text, "ull");
						field_value.value = nullptr;
					}
					// Array
					else if (c == '[') {
						err = parseArray(i, text, field_value);
					}
					// Object
					else if (c == '{') {
						err = parseObject(i, text, field_value);
					}
					else {
						return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
							msgWithPos("expected value but got " + asIs(c) + std::string(" instead"),
							i - 1, text));
					}
				}

				if (err.isBad()) {
					err.pushError(code_location, "failed to parse object");
					return err;
				}

				// add found value to field
				new_field->value = &field_value;
				mode = ObjectParseMode::FIELD_NAME;
				// skip ahead not required, already done by parser functions
			}
			else {
				return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
					msgWithPos("expected separator ':' between field name and value but got " + asIs(c) + std::string(" instead"),
					i, text));
			}
		}
	}
	return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
		msgWithPos("premature end of characters in file when parsing object",
		i, text));
}

ErrorStack parseJSON(std::vector<char>& text, uint64_t offset, bool use_64int, bool use_double,
	JSONGraph& json)
{
	uint64_t i = offset;

	if (!seekSymbol(i, text, '{')) {

		return ErrorStack(ExtraError::FAILED_TO_PARSE_JSON, code_location,
			msgWithPos("expected object begin symbol '{' not found",
			i, text));
	}

	json.use_64int = use_64int;
	json.use_double = use_double;
	JSONValue& root = json.values.emplace_front();

	ErrorStack err = json.parseObject(i, text, root);
	if (err.isBad()) {
		return err;
	}

	// root is the first element added thus last in forward_list
	std::forward_list<JSONValue>::iterator last_it = json.values.before_begin();

	for (auto it = json.values.begin(); it != json.values.end(); ++last_it, ++it) {}

	json.root = &(*last_it);

	return ErrorStack();
} 
