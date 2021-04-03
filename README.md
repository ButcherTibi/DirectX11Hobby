My attempts to make something . . .

UserInterface is meant to be a UI library using the DirectX 11 API.
Sculpt is a test application that uses the library.

### Other Stuff

---

**FilePath**
Uses the Win32 API to read a file into a vector, file path may be absolute or relative to executable directory.

**TextureAtlas**
An implementations of a texture atlas which packs monocrome bitmap images. Supports autoresize when not enough space is available.

**CharacterAtlas**
Uses FreeType library to rasterize characters and structure them into Font->Size->Character in a texture atlas.

**JSON Importer**
Imports the data in a JSON file as a graph of values, a values may of the following types:
- boolean
- int64_t
- double
- string
- vector of other values (json array type)
- vector of fields (json object type)

**BitVector**
Stores bits in a vector of uint8_t values, supports adding one bit at a time or one Base64 character that will be converted into the coresponding 6 bits of data.

**GLTF Mesh Only Importer**
Extracts the indexes, positions and normals of a mesh stored in the GLTF file. No support for material, animations or other primitives.
