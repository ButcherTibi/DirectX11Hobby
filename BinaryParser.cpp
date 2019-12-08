
// Stadard


// Header
#include "Importer.h"


void bin::Vector::push_back(bool bit)
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

/* converts one Base64 character to 6 bits */
bool bin::Vector::pushBase64Char(char c)
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
		d = 0;
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

ErrorStack bin::loadFromURI(std::vector<char>& uri, Path& this_file, Vector& r_bin)
{
	ErrorStack err;

	uint64_t uri_i = 0;

	// Data URI
	if (checkForKeyword(uri_i, uri, "data:application/octet-stream;base64,")) {

		r_bin.bytes.reserve(size_t(uri.size() * 0.76f));

		// decode rest of URI as Base64 binary
		for (; uri_i < uri.size(); uri_i++) {

			skipWhiteSpace(uri_i, uri);

			r_bin.pushBase64Char(uri[uri_i]);
		}
	}
	// Relative URI path
	else {
		std::string uri_path{ uri.begin(), uri.end() };

		Path bin_file = this_file;
		bin_file.pop_back();  // now point to directory containing file
		bin_file.push_back(uri_path);  // point to file

		if (bin_file.hasExtension("bin")) {

			// load directly 
			err = bin_file.read(r_bin.bytes);
			if (err.isBad()) {
				return err;
			}
			r_bin.bit_count = r_bin.bytes.size() * 8;
		}
		// TODO: GLB format
	}

	return ErrorStack();
}

template<typename T>
std::string bitsToString(T bits)
{
	std::string b = "";

	for (int8_t i = sizeof(T) * 8; i >= 0; i--) {

		if (bits & (1 << i)) {
			b.push_back('1');
		}
		else {
			b.push_back('0');
		}

		if (i % 8 == 0 && (sizeof(T) * 8 > i) && (i > 0)) {
			b.push_back(' ');
		}
	}

	return b;
}

ErrorStack loadIndexesFromBuffer(uint64_t offset, uint64_t component_type, uint64_t count,
	bin::Vector binary, std::vector<uint32_t>& indexes)
{
	indexes.resize(count);

	switch (component_type) {
	// fast path
	case gltf::UNSIGNED_INT: {
		std::memcpy(indexes.data(), binary.bytes.data() + offset, sizeof(uint32_t) * indexes.size());
		break;
	}		

	case gltf::BYTE: {
		std::vector<uint8_t> uint8_idxs;
		uint8_idxs.resize(count);

		std::memcpy(uint8_idxs.data(), binary.bytes.data() + offset, sizeof(uint8_t) * uint8_idxs.size());

		for (uint64_t i = 0; i < count; i++) {
			indexes[i] = uint8_idxs[i];
		}
		break;
	}

	case gltf::UNSIGNED_SHORT: {
		std::vector<uint16_t> uint16_idxs;
		uint16_idxs.resize(count);

		std::memcpy(uint16_idxs.data(), binary.bytes.data() + offset, sizeof(uint16_t) * uint16_idxs.size());

		for (uint64_t i = 0; i < count; i++) {
			indexes[i] = uint16_idxs[i];
		}
		break;
	}
	default:
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"invalid component_type for index buffer allowed types are BYTE, UNSIGNED_SHORT, UNSIGNED_INT");
	}

	return ErrorStack();
}

ErrorStack loadVec3FromBuffer(uint64_t offset, uint64_t component_type, uint64_t count,
	bin::Vector binary, std::vector<glm::vec3>& vecs)
{
	vecs.resize(count);

	switch (component_type) {
	// fast path
	case gltf::FLOAT: {
		// PARANOIA: maybe sizeof(glm::vec3) != sizeof(float) * 3
		std::memcpy(vecs.data(), binary.bytes.data() + offset, sizeof(glm::vec3) * count);
		break;
	}

	default:
		return ErrorStack(ExtraError::FAILED_TO_PARSE_GLTF, code_location,
			"invalid component_type for position, can only be VEC3");
	}

	return ErrorStack();
}
