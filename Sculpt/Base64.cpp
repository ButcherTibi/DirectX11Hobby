
// Header
#include "GLTF_File.hpp"


void base64::BitVector::push_back(bool bit)
{
	if (!bit_count || bit_count % 8 == 0) {
		bytes.push_back(bit << 7);
	}
	else {
		uint8_t next_bit = 8 - (bit_count % 8) - 1;
		bytes[bit_count / 8] |= bit << next_bit;
	}
	bit_count++;
}

bool base64::BitVector::pushBase64Char(char c)
{
	uint8_t d;

	// A to Z
	if (c > 0x40 && c < 0x5b) {
		d = c - 65;  // Base64 A is 0
	}
	// a to z
	else if (c > 0x60 && c < 0x7b) {
		d = c - 97 + 26;  // Base64 a is 26
	}
	// 0 to 9
	else if (c > 0x2F && c < 0x3a) {
		d = c - 48 + 52;  // Base64 0 is 52
	}
	else if (c == '+') {
		d = 0b111110;
	}
	else if (c == '/') {
		d = 0b111111;
	}
	else if (c == '=') {
		d = 0;  // padding
	}
	else {
		return false;
	}

	push_back(d & 0b100000);
	push_back(d & 0b010000);
	push_back(d & 0b001000);
	push_back(d & 0b000100);
	push_back(d & 0b000010);
	push_back(d & 0b000001);

	return true;
}