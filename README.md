My attempts to make something . . .

UserInterface is meant to be a UI library using the DirectX 11 API.
Sculpt is a test application that uses the library.

### User Interface

---

The UI is build using a graph of elements that are attached to each other in a parent child relationship.

**Wrap**
Is a container of other UI elements, all child components have their position relative to wrap, children that fall outside of the wrap element can be clipped by setting the `overflow` property to `Overflow::CLIP`.
The size of the element can be specified in absolute, relative units or as `ElementSizeType::FIT` meaning it will resize itself to fit its children.


**Text**
Is a text label that supports adjusting its size, line height and color.

**Surface**
Is a element  for rendering outside of the library using special surface events that hold relevant rendering context eg. Device, DeviceContext, Shader Resource Views, Render Target Views, viewport size and position etc.

Elements have components attached to them to add additional functionallity.

**NodeComp**
Is a component present on all UI elements and is responsable for enabling further attaching by other componets.

**EventComp**
Is the event handling component for the following events:
- mouse enter
- mouse hover
- mouse move
- mouse leave
- mouse delta capture begin
- mouse delta capture begin
- mouse delta capture end
- key down
- key held down
- key up

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
