
// FreeType font rasterization
#include "ft2build.h"
#include FT_FREETYPE_H

// Header
#include "UIComponents.h"


CharRaster* FontSize::findCharUnicode(uint32_t unicode)
{
	for (CharRaster& char_raster : rasters) {
		if (char_raster.unicode == unicode) {
			return &char_raster;
		}
	}
	return nullptr;
}

ErrStack UserInterface::addFont(std::vector<uint8_t>& font_ttf, FontInfo& info)
{
	ErrStack err_stack;

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

	// Font
	Font& font = this->fonts.emplace_back();
	{
		font.font_family = info.family_name;
		font.font_style = info.style_name;
		font.font_sizes.resize(info.sizes_px.size());

		// Size Specific
		for (uint32_t i = 0; i < info.sizes_px.size(); i++) {
				
			FontSize& font_size = font.font_sizes[i];
			font_size.raster_size = info.sizes_px[i];
			{
				ft_err = FT_Set_Pixel_Sizes(face, 0, info.sizes_px[i]);
				if (ft_err) {
					return ErrStack(code_location, "failed to set font size");
				}

				font_size.height = face->size->metrics.height / 64;
				font_size.ascender = face->size->metrics.ascender / 64;
				font_size.descender = face->size->metrics.descender / 64;
			}
			font_size.rasters.resize(95);
		}
	}

	auto renderAndStoreGlyph = [&](uint32_t	unicode, CharRaster& char_raster) {

		uint32_t glyph_idx = FT_Get_Char_Index(face, unicode);
		if (!glyph_idx) {
			return ErrStack(code_location, "character not found in font file");
		}

		FT_Error ft_err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
		if (ft_err) {
			return ErrStack(code_location, "failed to load glyph");
		}

		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			ft_err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (ft_err) {
				return ErrStack(code_location, "failed to render glyph");
			}
		}

		char_raster.unicode = unicode;

		// Metrics
		char_raster.width = face->glyph->metrics.width / 64.0f;
		char_raster.height = face->glyph->metrics.height / 64.0f;
		char_raster.baseline_height = face->glyph->metrics.horiBearingY / 64.0f;
		char_raster.advance = face->glyph->advance.x / 64.0f;

		// Copy rendered pixels to character
		FT_Bitmap& ft_bitmap = face->glyph->bitmap;
		BasicBitmap& bitmap = char_raster.bitmap;

		bitmap.width = ft_bitmap.width;
		bitmap.height = ft_bitmap.rows;
		bitmap.colors.resize(ft_bitmap.width * ft_bitmap.rows);
		bitmap.channels = 1;

		std::memcpy(bitmap.colors.data(), ft_bitmap.buffer, bitmap.colors.size());

		return ErrStack();
	};

	// For each font size
	for (uint32_t i = 0; i < font.font_sizes.size(); i++) {

		FontSize& font_size = font.font_sizes[i];

		ft_err = FT_Set_Pixel_Sizes(face, 0, info.sizes_px[i]);
		if (ft_err) {
			return ErrStack(code_location, "failed to set font size");
		}

		// space character
		checkErrStack1(renderAndStoreGlyph(' ', font_size.rasters[0]));
		uint32_t idx = 1;

		// the rest of visible chars
		for (uint32_t unicode = '!'; unicode <= '~'; unicode++) {
			checkErrStack1(renderAndStoreGlyph(unicode, font_size.rasters[idx++]));
		}
	}

	// For each char mesh
	font.meshes.resize(95);
	FontSize& last_size = font.font_sizes.back();

	for (uint32_t i = 0; i < 95; i++) {

		CharRaster& char_raster = last_size.rasters[i];
		CharMesh& char_mesh = font.meshes[i];
		char_mesh.unicode = char_raster.unicode;
			
		float width = (float)char_raster.bitmap.width;
		float height = (float)char_raster.bitmap.height;
		width = width / height;

		// Same as UV
		char_mesh.verts[0] = { 0, 1 };
		char_mesh.verts[1] = { width, 1 };
		char_mesh.verts[2] = { width, 0 };
		char_mesh.verts[3] = { 0, 0 };
	}

	return ErrStack();
}

ErrStack UserInterface::rebindToAtlas(uint32_t atlas_size)
{
	// Find max bitmap Character size
	float max_char_width = 0;
	float max_char_height = 0;
	{
		for (Font& font : fonts) {

			FontSize& font_size = font.font_sizes.back();

			for (CharRaster& raster : font_size.rasters) {

				uint32_t new_width = raster.bitmap.width;
				uint32_t new_height = raster.bitmap.height;

				max_char_width = new_width > max_char_width ? new_width : max_char_width;
				max_char_height = new_height > max_char_height ? new_height : max_char_height;
			}
		}
	}
	atlas.recreate(atlas_size, 1, (uint32_t)max_char_width, (uint32_t)max_char_height);

	for (Font& font : fonts) {
		for (FontSize& font_size : font.font_sizes) {
			for (CharRaster& raster : font_size.rasters) {

				if (!atlas.addBitmap(raster.bitmap, raster.zone)) {
					return ErrStack(code_location, "failed to find free zone in character atlas");
				}
			}
		}
	}

	return ErrStack();
}

FontSize* UserInterface::findBestFitFontSize(std::string font_family, std::string font_style, float font_size)
{
	auto deltaU32 = [](uint32_t a, uint32_t b) -> float {
		return a > b ? (float)a - b : (float)b - a;
	};

	for (Font& font : fonts) {
		if (font.font_family == font_family && font.font_style == font_style) {

			FontSize* found_size = &font.font_sizes.front();
			float found_delta = std::abs((float)found_size->raster_size - font_size);

			for (FontSize& font_size : font.font_sizes) {

				float new_delta = deltaU32(font_size.raster_size, found_size->raster_size);
				if (new_delta < found_delta) {

					found_size = &font_size;
					found_delta = new_delta;
				}
			}

			return found_size;
		}
	}
	return nullptr;
}
