#include <jbgl.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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

	instance->tex_index = tex_index;
	glm_vec4_copy((vec4) { 0.0f, 0.0f, 0.0f, 1.0f }, instance->colour);
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
	}

	state->textures[state->tex_index++] = tex;
	return state->tex_count++;
}

void jbgl_batch_set_proj_mat(JbglState* state)
{
	mat4 proj = GLM_MAT4_IDENTITY_INIT;
	glm_ortho(0.0f, (float)state->screen_w, (float)state->screen_h, 0.0f, -1.0f, 1.0f,proj);

	jbgl_shader_set_mat4(state->shader, "proj", proj);
}

int jbgl_find_texture(JbglState* state, JbglTexture tex)
{
	for (int i = 0; i < state->tex_count; i++)
	{
		if (state->textures[i].id == tex.id)
			return i;
	}

	return -1;
}

void jbgl_draw_texture(JbglState* state, JbglTexture tex, vec3 pos, int w, int h)
{
	int tex_index = jbgl_batch_add_tex(state, tex);

	JbglRectangle rect;
	rect.centre[0] = pos[0];
	rect.centre[1] = pos[1];

	rect.w = w;
	rect.h = h;
	
	jbgl_add_rect_instance(state, rect, tex_index);
}

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


JbglState* jbgl_init(int screen_w, int screen_h)
{
	JbglState* state = malloc(sizeof(*state));
	if (!state)
		return NULL;

	state->screen_w = screen_w;
	state->screen_h = screen_h;

	state->instance_count = 0;
	state->tex_count = 0;

	jbgl_init_batch_renderer(state);

	return state;

}

void jbgl_batch_cleanup(JbglState* state)
{
	// Cleanup batch renderer
	glDeleteVertexArrays(1, &state->vao_id);
	glDeleteBuffers(1, &state->identity_vbo_id);
	glDeleteBuffers(1, &state->identity_ebo_id);
	glDeleteBuffers(1, &state->vbo_id);
	glDeleteProgram(state->shader.id);
	free(state->instances);
	free(state);
}

void jbgl_destroy_texture(JbglTexture* tex)
{
	if (tex->id != 0)
	{
		glDeleteTextures(1, &tex->id);
		tex->id = 0;
	}
}


int jbgl_gladLoadGL(GLADloadfunc load)
{
	return gladLoadGL(load);
}

// Shaders ------------------------------------


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


JbglTexture jbgl_load_texture(const char* filepath)
{
	JbglTexture tex = {0};
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image);

	tex.width = width;
	tex.height = height;
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}


