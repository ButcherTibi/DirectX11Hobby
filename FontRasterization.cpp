
// FreeType font rasterization
#include "ft2build.h"
#include FT_FREETYPE_H

// Header
#include "TextRendering.h"


uint32_t deltaU32(uint32_t a, uint32_t b)
{
	return a > b ? a - b : b - a;
}

RasterizedCharacter* CharacterMesh::findBestFitRaster(uint32_t size)
{
	RasterizedCharacter* best_raster = &raster_sizes.front();

	for (RasterizedCharacter& raster : raster_sizes) {

		if (deltaU32(raster.raster_size, size) < deltaU32(best_raster->raster_size, size)) {
			best_raster = &raster;
		}
	}
	
	return best_raster;
}

ErrStack TextStuff::addFont(std::vector<uint8_t>& font_ttf, FontInfo& info)
{
	FT_Error ft_err = 0;
	FT_Library free_type;

	// Prepare for rasterization
	if (FT_Init_FreeType(&free_type)) {
		return ErrStack(code_location, "failed to init FreeType library");
	}

	FT_Face face;
	if (FT_New_Memory_Face(free_type, font_ttf.data(), (FT_Long)font_ttf.size(), 0, &face)) {
		return ErrStack(code_location, "failed to create font face");
	}

	Font& font = this->fonts.emplace_back();
	font.font_family = info.family_name;
	font.font_style = info.style_name;
	

	uint32_t char_idx = 0;
	if (char_meshs.size()) {
		char_idx = (uint32_t)char_meshs.size() - 1;
	}

	this->char_meshs.resize(char_meshs.size() + 95);
	
	for (uint32_t unicode = ' '; unicode <= '~'; unicode++) {

		CharacterMesh& mesh = char_meshs[char_idx];
		mesh.font = &font;
		mesh.unicode = unicode;
		mesh.raster_sizes.resize(info.sizes_px.size());

		for (uint32_t i = 0; i < info.sizes_px.size(); i++) {

			uint32_t size_px = info.sizes_px[i];
			RasterizedCharacter& raster_char = mesh.raster_sizes[i];
			raster_char.raster_size = size_px;

			ft_err = FT_Set_Pixel_Sizes(face, 0, size_px);
			if (ft_err) {
				return ErrStack(code_location, "failed to set font size");
			}

			uint32_t glyph_idx = FT_Get_Char_Index(face, unicode);
			if (!glyph_idx) {
				return ErrStack(code_location, "character not found in font file");
			}

			ft_err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
			if (ft_err) {
				return ErrStack(code_location, "failed to load glyph");
			}

			if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
				ft_err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				if (ft_err) {
					return ErrStack(code_location, "failed to render glyph");
				}
			}

			// Copy rendered pixels to character
			FT_Bitmap& ft_bitmap = face->glyph->bitmap;
			BasicBitmap& bitmap = raster_char.bitmap;

			bitmap.width = ft_bitmap.width;
			bitmap.height = ft_bitmap.rows;
			bitmap.colors.resize(ft_bitmap.width * ft_bitmap.rows);
			bitmap.channels = 1;

			std::memcpy(bitmap.colors.data(), ft_bitmap.buffer, bitmap.colors.size());

			// Metrics
			raster_char.width = face->glyph->metrics.width / 64.0f;
			raster_char.height = face->glyph->metrics.height / 64.0f;
			raster_char.baseline_height = face->glyph->metrics.horiBearingY / 64.0f;
			raster_char.advance = face->glyph->advance.x / 64.0f;
		}

		if (mesh.unicode == '_') {
			[]() {};
		}

		// Set Vertices from last size
		{
			float width = (float)mesh.raster_sizes.back().bitmap.width;
			float height = (float)mesh.raster_sizes.back().bitmap.height;
	
			width = width / height;
			height = 1;

			/*if (width < height) {
				width = width / height;
				height = 1;
			}
			else if (width > height) {
				height = height / width;
				width = 1;
			}*/

			// Same as UV
			mesh.verts[0] = { 0, height };
			mesh.verts[1] = { width, height };
			mesh.verts[2] = { width, 0 };
			mesh.verts[3] = { 0, 0 };
		}
		
		char_idx++;
	}

	return ErrStack();
}

ErrStack TextStuff::rebindToAtlas(uint32_t atlas_size)
{
	// Find max bitmap Character size
	this->max_char_width = 0;
	this->max_char_height = 0;
	{
		for (CharacterMesh& mesh : char_meshs) {
			uint32_t new_width = mesh.raster_sizes.back().bitmap.width;
			uint32_t new_height = mesh.raster_sizes.back().bitmap.height;

			max_char_width = new_width > max_char_width ? new_width : max_char_width;
			max_char_height = new_height > max_char_height ? new_height : max_char_height;
		}
	}
	atlas.recreate(atlas_size, 1, max_char_width, max_char_height);
	
	for (CharacterMesh& mesh : char_meshs) {
		for (RasterizedCharacter &raster : mesh.raster_sizes) {

			if (!atlas.addBitmap(raster.bitmap, raster.zone)) {
				return ErrStack(code_location, "failed to find free zone in character atlas");
			}
		}
	}

	return ErrStack();
}

CharacterMesh* TextStuff::findCharacter(std::string font_family, std::string font_style, uint32_t unicode)
{
	for (CharacterMesh& mesh : char_meshs) {
		if (mesh.font->font_family == font_family && mesh.font->font_style == font_style &&
			mesh.unicode == unicode) 
		{
			return &mesh;
		}
	}

	return nullptr;
}

ErrStack TextStuff::addInstances(ui::CharSeq& char_seq, uint32_t screen_width, uint32_t screen_height)
{
	glm::vec2 pen_pos = char_seq.pos;

	for (ui::Character& ui_ch : char_seq.characters) {

		CharacterMesh* mesh = findCharacter(ui_ch.font_family, ui_ch.font_style, ui_ch.unicode);
		if (mesh == nullptr) {
			return ErrStack(code_location, "failed to find character");
		}
		RasterizedCharacter* raster = mesh->findBestFitRaster(ui_ch.size);
	
		float scale_unit = (float)ui_ch.size / raster->raster_size;

		CharacterInstance& inst = mesh->instances.emplace_back();
		inst.screen_pos.x = pen_pos.x;
		inst.screen_pos.y = pen_pos.y - ((raster->height - raster->baseline_height) * scale_unit) / screen_height;
		inst.scale = (raster->height * scale_unit) / screen_height;
		inst.zone = raster->zone;

		// update pen pos
		pen_pos.x += (raster->advance * scale_unit) / screen_height;
	}

	return ErrStack();
}
