#include "jbgl.h"
#include "jobanote.h"
#include "odt.h"
#include "document_renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

void on_init();
void on_render();
void on_update();
void on_cleanup();

// TODO Remove need for globals
// TODO Asset Manager
typedef struct
{
	JbglState* state;
	JbglFont* font;
	JbglFont* toolbar_font;
	JbglTexture cursor_texture;

	Document doc;

	float scroll_y;

	int screen_w;
	int screen_h;

} Application;

// -----
Application app;
void debug_document(Document* doc);

// --- Callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void character_callback(GLFWwindow* window, unsigned int codepoint);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(void)
{
	app.screen_w = 1080; app.screen_h = 768;

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(app.screen_w, app.screen_h, "jobanote", NULL, NULL);
	glfwMakeContextCurrent(window);
	jbgl_gladLoadGL(glfwGetProcAddress);

	// Callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCharCallback(window, character_callback);
	glfwSetKeyCallback(window, key_callback);

	app.state = jbgl_init(app.screen_w, app.screen_h);
	on_init();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.071f, 0.165f, 0.263, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		

		// Update
		on_update();
		// Render
		on_render();
		
		// --
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	on_cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}


// --------------------------



void on_init()
{
	// -- Asset Loading -- 
	// TODO Asset Manager
	app.cursor_texture = jbgl_load_texture("resources/textures/southey.jpg");
	app.font = jbgl_load_font(app.state, "resources/fonts/arial.ttf", 42);
	app.toolbar_font = jbgl_load_font(app.state, "resources/fonts/arial.ttf", 25);

	// XML
	app.doc = *run_program();


}

void on_update()
{

}

void on_render()
{
	jbgl_begin_batch(app.state);

		
		render_document(&app.doc, app.state, app.font, app.toolbar_font, app.cursor_texture, app.scroll_y);
	
	jbgl_end_batch(app.state);
}

void on_cleanup()
{
	export_odt(&app.doc, "resources/output.odt");
	jbgl_destroy_texture(&app.cursor_texture);
	jbgl_free_font(app.font);
	jbgl_batch_cleanup(app.state);
}




void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	// regenerate projection matrix
	app.state->screen_w = width;
	app.state->screen_h = height;
	glUseProgram(app.state->shader.id);
	jbgl_batch_set_proj_mat(app.state);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	app.scroll_y += (float)yoffset * 30.0f;   

	if (app.scroll_y > 0.0f)
		app.scroll_y = 0.0f;                 
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (has_selection(&app.doc))
	{
		delete_selection(&app.doc);
	}

	insert_character(&app.doc, codepoint);
}

void debug_document(Document* doc)
{
	printf("\n========== DOCUMENT ==========\n");

	ParagraphNode* paragraph = doc->paragraphs;
	int p = 1;

	while (paragraph)
	{
		printf("\nParagraph %d: %p\n",p,(void*)paragraph);

		printf(
			"  prev = %p\n"
			"  next = %p\n"
			"  text = %p\n",
			(void*)paragraph->prev,
			(void*)paragraph->next,
			(void*)paragraph->text
		);

		TextNode* text = paragraph->text;
		int t = 1;

		while (text)
		{
			printf(
				"  Text %d: %p | prev=%p next=%p | \"%s\"\n",
				t,
				(void*)text,
				(void*)text->prev,
				(void*)text->next,
				text->data
			);

			text = text->next;
			t++;
		}

		paragraph = paragraph->next;
		p++;
	}

	printf("\nCURSOR:\n");
	printf(
		"  paragraph = %p\n"
		"  text      = %p\n"
		"  character = %d\n",
		(void*)doc->cursor.paragraph_node,
		(void*)doc->cursor.text_node,
		doc->cursor.character
	);

	printf("==============================\n\n");
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (mods & GLFW_MOD_SHIFT)
		{
			// Start a new selection only when one isn't already active
			if (!app.doc.selecting)
			{
				app.doc.selection_start = app.doc.cursor;
				app.doc.selecting = true;
			}

			decrement_cursor(&app.doc);
		}
		else
		{
			// Move normally and completely clear selection
			app.doc.selecting = false;

			decrement_cursor(&app.doc);

			// Keep selection_start synchronized with cursor
			app.doc.selection_start = app.doc.cursor;
		}
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (mods & GLFW_MOD_SHIFT)
		{
			// Start a new selection only when one isn't already active
			if (!app.doc.selecting)
			{
				app.doc.selection_start = app.doc.cursor;
				app.doc.selecting = true;
			}

			increment_cursor(&app.doc);
		}
		else
		{
			// Move normally and completely clear selection
			app.doc.selecting = false;

			increment_cursor(&app.doc);

			// Keep selection_start synchronized with cursor
			app.doc.selection_start = app.doc.cursor;
		}
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) 
	{
		if (has_selection(&app.doc))
		{
			delete_selection(&app.doc);
		} 
		insert_paragraph_node_at_cursor(&app.doc);
	}

	if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (has_selection(&app.doc))
		{
			delete_selection(&app.doc);
		}
		else
		{
			delete_character(&app.doc);
		}
	}
	if (key == GLFW_KEY_DELETE && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (has_selection(&app.doc))
			delete_selection(&app.doc);
		else
			delete_character_forward(&app.doc);
	}

	if (key == GLFW_KEY_B && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
	{
		toggle_bold_selection(&app.doc);
	}


}