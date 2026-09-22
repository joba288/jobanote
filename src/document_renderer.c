#include "jobanote.h"
#include "document_renderer.h"
#include "GLFW/glfw3.h"

static void draw_cursor(Document* doc, TextNode* text, float x, float y, JbglState* state, JbglFont* font, JbglTexture cursor_texture)
{
	if (text != doc->cursor.text_node)
		return;

	vec2 size;

	jbgl_measure_text_range(state, font, text->data, 0, doc->cursor.character, (vec2) { x, y }, size, false);

	float cursor_x = x + size[0];
	float cursor_height = size[1] - 2.0f;

	jbgl_draw_texture(state, cursor_texture, (vec3) { cursor_x, y - (size[1] * 0.85f), JBGL_2D_DEPTH }, 4, cursor_height, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });
}

static void render_toolbar(Document* doc,JbglState* state,JbglFont* font,int screen_width)
{
	const float toolbar_height = 36.0f;

	// Toolbar background
	jbgl_draw_rect(state,(vec3) {0.0f, 0.0f, JBGL_2D_DEPTH},screen_width,(int)toolbar_height, (vec4) { 0.105f, 0.22f, 0.34f, 1.0f });

	// Application name
	jbgl_draw_text(state,"jobanote",(vec2) {15.0f, 25.0f},font,false,(vec4) {1.0f, 1.0f, 1.0f, 1.0f},true,false, false,false);

	// Current span information 
	TextNode* text = doc->cursor.text_node;

	if (text && text->resolved_style)
	{
		char span_info[256];

		snprintf(
			span_info,
			sizeof(span_info),
			"Bold: %s   Italic: %s   Underline: %s   Strikethrough: %s",
			text->resolved_style->bold ? "On" : "Off",
			text->resolved_style->italic ? "On" : "Off",
			text->resolved_style->underline ? "On" : "Off",
			text->resolved_style->strikethrough ? "On" : "Off"
		);

		jbgl_draw_text(state,span_info,(vec2) {420.0f, 25.0f},font,false,(vec4) {1.0f, 1.0f, 1.0f, 1.0f},false,false,false,false);
	}
	else
	{
		jbgl_draw_text(state,"No span",(vec2) {320.0f, 25.0f},font,false,(vec4) {0.7f, 0.7f, 0.7f, 1.0f},false,false,false,false);
	}
}

void render_document(Document* doc, JbglState* state, JbglFont* font, JbglFont* toolbar_font, JbglTexture cursor_texture, float scroll_y)
{
	// sidebar
	jbgl_draw_rect(state, (vec3) { 0.0f, 0.0f, 0.0f }, 64, state->screen_h, (vec4) { 0.105f, 0.22f, 0.34f, 1.0f });
	
	Cursor start;
	Cursor end;

	if (cursor_compare(doc, doc->selection_start, doc->cursor) <= 0)
	{
		start = doc->selection_start;
		end = doc->cursor;
	}
	else
	{
		start = doc->cursor;
		end = doc->selection_start;
	}

	const float toolbar_height = 70.0f;
	const float document_margin = 20.0f;

	vec2 pos = {
		document_margin,
		toolbar_height + document_margin + scroll_y
	};
	float line_height = font->face->size->metrics.height / 64.0f;

	ParagraphNode* paragraph = doc->paragraphs;
	
	int paragraph_count = 1;

	while (paragraph)
	{
		float x = pos[0];
		TextNode* text = paragraph->text;

		char paragraph_number[12];
		itoa(paragraph_count, paragraph_number, 10);

		// Draw Line Number
		JbglTextInfo number_info = jbgl_draw_text(state, paragraph_number, pos, font, false, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, false, true, false, false);

		x = number_info.pos[0] + line_height;

		// Draw Spans
		while (text)
		{
			if (doc->selecting)
				draw_selection(doc, start, end, text, x, pos[1], state, font);

			vec4 colour;
			hex_to_vec4(text->resolved_style->colour, colour);

			//JbglTextInfo text_info = jbgl_draw_text_animated(state, text->data, (vec2) { x, pos[1] }, font, true, colour, text->resolved_style->bold, text->resolved_style->italic, text->resolved_style->underline, text->resolved_style->strikethrough, glfwGetTime());
			JbglTextInfo text_info = jbgl_draw_text(state, text->data, (vec2) { x, pos[1] }, font, true, colour, text->resolved_style->bold, text->resolved_style->italic, text->resolved_style->underline, text->resolved_style->strikethrough);

			draw_cursor(doc, text, x, pos[1], state, font, cursor_texture);

			x = text_info.pos[0];
			pos[1] = text_info.pos[1];

			text = text->next;
		}

		pos[1] += line_height;
		paragraph = paragraph->next;
		paragraph_count++;
	}

	render_toolbar(doc, state, toolbar_font, state->screen_w);


}

void draw_selection(Document* doc, Cursor start, Cursor end, TextNode* text, float x, float y, JbglState* state, JbglFont* font)
{
	if (!doc || !text || !text->data)
		return;

	if (!text_node_in_selection(doc, text, start, end))
		return;

	int text_length = (int)strlen(text->data);

	int start_char = 0;
	int end_char = text_length;

	if (text == start.text_node)
		start_char = start.character;

	if (text == end.text_node)
		end_char = end.character;

	if (start_char < 0)
		start_char = 0;

	if (start_char > text_length)
		start_char = text_length;

	if (end_char < 0)
		end_char = 0;

	if (end_char > text_length)
		end_char = text_length;

	if (start_char >= end_char)
		return;

	vec2 before_size;
	vec2 selected_size;

	jbgl_measure_text_range(state, font, text->data, 0, start_char, (vec2) { x, y }, before_size, false);
	jbgl_measure_text_range(state, font, text->data, start_char, end_char, (vec2) { x, y }, selected_size, false);

	float line_height = font->face->size->metrics.height / 64.0f;

	jbgl_draw_rect(state, (vec3) { x + before_size[0], y - line_height * 0.8f, JBGL_2D_DEPTH }, (int)selected_size[0], (int)line_height, (vec4) { 0.2f, 0.4f, 1.0f, 1.0f });
}