#pragma once
#include <glad/gl.h>
#include <cglm/cglm.h>

#include <ft2build.h>
#include FT_FREETYPE_H  


#define JBGL_2D_DEPTH 0
#define MAX_VERTS 10000
#define JBGL_MAX_BATCH_INSTANCES 200
#define JBGL_MAX_BATCH_TEXTURES 32

#ifdef __cplusplus
extern "C" 
{
#endif
	typedef struct JbglVertex
	{
		vec3 pos;
		vec2 tex_coords;
	} JbglVertex;

	typedef struct JbglTexture
	{
		GLuint id;
		int width;
		int height;

	}JbglTexture;

	typedef struct JbglShader 
	{
		GLuint id;
	} JbglShader;

	typedef struct JbglRectInstance
	{
		vec3 pos;
		vec2 size;
		vec4 colour;
		int tex_index;
		vec2 uv0;
		vec2 uv1;


	} JbglRectInstance;

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


	typedef struct JbglRectangle
	{
		float w, h;
		vec2 centre;
	} JbglRectangle;




	typedef struct JbglGlyph
	{
		vec2 size; // size of glyph
		vec2 bearing; // offset from baseline to left
		GLuint advance; // offset advance of next glyph
		int codepoint;


		vec2 uv0; // UV location of place in atlas (top left)
		vec2 uv1; // UV location of place in atlas (bottom right)
	} JbglGlyph;

	typedef struct JbglGlyphCache
	{
		JbglGlyph* cache;
		GLuint count;
		int capacity;
	} JbglGlyphCache;

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

	} JbglFont;


	void jbgl_cache_glyph(JbglState* state, JbglFont* font, JbglGlyph glyph);
	JbglGlyph jbgl_get_glyph_from_cache(JbglState* state, JbglFont* font, int codepoint);



	void jbgl_init_batch_renderer(JbglState* state);
	void jbgl_begin_batch(JbglState* state);
	void jbgl_end_batch(JbglState* state);

	void jbgl_draw_texture(JbglState* state, JbglTexture tex, vec3 pos, int w, int h);
	void jbgl_batch_set_proj_mat(JbglState* state);

	void jbgl_batch_renderer_flush(JbglState* state);
	int jbgl_batch_add_tex(JbglState* state, JbglTexture tex);
	int jbgl_find_texture(JbglState* state, const JbglTexture tex);
	


	JbglRectInstance* jbgl_add_rect_instance(JbglState* state, JbglRectangle rect, uint8_t tex_index);

	void jbgl_destroy_texture(JbglTexture* tex);


	// Fonts:

	JbglFont* jbgl_load_font(JbglState* state, const char* filepath, int size);
	void jbgl_draw_text(JbglState* state, const char* text, vec2 pos, JbglFont* font);

	JbglGlyph jbgl_find_glyph(JbglFont* font, int index);

	void jbgl_render_glyph(JbglState* state, JbglFont* font, JbglGlyph* glyph, vec2 pos);

	void jbgl_free_font(JbglFont* font);



	// Shaders ---------------------------------------------------------------------
	JbglShader jbgl_init_shader(const char* vs_source, const char* fs_source);
	//void jbgl_use_shader(JbglShader shader);
	
	JbglShader jbgl_init_shader_from_file(const char* vs_filepath, const char* fs_filepath);
	
	void jbgl_shader_set_int(JbglShader shader, const char* name, int value);
	void jbgl_shader_set_float(JbglShader shader, const char* name, float value);
	
	//void jbgl_shader_set_vec2();
	//void jbgl_shader_set_vec3();
	//void jbgl_shader_set_vec4();

	void jbgl_shader_set_mat4(JbglShader shader, const char* name, mat4 value);

	//--------------------------------------------------------------------------------
	JbglState* jbgl_init(int screen_w, int screen_h);
	void jbgl_batch_cleanup(JbglState* state);
	
	JbglTexture jbgl_load_texture(const char* filepath);
	

	int jbgl_gladLoadGL(GLADloadfunc load);





#ifdef __cplusplus
} 
#endif