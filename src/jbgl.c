#include <jbgl.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <limits.h>

// ------------------------
// --- Helper Functions ---
// ------------------------

static void jbgl_configure_texture(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

uint32_t utf8_next(const char** s)
{
	const unsigned char* p = (const unsigned char*)*s;
	uint32_t cp;

	if (p[0] < 0x80)
	{
		cp = p[0];
		*s += 1;
	}
	else if ((p[0] & 0xE0) == 0xC0)
	{
		cp = ((p[0] & 0x1F) << 6) |
			(p[1] & 0x3F);
		*s += 2;
	}
	else if ((p[0] & 0xF0) == 0xE0)
	{
		cp = ((p[0] & 0x0F) << 12) |
			((p[1] & 0x3F) << 6) |
			(p[2] & 0x3F);
		*s += 3;
	}
	else
	{
		cp = ((p[0] & 0x07) << 18) |
			((p[1] & 0x3F) << 12) |
			((p[2] & 0x3F) << 6) |
			(p[3] & 0x3F);
		*s += 4;
	}

	return cp;
}

void hex_to_vec4(const char* hex, vec4 colour)
{
	unsigned int r, g, b;

	if (hex[0] == '#')
		hex++;

	sscanf(hex, "%02x%02x%02x", &r, &g, &b);

	colour[0] = r / 255.0f;
	colour[1] = g / 255.0f;
	colour[2] = b / 255.0f;
	colour[3] = 1.0f;
}




// -----------------------

JbglState* jbgl_init(int screen_w, int screen_h)
{
	JbglState* state = malloc(sizeof(*state));
	if (!state)
		return NULL;

	state->screen_w = screen_w;
	state->screen_h = screen_h;

	state->instance_count = 0;
	state->tex_count = 0;


	if (FT_Init_FreeType(&state->ft))
	{
		printf("ERROR::FREETYPE: Could not init FreeType Library\n");
		return NULL;
	}
#
	jbgl_init_batch_renderer(state);

	return state;

}

int jbgl_gladLoadGL(GLADloadfunc load)
{
	return gladLoadGL(load);
}


// ------------------------
// --- Batch Rendering ---
// ------------------------


void jbgl_begin_batch(JbglState* state)
{
	// init variables
	state->instance_count = 0;
	state->tex_count = 1;
	state->tex_index = 1;
}

void jbgl_end_batch(JbglState* state)
{
	// flush renderer
	jbgl_batch_renderer_flush(state);

}

JbglRectInstance* jbgl_add_rect_instance(JbglState* state, JbglRectangle rect, uint8_t tex_index)
{
	// If batch has reached capacity then flush
	if (state->instance_count + 1 >= JBGL_MAX_BATCH_INSTANCES)
	{
		jbgl_batch_renderer_flush(state);
		state->instance_count = 0;
	}

	JbglRectInstance* instance = &state->instances[state->instance_count++];

	instance->size[0] = rect.w;
	instance->size[1] = rect.h;

	instance->pos[0] = rect.centre[0];
	instance->pos[1] = rect.centre[1];
	instance->pos[2] = JBGL_2D_DEPTH;

	instance->uv0[0] = 0.0f;
	instance->uv0[1] = 0.0f;

	instance->uv1[0] = 1.0f;
	instance->uv1[1] = 1.0f;

	instance->tex_index = tex_index;
	
	return instance;
}

void jbgl_init_batch_renderer(JbglState* state)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	state->instance_count = 0;
	state->instances = calloc(JBGL_MAX_BATCH_INSTANCES, sizeof(JbglRectInstance));

	glGenVertexArrays(1, &state->vao_id);
	glBindVertexArray(state->vao_id);


	// Send identity quad

	JbglVertex identity_quad_vertices[4] = {
	  {{0.0f, 0.0f, JBGL_2D_DEPTH}, {0.0f, 0.0f}},
	  {{1.0f, 0.0f, JBGL_2D_DEPTH}, {1.0f, 0.0f}},
	  {{1.0f, 1.0f, JBGL_2D_DEPTH}, {1.0f, 1.0f}},
	  {{0.0f, 1.0f, JBGL_2D_DEPTH}, {0.0f, 1.0f}},
	};
	uint32_t identity_quad_indices[6] = { 0, 1, 2, 2, 3, 0 };

	glGenBuffers(1, &state->identity_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, state->identity_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(identity_quad_vertices), identity_quad_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &state->identity_ebo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->identity_ebo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(identity_quad_indices), identity_quad_indices, GL_STATIC_DRAW);

	// - Vert Pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(JbglVertex), (void*)offsetof(JbglVertex, pos));
	glEnableVertexAttribArray(0);
	// - Texcoord 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(JbglVertex), (void*)offsetof(JbglVertex, tex_coords));
	glEnableVertexAttribArray(1);

	// Send Dynamic Attributes

	glGenBuffers(1, &state->vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(JbglRectInstance) * JBGL_MAX_BATCH_INSTANCES, NULL, GL_DYNAMIC_DRAW);

	// - WorldPos
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(JbglRectInstance), (void*)offsetof(JbglRectInstance, pos));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	// - Size 
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(JbglRectInstance), (void*)offsetof(JbglRectInstance, size));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	// - Colour 
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(JbglRectInstance), (void*)offsetof(JbglRectInstance, colour));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);
	// - Tex Index
	glVertexAttribPointer(5, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(JbglRectInstance), (void*)offsetof(JbglRectInstance, tex_index));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);
	// UV0 (texture atlasing fonts)
	glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(JbglRectInstance),(void*)offsetof(JbglRectInstance, uv0));
	glEnableVertexAttribArray(6);
	glVertexAttribDivisor(6, 1);
	// UV1
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE,sizeof(JbglRectInstance),(void*)offsetof(JbglRectInstance, uv1));
	glEnableVertexAttribArray(7);
	glVertexAttribDivisor(7, 1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Create texture slots
	int tex_slots[JBGL_MAX_BATCH_TEXTURES];
	for (int i = 0; i < JBGL_MAX_BATCH_TEXTURES; i++)
	{
		tex_slots[i] = i;
	}

	// Shader Setup
	state->shader = jbgl_init_shader_from_file("resources/shaders/vsDefaultBatch.glsl", "resources/shaders/fsDefaultBatch.glsl");
	glUseProgram(state->shader.id);
	glBindVertexArray(state->vao_id);
	jbgl_batch_set_proj_mat(state);
	glUniform1iv(glGetUniformLocation(state->shader.id, "textures"), JBGL_MAX_BATCH_TEXTURES, tex_slots);

}

void jbgl_batch_renderer_flush(JbglState* state)
{

	if (state->instance_count <= 0) return;
	glBindVertexArray(state->vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(JbglRectInstance) * state->instance_count, state->instances);
	
	// Bind Textures
	for (int i = 0; i < state->tex_count; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, state->textures[i].id);
	}
	//
	glUseProgram(state->shader.id);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, state->instance_count);
	glBindVertexArray(0);

}

int jbgl_batch_add_tex(JbglState* state, const JbglTexture tex)
{
	int index = jbgl_find_texture(state, tex);
	if (index != -1)
		return index;

	if (state->tex_count >= JBGL_MAX_BATCH_TEXTURES)
	{
		jbgl_batch_renderer_flush(state);

		state->instance_count = 0;
		state->tex_count = 1;
		state->tex_index = 1;
	}

	state->textures[state->tex_index] = tex;

	int result = state->tex_count;

	state->tex_index++;
	state->tex_count++;

	return result;
}

void jbgl_batch_set_proj_mat(JbglState* state)
{
	mat4 proj = GLM_MAT4_IDENTITY_INIT;
	glm_ortho(0.0f, (float)state->screen_w, (float)state->screen_h, 0.0f, -1.0f, 1.0f,proj);

	jbgl_shader_set_mat4(state->shader, "proj", proj);
}

void jbgl_batch_cleanup(JbglState* state)
{
	// Cleanup batch renderer
	glDeleteVertexArrays(1, &state->vao_id);
	glDeleteBuffers(1, &state->identity_vbo_id);
	glDeleteBuffers(1, &state->identity_ebo_id);
	glDeleteBuffers(1, &state->vbo_id);
	glDeleteProgram(state->shader.id);
	FT_Done_FreeType(state->ft);
	free(state->instances);
	free(state);
}

// ----------------
// --- Textures ---
// ----------------

JbglTexture jbgl_load_texture(const char* filepath)
{
	JbglTexture tex = { 0 };
	int width, height, nr_channels;
	//stbi_set_flip_vertically_on_load(1);
	unsigned char* image = stbi_load(filepath, &width, &height, &nr_channels, 0);

	if (!image)
	{
		printf("Failed to load texture");
		return tex;
	}

	GLenum format;

	if (nr_channels == 1)
		format = GL_RED;
	else if (nr_channels == 3)
		format = GL_RGB;
	else if (nr_channels == 4)
		format = GL_RGBA;
	else
	{
		printf("Unexpected channels: %d\n", nr_channels);
		stbi_image_free(image);
		return tex;
	}

	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	jbgl_configure_texture();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image);

	tex.width = width;
	tex.height = height;
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

int jbgl_find_texture(JbglState* state, JbglTexture tex)
{
	for (int i = 1; i < state->tex_count; i++)
	{
		if (state->textures[i].id == tex.id)
			return i;
	}

	return -1;
}

void jbgl_draw_rect(JbglState* state, vec3 pos, int w, int h, vec4 colour)
{
	JbglRectangle rect;

	rect.centre[0] = pos[0];
	rect.centre[1] = pos[1];
	rect.w = w;
	rect.h = h;

	// 0 = no texture
	JbglRectInstance* instance = jbgl_add_rect_instance(state, rect, 0);

	glm_vec4_copy(colour, instance->colour);

	instance->uv0[0] = 0.0f;
	instance->uv0[1] = 0.0f;

	instance->uv1[0] = 1.0f;
	instance->uv1[1] = 1.0f;
}

void jbgl_draw_texture(JbglState* state, JbglTexture tex, vec3 pos, int w, int h, vec4 col)
{
	int tex_index = jbgl_batch_add_tex(state, tex);

	JbglRectangle rect;
	rect.centre[0] = pos[0];
	rect.centre[1] = pos[1];

	rect.w = w;
	rect.h = h;
	
	JbglRectInstance* instance = jbgl_add_rect_instance(state, rect, tex_index);
	glm_vec4_copy(col, instance->colour);
}

void jbgl_destroy_texture(JbglTexture* tex)
{
	if (tex->id != 0)
	{
		glDeleteTextures(1, &tex->id);
		tex->id = 0;
	}
}

// ------------------------
// --- Fonts and Glyphs ---
// ------------------------

JbglFont* jbgl_load_font(JbglState* state, const char* filepath, int size)
{
	JbglFont* font = malloc(sizeof(JbglFont));

	if (FT_New_Face(state->ft, filepath, 0, &font->face))
	{
		printf("ERROR::FREETYPE: Failed to load font\n");
		free(font);
		return;
	}

	// todo sort this out
	if (FT_Set_Charmap(font->face, FT_ENCODING_UNICODE))
	{
		for (int i = 0; i < font->face->num_charmaps; i++) {
			if (font->face->charmaps[i]->encoding == FT_ENCODING_UNICODE) {
				FT_Set_Charmap(font->face, font->face->charmaps[i]);
				break;
			}
		}
	}

	font->size = size;

	// Configure font sizing
	if (font->face->num_fixed_sizes > 0)
	{
		int best_match = 0;

		int best_diff = abs((int)font->face->available_sizes[0].height - size);

		for (int i = 1; i < font->face->num_fixed_sizes; i++)
		{
			int diff = abs((int)font->face->available_sizes[i].height - size);

			if (diff < best_diff)
			{
				best_match = i;
				best_diff = diff;
			}
		}

		if (FT_Select_Size(font->face, best_match))
		{
			printf("ERROR::FREETYPE: Failed to select bitmap strike\n");

			FT_Done_Face(font->face);
			free(font);

			return NULL;
		}

		font->selected_strike_size = font->face->available_sizes[best_match].height;
	}
	else
	{
		if (FT_Set_Pixel_Sizes(font->face, 0, size))
		{
			printf("ERROR::FREETYPE: Failed to set pixel size\n");

			FT_Done_Face(font->face);
			free(font);

			return NULL;
		}

		font->selected_strike_size = 0;
	}

	// Configure font
	font->atlas_w = 1024;
	font->atlas_h = 1024;
	font->atlas_row_h = 0;
	font->atlas_x = 0;
	font->atlas_y = 0;
	font->glyph_cache.capacity = 4;
	font->glyph_cache.count = 0;
	font->glyph_cache.cache = (JbglGlyph*)malloc(sizeof(JbglGlyph) * font->glyph_cache.capacity);



	// Create texture atlas
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &(font->atlas_id));
	glBindTexture(GL_TEXTURE_2D, font->atlas_id);
	jbgl_configure_texture();

	// Texture will get Sub-imaged in later
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font->atlas_w, font->atlas_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	return font;
}

JbglGlyph jbgl_find_glyph(JbglFont* font, int index, bool bold, bool italic)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	JbglGlyph glyph = { 0 };

	// Load glyph
	if (FT_Load_Glyph(font->face, index, FT_LOAD_RENDER))
	{
		printf("ERROR::FREETYPE: Failed to load Glyph\n");
		return glyph;
	}

	// Artificial bold
	if (bold)
	{
		FT_GlyphSlot_Embolden(font->face->glyph);
	}

	FT_GlyphSlot glyph_slot = font->face->glyph;

	int w = glyph_slot->bitmap.width;
	int h = glyph_slot->bitmap.rows;

	float shear = 0.25f;

	// Amount of extra space required by the shear
	int extra_width = italic ? (int)(h * shear) : 0;

	int final_w = w + extra_width;
	int final_h = h;

	// Allocate final bitmap
	unsigned char* texture_data = (unsigned char*)calloc(final_w * final_h, 4);

	if (!texture_data)
	{
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	// Copy the FreeType bitmap into the font's bitmap.
	// The bottom of the glyph stays where it is.
	// Higher rows move further to the right.

	for (int y = 0; y < h; y++)
	{
		float shift = 0.0f;

		if (italic)
		{
			shift = (float)(h - 1 - y) * shear;
		}

		for (int x = 0; x < w; x++)
		{
			float dest_x = x + shift;

			int x0 = (int)floorf(dest_x);
			int x1 = x0 + 1;

			float t = dest_x - (float)x0;

			unsigned char value = glyph_slot->bitmap.buffer[y * glyph_slot->bitmap.pitch + x];

			// Distribute the coverage between the two neighbouring pixels.
			if (x0 >= 0 && x0 < final_w)
			{
				unsigned char* dest = &texture_data[(y * final_w + x0) * 4];

				float v = value * (1.0f - t);

				dest[0] = (unsigned char)fminf(255.0f, dest[0] + v);
				dest[1] = (unsigned char)fminf(255.0f, dest[1] + v);
				dest[2] = (unsigned char)fminf(255.0f, dest[2] + v);
				dest[3] = (unsigned char)fminf(255.0f, dest[3] + v);
			}

			if (x1 >= 0 && x1 < final_w)
			{
				unsigned char* dest = &texture_data[(y * final_w + x1) * 4];

				float v = value * t;

				dest[0] = (unsigned char)fminf(255.0f, dest[0] + v);
				dest[1] = (unsigned char)fminf(255.0f, dest[1] + v);
				dest[2] = (unsigned char)fminf(255.0f, dest[2] + v);
				dest[3] = (unsigned char)fminf(255.0f, dest[3] + v);
			}
		}
	}

	// Atlas row change
	if (font->atlas_x + final_w > font->atlas_w)
	{
		font->atlas_x = 0;
		font->atlas_y += font->atlas_row_h;
		font->atlas_row_h = 0;
	}

	// Atlas resize
	if (font->atlas_y + final_h > font->atlas_h)
	{
		int new_w = font->atlas_w * 2;
		int new_h = font->atlas_h * 2;

		GLuint new_id;

		glBindTexture(GL_TEXTURE_2D, font->atlas_id);

		unsigned char* old_texture = (unsigned char*)malloc(font->atlas_w * font->atlas_h * 4);

		if (!old_texture)
		{
			fprintf(stderr, "Memory allocation failed\n");
			exit(EXIT_FAILURE);
		}

		glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,old_texture);

		glGenTextures(1, &new_id);
		glBindTexture(GL_TEXTURE_2D, new_id);

		jbgl_configure_texture();

		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,new_w,	new_h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

		// Copy old texture into new texture
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,font->atlas_w,font->atlas_h,GL_RGBA,GL_UNSIGNED_BYTE,old_texture);

		glDeleteTextures(1, &font->atlas_id);

		font->atlas_id = new_id;
		font->atlas_w = new_w;
		font->atlas_h = new_h;

		free(old_texture);
	}

	// Put bitmap glyph into atlas
	glBindTexture(GL_TEXTURE_2D, font->atlas_id);

	jbgl_configure_texture();

	glTexSubImage2D(GL_TEXTURE_2D,0,font->atlas_x,font->atlas_y,final_w,final_h,GL_RGBA,GL_UNSIGNED_BYTE,texture_data);

	// Glyph dimensions
	glyph.size[0] = final_w;
	glyph.size[1] = final_h;

	// Shear compensation
	glyph.bearing[0] = glyph_slot->bitmap_left;
	glyph.bearing[1] = glyph_slot->bitmap_top;

	glyph.advance = glyph_slot->advance.x / 64.0f;

	glyph.codepoint = glyph_slot->glyph_index;

	glyph.bold = bold;
	glyph.italic = italic;

	// UVs
	glyph.uv0[0] = (float)font->atlas_x / (float)font->atlas_w;
	glyph.uv0[1] = (float)font->atlas_y / (float)font->atlas_h;
	glyph.uv1[0] = (float)(font->atlas_x + final_w) / (float)font->atlas_w;
	glyph.uv1[1] = (float)(font->atlas_y + final_h) / (float)font->atlas_h;

	// Advance atlas position
	font->atlas_x += final_w + 1;

	if (final_h > font->atlas_row_h)
	{
		font->atlas_row_h = final_h;
	}
	free(texture_data);

	return glyph;
}

void jbgl_free_font(JbglFont* font)
{
	if (font->atlas_id != 0)
	{
		glDeleteTextures(1, &font->atlas_id);
	}

	FT_Done_Face(font->face);
	
	free(font->glyph_cache.cache);
	free(font);
}

void jbgl_cache_glyph(JbglState* state, JbglFont* font, JbglGlyph glyph)
{

	JbglGlyphCache* cache = &font->glyph_cache;

	if (cache->count + 1 > cache->capacity)
	{
		cache->capacity = cache->capacity * 2;
		JbglGlyph* new_cache = (JbglGlyph*)malloc(sizeof(JbglGlyph) * cache->capacity);
		memcpy(new_cache, cache->cache, cache->count * sizeof(JbglGlyph));
		free(font->glyph_cache.cache);
		font->glyph_cache.cache = new_cache;
	}

	cache->cache[cache->count] = glyph;
	cache->count++;
}

JbglGlyph jbgl_get_glyph_from_cache(JbglState* state, JbglFont* font, int codepoint, bool bold, bool italic)
{
	// Search for glyph in cache

	JbglGlyphCache* cache = &font->glyph_cache;

	for (int i = 0; i < cache->count; i++)
	{
		if (codepoint == cache->cache[i].codepoint && cache->cache[i].bold == bold && cache->cache[i].italic == italic)
		{
			return cache->cache[i];
		}
	}
	// If unpresent, generate, add to cache and return.
	JbglGlyph new_glyph = jbgl_find_glyph(font, codepoint, bold, italic);
	new_glyph.bold = bold;
	jbgl_cache_glyph(state, font, new_glyph);

	return new_glyph;
}

// ------------------------
// --- Text Rendering ---
// ------------------------


JbglTextInfo jbgl_draw_text(JbglState* state, const char* text, vec2 pos, JbglFont* font, bool wrap, vec4 col, bool bold, bool italic, bool underline, bool strikethrough)
{
	vec2 draw_pos;
	draw_pos[0] = pos[0];
	draw_pos[1] = pos[1];
	float line_height = font->face->size->metrics.height / 64.0f;

	const char* p = text;
	// Loop through string
	while (*p)
	{
		uint32_t codepoint = utf8_next(&p);

		if (codepoint == '\n')
		{
				draw_pos[0] = pos[0];
				draw_pos[1] += line_height;
				continue;
		}

		if (codepoint == '\t')
		{
			// TODO
				continue;
		}

		FT_UInt glyph_index = FT_Get_Char_Index(font->face, codepoint);

		if (!glyph_index)
			continue;

		JbglGlyph glyph = jbgl_get_glyph_from_cache(state, font, glyph_index, bold, italic);


		// Wrapping
		if (wrap && draw_pos[0] + glyph.advance > state->screen_w)
		{
			draw_pos[0] = pos[0];
			draw_pos[1] += line_height;
		}

		vec2 render_pos = {
			draw_pos[0] + glyph.bearing[0],
			draw_pos[1] - glyph.bearing[1]
		};

		jbgl_render_glyph(state, font, &glyph, render_pos, col);

		if (underline)
		{
			float underline_y = draw_pos[1] + 3.0f;

			float underline_thickness = 1.5f;

			jbgl_draw_rect(state,(vec3) {draw_pos[0], underline_y, JBGL_2D_DEPTH}, (int)glyph.advance, (int)underline_thickness, col);
		}

		if (strikethrough)
		{
			float strikeline_y = draw_pos[1] - 3.0f + (font->face->size->metrics.descender) / 64.0f;

			float strikeline_thickness = 3.0f;

			jbgl_draw_rect(state,(vec3) {draw_pos[0],strikeline_y,JBGL_2D_DEPTH},(int)glyph.advance,(int)strikeline_thickness,col);
		}


		draw_pos[0] += glyph.advance;
	
	}
	JbglTextInfo info;
	info.pos[0] = draw_pos[0];
	info.pos[1] = draw_pos[1];

	return info;
}

JbglTextInfo jbgl_draw_text_animated(JbglState* state, const char* text, vec2 pos, JbglFont* font, bool wrap, vec4 col, bool bold, bool italic, bool underline, bool strikethrough, float time)
{
	vec2 draw_pos;
	draw_pos[0] = pos[0];
	draw_pos[1] = pos[1];
	float line_height = font->face->size->metrics.height / 64.0f;

	const char* p = text;
	int counter = 0;

	while (*p)
	{
		counter++;
		uint32_t codepoint = utf8_next(&p);

		if (codepoint == '\n')
		{
			draw_pos[1] += line_height;
			continue;
		}

		if (codepoint == '\t')
		{

			continue;
		}

		FT_UInt glyph_index = FT_Get_Char_Index(font->face, codepoint);

		if (!glyph_index)
			continue;

		JbglGlyph glyph = jbgl_get_glyph_from_cache(state, font, glyph_index, bold, italic);



		if (wrap && draw_pos[0] + glyph.advance > state->screen_w)
		{
			draw_pos[1] += line_height;
		}

		vec2 render_pos = {
			draw_pos[0] + glyph.bearing[0],
			draw_pos[1] - glyph.bearing[1] + sinf(time + counter)*5
		};

		jbgl_render_glyph(state, font, &glyph, render_pos, col);

		if (underline)
		{
			float underline_y = draw_pos[1] + 3.0f + sinf(time + counter) * 5;

			float underline_thickness = 1.5f;

			jbgl_draw_rect(state, (vec3) { draw_pos[0], underline_y, JBGL_2D_DEPTH }, (int)glyph.advance, (int)underline_thickness, col);
		}

		if (strikethrough)
		{
			float strikeline_y = draw_pos[1] - 3.0f + (font->face->size->metrics.descender) / 64.0f + sinf(time + counter) * 5;

			float strikeline_thickness = 3.0f;

			jbgl_draw_rect(state, (vec3) { draw_pos[0], strikeline_y, JBGL_2D_DEPTH }, (int)glyph.advance, (int)strikeline_thickness, col);
		}


		draw_pos[0] += glyph.advance;




	}
	JbglTextInfo info;
	info.pos[0] = draw_pos[0];
	info.pos[1] = draw_pos[1];

	return info;
}

void jbgl_render_glyph(JbglState* state, JbglFont* font, JbglGlyph* glyph, vec2 pos, vec4 col)
{
	vec2 tex_coords[4] = {
			{glyph->uv0[0], glyph->uv0[1]}, // Bottom-left
			{glyph->uv1[0], glyph->uv0[1]}, // Bottom-right
			{glyph->uv1[0], glyph->uv1[1]}, // top-right
			{glyph->uv0[0], glyph->uv1[1]} // top-left
	};

	JbglTexture tex = (JbglTexture){
		.id = font->atlas_id,
	 .width = glyph->size[0],
	 .height = glyph->size[1]
	};

	JbglRectangle rect;
	rect.w = tex.width;
	rect.h = tex.height;
	rect.centre[0] = pos[0];
	rect.centre[1] = pos[1];

	vec3 pos3;
	pos3[0] = pos[0];
	pos3[1] = pos[1];
	pos3[2] = JBGL_2D_DEPTH;

	int tex_index = jbgl_batch_add_tex(state, tex);
	JbglRectInstance* instance = jbgl_add_rect_instance(state, rect, tex_index);

	glm_vec4_copy(col, instance->colour);
	glm_vec2_copy(glyph->uv0, instance->uv0);
	glm_vec2_copy(glyph->uv1, instance->uv1);
}

void jbgl_measure_text(JbglState* state, JbglFont* font, const char* text, vec2 start_pos, vec2 dest, bool wrap)
{
	jbgl_measure_text_range(state, font, text, 0, INT_MAX, start_pos, dest, wrap);
}

void jbgl_measure_text_range(JbglState* state, JbglFont* font, const char* text, int start, int end, vec2 start_pos, vec2 dest, bool wrap)
{
	vec2 pos = { start_pos[0], start_pos[1] };
	float line_height = font->face->size->metrics.height / 64.0f;

	float max_x = start_pos[0];

	const char* p = text;
	int character = 0;

	while (*p && character < end)
	{
		uint32_t codepoint = utf8_next(&p);

		if (character >= start)
		{
			if (codepoint == '\n')
			{
				if (pos[0] > max_x)
					max_x = pos[0];

				pos[0] = start_pos[0];
				pos[1] += line_height;
				character++;
				continue;
			}

			FT_UInt glyph_index = FT_Get_Char_Index(font->face, codepoint);

			if (glyph_index)
			{
				JbglGlyph glyph = jbgl_get_glyph_from_cache(state, font, glyph_index, false, false);

				if (wrap && pos[0] + glyph.advance > state->screen_w)
				{
					if (pos[0] > max_x)
						max_x = pos[0];

					pos[0] = start_pos[0];
					pos[1] += line_height;
				}

				pos[0] += glyph.advance;
			}
		}

		character++;
	}

	if (pos[0] > max_x)
		max_x = pos[0];

	dest[0] = max_x - start_pos[0];
	dest[1] = (pos[1] - start_pos[1]) + line_height;
}


// ------------------------
// --- Shaders ---
// ------------------------

JbglShader jbgl_init_shader(const char* vs_source, const char* fs_source)
{
	GLuint vs_id, fs_id, program_id;

	vs_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs_id, 1, &vs_source, NULL);
	glCompileShader(vs_id);
	fs_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs_id, 1, &fs_source, NULL);
	glCompileShader(fs_id);

	program_id = glCreateProgram();
	glAttachShader(program_id, vs_id);
	glAttachShader(program_id, fs_id);
	glLinkProgram(program_id);



	int success;
	char infoLog[512];

	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vs_id, 512, NULL, infoLog);
		printf("Vertex shader error:\n%s\n", infoLog);
	}

	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fs_id, 512, NULL, infoLog);
		printf("Fragment shader error:\n%s\n", infoLog);
	}

	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program_id, 512, NULL, infoLog);
		printf("Shader link error:\n%s\n", infoLog);
	}

	glDeleteShader(vs_id);
	glDeleteShader(fs_id);

	JbglShader dest;
	dest.id = program_id;

	return dest;
}

JbglShader jbgl_init_shader_from_file(const char* vs_filepath, const char* fs_filepath)
{
	// Read Vertex shader
	FILE* fptr;
	fptr = fopen(vs_filepath, "rb");

	if (fptr == NULL)
	{
		printf("vertex shader failed to open");
		exit(EXIT_FAILURE);
	}

	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	rewind(fptr);

	char* vs_source = (char*)malloc(length + 1);
	fread(vs_source, 1, length, fptr);
	vs_source[length] = '\0';
	fclose(fptr);

	if ((unsigned char)vs_source[0] == 0xEF && (unsigned char)vs_source[1] == 0xBB && (unsigned char)vs_source[2] == 0xBF)
	{
		memmove(vs_source, vs_source + 3, length - 2);
		vs_source[length - 3] = '\0';
	}

	// Read fragment shader
	fptr = fopen(fs_filepath, "rb");

	if (fptr == NULL)
	{
		printf("fragment shader failed to open");
		exit(EXIT_FAILURE);
	}

	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	rewind(fptr);

	char* fs_source = (char*)malloc(length + 1);
	fread(fs_source, 1, length, fptr);
	fs_source[length] = '\0';
	fclose(fptr);

	if ((unsigned char)fs_source[0] == 0xEF && (unsigned char)fs_source[1] == 0xBB && (unsigned char)fs_source[2] == 0xBF)
	{
		memmove(fs_source, fs_source + 3, length - 2);
		fs_source[length - 3] = '\0';
	}


	JbglShader dest = jbgl_init_shader(vs_source, fs_source);

	free(vs_source);
	free(fs_source);


	return dest;

}

void jbgl_shader_set_int(JbglShader shader, const char* name, int value)
{
	//glUseProgram(shader.id);
	glUniform1i(glGetUniformLocation(shader.id, name), value);
}

void jbgl_shader_set_float(JbglShader shader, const char* name, float value)
{
	//glUseProgram(shader.id);
	glUniform1f(glGetUniformLocation(shader.id, name), value);
}

void jbgl_shader_set_mat4(JbglShader shader, const char* name, mat4 value)
{
	//glUseProgram(shader.id);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, name), 1, GL_FALSE, &value[0][0]);
}










