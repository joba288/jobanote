#pragma once
#include <glad/gl.h>
#include <cglm/cglm.h>

#include <ft2build.h>
#include FT_FREETYPE_H  


// --- Macros ---
#define JBGL_2D_DEPTH 0
#define JBGL_MAX_BATCH_INSTANCES 200
#define JBGL_MAX_BATCH_TEXTURES 32

#ifdef __cplusplus
extern "C" 
{
#endif

	// --- Types ---

	/** 
	* @struct JbglVertex 
	* @brief Represents 3D position and texture coordinates associated with a vertex. */
	typedef struct JbglVertex
	{
		vec3 pos;
		vec2 tex_coords;
	} JbglVertex;

	/**
	 * @struct JbglTexture
	 * @brief Represents an OpenGL texture.
	 *
	 * Stores the OpenGL texture ID along with the dimensions of
	 * the texture.*/
	typedef struct JbglTexture
	{
		GLuint id;
		int width;
		int height;

	}JbglTexture;

	/** 
	* @struct JbglShader 
	* @brief Represents an OpenGL shader program.*/
	typedef struct JbglShader 
	{
		GLuint id;
	} JbglShader;

	/** 
	* @struct JbglRectInstance 
	* @brief Represents a rectangle queued for batch rendering. 
	* Stores the position, dimensions, colour, texture and texture 
	* coordinates required to render a rectangle. */
	typedef struct JbglRectInstance
	{
		vec3 pos;
		vec2 size;
		vec4 colour;
		int tex_index;
		vec2 uv0;
		vec2 uv1;
	} JbglRectInstance;

	/** 
	* @struct JbglRectangle 
	* @brief Represents the dimensions and centre position of a rectangle. */
	typedef struct JbglRectangle
	{
		float w, h;
		vec2 centre;
	} JbglRectangle;


	/** 
	* @struct JbglGlyph 
	* @brief Represents a single font glyph. 
	* Stores the glyph's dimensions, positioning information and 
	* location within the font texture atlas. */
	typedef struct JbglGlyph
	{
		vec2 size; // size of glyph
		vec2 bearing; // offset from baseline to left
		GLuint advance; // offset advance of next glyph
		int codepoint;

		vec2 uv0; // UV location of place in atlas (top left)
		vec2 uv1; // UV location of place in atlas (bottom right)
		bool bold;
		bool italic;
	} JbglGlyph;

	/**
	* @struct JbglGlyphCache 
	* @brief Stores cached glyphs for a font. 
	* The glyph cache stores glyph information that has already 
	* been generated, avoiding the need to repeatedly process 
	* the same glyph. */
	typedef struct JbglGlyphCache
	{
		JbglGlyph* cache;
		GLuint count; // Number of glyphs already stored in cache
		int capacity; // Maximum number of glyphs
	} JbglGlyphCache;

	/** 
	* @struct JbglFont 
	* @brief Represents a loaded font and its texture atlas. 
	* Stores the FreeType face, font atlas information and cached 
	* glyphs used when rendering text. */
	typedef struct JbglFont
	{
		GLuint atlas_id;
		int atlas_w;
		int atlas_h;

		int atlas_x;
		int atlas_y;

		int atlas_row_h;

		FT_Face face;

		JbglGlyphCache glyph_cache;

		int size;
		int selected_strike_size;

	} JbglFont;

	/** 
	* @struct JbglTextInfo 
	* @brief Stores information about rendered text. 
	* Contains the resulting position after text rendering, allowing 
	* callers to determine where subsequent text can be placed. */
	typedef struct JbglTextInfo
	{
		vec2 pos;
	} JbglTextInfo;

	/** 
	* @struct JbglState 
	* @brief Stores the state of the jbgl renderer.
	* Contains the OpenGL objects, batch rendering data, textures, 
	* viewport dimensions and FreeType library used by jbgl. */
	typedef struct JbglState
	{
		GLuint vao_id, vbo_id;
		GLuint identity_vbo_id, identity_ebo_id;

		JbglShader shader;

		JbglRectInstance* instances;
		int instance_count;

		JbglTexture textures[JBGL_MAX_BATCH_TEXTURES];
		int tex_index;
		int tex_count;

		int screen_w;
		int screen_h;

		FT_Library ft;

		
	} JbglState;

	// --------------------
	// -- Initialisation  --
	// --------------------

	/** 
	* @brief Initialise the jbgl library. 
	* Creates the rendering state and initialises the resources 
	* required by jbgl. 
	* 
	* @param screen_w Width of the rendering viewport. 
	* @param screen_h Height of the rendering viewport. 
	* @return A pointer to the initialised JbglState, or NULL if * initialisation fails. */ 
	JbglState* jbgl_init(int screen_w, int screen_h);


	/** * @brief Load OpenGL function pointers. 
	* Uses the supplied function to retrieve OpenGL function * addresses required by jbgl. 
	*
	* @param load OpenGL function loader. 
	* @return Non-zero on success, zero on failure. */ 
	int jbgl_gladLoadGL(GLADloadfunc load);


	// --------------------
	// -- Batch Renderer --
	// --------------------

	/** * @brief Begin a batch rendering operation. * @param state The jbgl rendering state. */ 
	void jbgl_begin_batch(JbglState* state);

	/** * @brief End the current batch rendering operation and flush queued instances. * @param state The jbgl rendering state. */ 
	void jbgl_end_batch(JbglState* state);

	/** * @brief Initialise the OpenGL resources used by the batch renderer. * @param state The jbgl rendering state. */ 
	void jbgl_init_batch_renderer(JbglState* state);

	/** * @brief Add a rectangle instance to the current batch. * @param state The jbgl rendering state. * @param rect Rectangle dimensions and position. * @param tex_index Index of the texture used by the rectangle. * @return A pointer to the newly added rectangle instance. */ 
	JbglRectInstance* jbgl_add_rect_instance(JbglState* state, JbglRectangle rect, uint8_t tex_index);

	/** * @brief Flush all queued rectangle instances to OpenGL. * @param state The jbgl rendering state. */ 
	void jbgl_batch_renderer_flush(JbglState* state);

	/** * @brief Add a texture to the current batch if it is not already present. * @param state The jbgl rendering state. * @param tex Texture to add to the batch. * @return The texture slot index assigned to the texture. */
	int jbgl_batch_add_tex(JbglState* state, const JbglTexture tex);

	/** * @brief Find the batch texture slot containing a texture. * @param state The jbgl rendering state. * @param tex Texture to search for. * @return The texture slot index if found, otherwise -1. */ 
	int jbgl_find_texture(JbglState* state, JbglTexture tex);

	/** * @brief Set the orthographic projection matrix used by the batch renderer. * @param state The jbgl rendering state. */ 
	void jbgl_batch_set_proj_mat(JbglState* state);

	/** * @brief Add a coloured rectangle to the current batch. * @param state The jbgl rendering state. * @param pos Position of the rectangle. * @param w Width of the rectangle. * @param h Height of the rectangle. * @param colour RGBA colour of the rectangle. */ 
	void jbgl_draw_rect(JbglState* state, vec3 pos, int w, int h, vec4 colour);

	/** * @brief Add a textured rectangle to the current batch. * @param state The jbgl rendering state. * @param tex Texture to render. * @param pos Position of the rectangle. * @param w Width of the rectangle. * @param h Height of the rectangle. * @param col RGBA colour tint applied to the texture. */ 
	void jbgl_draw_texture(JbglState* state, JbglTexture tex, vec3 pos, int w, int h, vec4 col);

	
	// -- Textures --

	/** * @brief Load an image file into an OpenGL 2D texture. * @param filepath Path to the image file. * @return The loaded texture, or a texture with id 0 if loading fails. */ 
	JbglTexture jbgl_load_texture(const char* filepath);

	/** * @brief Delete an OpenGL texture and reset its texture ID to zero. * @param tex Texture to destroy. */ 
	void jbgl_destroy_texture(JbglTexture* tex);

	// -- Fonts --

	/** * @brief Load a font using FreeType and create a texture atlas for its glyphs. * @param state The jbgl rendering state containing the FreeType library. * @param filepath Path to the font file. * @param size Requested font size in pixels. * @return A pointer to the loaded font, or NULL if loading fails. */ 
	JbglFont* jbgl_load_font(JbglState* state, const char* filepath, int size);

	/** * @brief Find and generate a glyph for a font, including optional artificial bold and italic styling. * @param font Font containing the glyph. * @param index FreeType glyph index to load. * @param bold Whether to apply artificial bold styling. * @param italic Whether to apply italic shear to the glyph bitmap. * @return The generated glyph, or a zero-initialised glyph if loading fails. */ 
	JbglGlyph jbgl_find_glyph(JbglFont* font, int index, bool bold, bool italic);

	/** * @brief Add a glyph to a font's glyph cache. * @param state The jbgl rendering state. * @param font The font whose cache will be updated. * @param glyph Glyph to add to the cache. */ 
	void jbgl_cache_glyph(JbglState* state, JbglFont* font, JbglGlyph glyph);

	/** * @brief Retrieve a glyph from the cache or generate and cache it if it is not present. * @param state The jbgl rendering state. * @param font Font whose glyph cache will be searched. * @param codepoint FreeType glyph index used to identify the glyph. * @param bold Whether the glyph uses artificial bold styling. * @param italic Whether the glyph uses italic styling. * @return The matching cached or newly generated glyph. */ 
	JbglGlyph jbgl_get_glyph_from_cache(JbglState* state, JbglFont* font, int codepoint, bool bold, bool italic);

	/** * @brief Add a glyph from a font atlas to the current batch. * @param state The jbgl rendering state. * @param font Font containing the glyph texture atlas. * @param glyph Glyph to render. * @param pos Position at which to render the glyph. * @param col RGBA colour of the glyph. */ 
	void jbgl_render_glyph(JbglState* state, JbglFont* font, JbglGlyph* glyph, vec2 pos, vec4 col);

	/** * @brief Draw a string of text using a font. * @param state The jbgl rendering state. * @param text Null-terminated UTF-8 string to render. * @param pos Starting position of the text. * @param font Font used to render the text. * @param wrap Whether text should wrap at the rendering viewport width. * @param col RGBA colour of the text. * @param bold Whether artificial bold styling is enabled. * @param italic Whether italic styling is enabled. * @param underline Whether underline rendering is enabled. * @param strikethrough Whether strikethrough rendering is enabled. * @return The final drawing position after rendering the text. */ 
	JbglTextInfo jbgl_draw_text(JbglState* state, const char* text, vec2 pos, JbglFont* font, bool wrap, vec4 col, bool bold, bool italic, bool underline, bool strikethrough);

	/** * @brief Draw animated text with a vertical sine-wave displacement applied to each glyph. * @param state The jbgl rendering state. * @param text Null-terminated UTF-8 string to render. * @param pos Starting position of the text. * @param font Font used to render the text. * @param wrap Whether text should wrap at the rendering viewport width. * @param col RGBA colour of the text. * @param bold Whether artificial bold styling is enabled. * @param italic Whether italic styling is enabled. * @param underline Whether underline rendering is enabled. * @param strikethrough Whether strikethrough rendering is enabled. * @param time Time value used to control the animation. * @return The final drawing position after rendering the text. */ 
	JbglTextInfo jbgl_draw_text_animated(JbglState* state, const char* text, vec2 pos, JbglFont* font, bool wrap, vec4 col, bool bold, bool italic, bool underline, bool strikethrough, float time);

	/** * @brief Measure the dimensions required to render a string of text. * @param state The jbgl rendering state used for viewport dimensions and glyph caching. * @param font Font used to measure the text. * @param text Null-terminated UTF-8 string to measure. * @param start_pos Starting position of the text. * @param dest Destination vector receiving the measured width and height. * @param wrap Whether text should wrap at the rendering viewport width. */ 
	void jbgl_measure_text(JbglState* state, JbglFont* font, const char* text, vec2 start_pos, vec2 dest, bool wrap);

	void jbgl_measure_text_range(JbglState* state, JbglFont* font, const char* text, int start, int end, vec2 start_pos, vec2 dest, bool wrap);
	/** * @brief Free a loaded font, its glyph cache, and its OpenGL texture atlas. * @param font Font to free. */ 
	void jbgl_free_font(JbglFont* font);


	// -- Shaders --

	/** * @brief Create an OpenGL shader program from vertex and fragment shader source code. * @param vs_source Vertex shader source code. * @param fs_source Fragment shader source code. * @return The created shader program. */ 
	JbglShader jbgl_init_shader(const char* vs_source, const char* fs_source);

	/** * @brief Create an OpenGL shader program by loading vertex and fragment shader source from files. * @param vs_filepath Path to the vertex shader file. * @param fs_filepath Path to the fragment shader file. * @return The created shader program. */ 
	JbglShader jbgl_init_shader_from_file(const char* vs_filepath, const char* fs_filepath);

	/** * @brief Set an integer uniform in a shader program. * @param shader Shader program containing the uniform. * @param name Name of the uniform. * @param value Integer value to assign to the uniform. */ 
	void jbgl_shader_set_int(JbglShader shader, const char* name, int value);

	/** * @brief Set a floating-point uniform in a shader program. * @param shader Shader program containing the uniform. * @param name Name of the uniform. * @param value Floating-point value to assign to the uniform. */ 
	void jbgl_shader_set_float(JbglShader shader, const char* name, float value);

	/** * @brief Set a 4x4 matrix uniform in a shader program. * @param shader Shader program containing the uniform. * @param name Name of the uniform. * @param value 4x4 matrix to assign to the uniform. */ 
	void jbgl_shader_set_mat4(JbglShader shader, const char* name, mat4 value);


	// -- Utilities --

	/** * @brief Decode the next UTF-8 codepoint from a string. * @param s Pointer to the string pointer being advanced. * @return The decoded Unicode codepoint. */ 
	uint32_t utf8_next(const char** s);
	/** * @brief Convert a hexadecimal RGB colour string to an RGBA vector. * @param hex Hexadecimal colour string in RRGGBB format, optionally beginning with #. * @param colour Destination vec4 receiving the converted colour. */ 
	void hex_to_vec4(const char* hex, vec4 colour);

#ifdef __cplusplus
} 
#endif