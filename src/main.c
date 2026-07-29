#include "jbgl.h"
#include "jobanote.h"


#include <glad/gl.h>
#include <GLFW/glfw3.h>

void on_init();
void on_render();
void on_update();

JbglTexture texture;
JbglTexture tex2;
int screen_w, screen_h;
JbglShader shader;
JbglState* state;

// -----

// -----

int main(void)
{
	screen_w = 1080; screen_h = 768;

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(screen_w, screen_h, "jobanote", NULL, NULL);
	glfwMakeContextCurrent(window);
	jbgl_gladLoadGL(glfwGetProcAddress);

	state = jbgl_init(screen_w, screen_h);

	on_init();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.76f, 0.65f, 0.51f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		

		// Update
		on_update();
		// Render
		on_render();
		
		// --
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	
	jbgl_destroy_texture(&texture);
	jbgl_destroy_texture(&tex2);
	jbgl_batch_cleanup(state);

	

	return 0;
}

void on_init()
{

	texture = jbgl_load_texture("resources/textures/southey.jpg");
	tex2 = jbgl_load_texture("resources/textures/strawberry.jpg");

	// XML
	//run_program();

}

void on_update()
{

}

void on_render()
{
	jbgl_begin_batch(state);


	vec2 pos = {100.0f, 0.0f};
	vec2 pos2 = { 500.0f, 100.0f };
	jbgl_draw_texture(state, texture, pos, 100, 100);
	jbgl_draw_texture(state, tex2, pos2, 100, 100);

	jbgl_end_batch(state);
	

}