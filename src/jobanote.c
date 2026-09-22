#include "jobanote.h"

void increment_cursor(Document* doc)
{
	Cursor* cursor = &doc->cursor;

	if (!cursor->text_node)
	{
		if (cursor->paragraph_node->next)
		{
			cursor->paragraph_node = cursor->paragraph_node->next;
			cursor->text_node = cursor->paragraph_node->text;
			cursor->character = 0;
		}

		return;
	}

	int current_len = (int)strlen(cursor->text_node->data);

	if (cursor->character < current_len)
	{
		cursor->character += utf8_char_len(cursor->text_node->data + cursor->character);
		return;
	}

	if (cursor->text_node->next)
	{
		cursor->text_node = cursor->text_node->next;
		cursor->character = 0;
		increment_cursor(doc);
		return;
	}

	if (cursor->paragraph_node->next)
	{
		cursor->paragraph_node = cursor->paragraph_node->next;
		cursor->text_node = cursor->paragraph_node->text;
		cursor->character = 0;
	}
}

void decrement_cursor(Document* doc)
{
	Cursor* cursor = &doc->cursor;

	if (!cursor->paragraph_node)
		return;

	if (!cursor->text_node)
	{
		if (!cursor->paragraph_node->prev)
			return;

		cursor->paragraph_node = cursor->paragraph_node->prev;
		cursor->text_node = last_text_node(cursor->paragraph_node->text);

		if (!cursor->text_node)
		{
			cursor->character = 0;
			decrement_cursor(doc);
			return;
		}

		cursor->character = (int)strlen(cursor->text_node->data);
		return;
	}

	if (cursor->character > 0)
	{
		cursor->character -= utf8_prev_char_len(cursor->text_node->data, cursor->character);
		return;
	}

	if (cursor->text_node->prev)
	{
		cursor->text_node = cursor->text_node->prev;
		cursor->character = (int)strlen(cursor->text_node->data);
		decrement_cursor(doc);
		return;
	}

	if (cursor->paragraph_node->prev)
	{
		cursor->paragraph_node = cursor->paragraph_node->prev;
		cursor->text_node = last_text_node(cursor->paragraph_node->text);

		if (cursor->text_node)
			cursor->character = (int)strlen(cursor->text_node->data);
		else
			cursor->character = 0;
	}
}

bool cursor_equal(Cursor* a, Cursor* b)
{
	return a->paragraph_node == b->paragraph_node &&
		a->text_node == b->text_node &&
		a->character == b->character;
}

int cursor_compare(Document* doc, Cursor a, Cursor b)
{
	if (cursor_equal(&a, &b))
		return 0;

	ParagraphNode* paragraph = doc->paragraphs;

	while (paragraph)
	{
		TextNode* text = paragraph->text;

		while (text)
		{
			if (paragraph == a.paragraph_node && text == a.text_node && a.character == 0)
				return -1;

			if (paragraph == b.paragraph_node && text == b.text_node && b.character == 0)
				return 1;

			if (paragraph == a.paragraph_node && text == a.text_node)
			{
				if (paragraph == b.paragraph_node && text == b.text_node)
					return a.character < b.character ? -1 : 1;

				return -1;
			}

			if (paragraph == b.paragraph_node && text == b.text_node)
				return 1;

			text = text->next;
		}

		if (paragraph == a.paragraph_node && paragraph != b.paragraph_node)
			return -1;

		if (paragraph == b.paragraph_node && paragraph != a.paragraph_node)
			return 1;

		paragraph = paragraph->next;
	}

	return 0;
}

bool text_node_in_selection(Document* doc, TextNode* target, Cursor start, Cursor end)
{
	if (!doc || !target)
		return false;

	ParagraphNode* paragraph = doc->paragraphs;

	while (paragraph)
	{
		TextNode* text = paragraph->text;

		while (text)
		{
			if (text == target)
			{
				int len = text->data ? (int)strlen(text->data) : 0;

				Cursor node_start = { paragraph, text, 0 };
				Cursor node_end = { paragraph, text, len };

				int start_vs_node_end = cursor_compare(doc, start, node_end);
				int end_vs_node_start = cursor_compare(doc, end, node_start);

				return start_vs_node_end < 0 && end_vs_node_start > 0;
			}

			text = text->next;
		}

		paragraph = paragraph->next;
	}

	return false;
}

bool has_selection(Document* doc)
{
	if (!doc || !doc->selecting)
		return false;

	return cursor_compare(doc, doc->selection_start, doc->cursor) != 0;
}

void delete_selection(Document* doc)
{
	if (!doc || !has_selection(doc))
	{
		if (doc)
			doc->selecting = false;

		return;
	}

	Cursor start;
	Cursor end;

	if (cursor_compare(doc, doc->selection_start, doc->cursor) < 0)
	{
		start = doc->selection_start;
		end = doc->cursor;
	}
	else
	{
		start = doc->cursor;
		end = doc->selection_start;
	}

	if (!start.paragraph_node || !end.paragraph_node)
	{
		doc->selecting = false;
		return;
	}

	if (start.paragraph_node == end.paragraph_node && start.text_node == end.text_node)
	{
		TextNode* node = start.text_node;

		if (!node || !node->data)
		{
			doc->cursor = start;
			doc->selection_start = start;
			doc->selecting = false;
			return;
		}

		int len = (int)strlen(node->data);
		int a = start.character;
		int b = end.character;

		if (a < 0)
			a = 0;

		if (b < 0)
			b = 0;

		if (a > len)
			a = len;

		if (b > len)
			b = len;

		if (a > b)
		{
			int temp = a;
			a = b;
			b = temp;
		}

		memmove(node->data + a, node->data + b, len - b + 1);

		doc->cursor.paragraph_node = start.paragraph_node;
		doc->cursor.text_node = node;
		doc->cursor.character = a;
		doc->selection_start = doc->cursor;
		doc->selecting = false;

		return;
	}

	ParagraphNode* start_para = start.paragraph_node;
	ParagraphNode* end_para = end.paragraph_node;

	TextNode* start_node = start.text_node;
	TextNode* end_node = end.text_node;

	TextNode* suffix = NULL;

	if (end_node && end_node->data)
	{
		int end_len = (int)strlen(end_node->data);
		int end_pos = end.character;

		if (end_pos < 0)
			end_pos = 0;

		if (end_pos > end_len)
			end_pos = end_len;

		if (end_pos < end_len)
		{
			int suffix_len = end_len - end_pos;

			suffix = malloc(sizeof(TextNode));

			if (!suffix)
				return;

			memset(suffix, 0, sizeof(TextNode));

			suffix->data = malloc(suffix_len + 1);

			if (!suffix->data)
			{
				free(suffix);
				return;
			}

			memcpy(suffix->data, end_node->data + end_pos, suffix_len);
			suffix->data[suffix_len] = '\0';

			if (end_node->resolved_style)
			{
				suffix->resolved_style = malloc(sizeof(Style));

				if (!suffix->resolved_style)
				{
					free(suffix->data);
					free(suffix);
					return;
				}

				*suffix->resolved_style = *end_node->resolved_style;
			}

			suffix->prev = NULL;
			suffix->next = NULL;
		}
	}

	TextNode* after_end = NULL;

	if (end_node)
	{
		after_end = end_node->next;

		if (after_end)
			after_end->prev = NULL;

		end_node->next = NULL;
	}

	if (suffix)
	{
		suffix->next = after_end;

		if (after_end)
			after_end->prev = suffix;
	}
	else
	{
		suffix = after_end;
	}

	if (start_node && start_node->data)
	{
		int start_len = (int)strlen(start_node->data);
		int start_pos = start.character;

		if (start_pos < 0)
			start_pos = 0;

		if (start_pos > start_len)
			start_pos = start_len;

		start_node->data[start_pos] = '\0';
	}

	if (start_node)
	{
		TextNode* node = start_node->next;
		start_node->next = NULL;

		while (node && node != end_node)
		{
			TextNode* next = node->next;

			free(node->data);
			free(node->resolved_style);
			free(node);

			node = next;
		}
	}
	else
	{
		TextNode* node = start_para->text;
		start_para->text = NULL;

		while (node && node != end_node)
		{
			TextNode* next = node->next;

			free(node->data);
			free(node->resolved_style);
			free(node);

			node = next;
		}
	}

	if (start_para == end_para)
	{
		if (end_node)
		{
			free(end_node->data);
			free(end_node->resolved_style);
			free(end_node);
		}

		if (suffix)
		{
			if (start_node)
			{
				start_node->next = suffix;
				suffix->prev = start_node;
			}
			else
			{
				start_para->text = suffix;
				suffix->prev = NULL;
			}
		}

		doc->cursor.paragraph_node = start_para;

		if (start_node)
		{
			doc->cursor.text_node = start_node;
			doc->cursor.character = start.character;

			int len = (int)strlen(start_node->data);

			if (doc->cursor.character < 0)
				doc->cursor.character = 0;

			if (doc->cursor.character > len)
				doc->cursor.character = len;
		}
		else
		{
			doc->cursor.text_node = start_para->text;
			doc->cursor.character = 0;
		}

		doc->selection_start = doc->cursor;
		doc->selecting = false;

		return;
	}

	ParagraphNode* paragraph = start_para->next;

	while (paragraph && paragraph != end_para)
	{
		ParagraphNode* next = paragraph->next;

		free_text_list(paragraph->text);
		free(paragraph);

		paragraph = next;
	}

	if (end_node)
	{
		TextNode* node = end_para->text;

		while (node && node != end_node)
		{
			TextNode* next = node->next;

			free(node->data);
			free(node->resolved_style);
			free(node);

			node = next;
		}

		if (node == end_node)
		{
			free(end_node->data);
			free(end_node->resolved_style);
			free(end_node);
		}
	}

	end_para->text = NULL;

	ParagraphNode* after_para = end_para->next;

	start_para->next = after_para;

	if (after_para)
		after_para->prev = start_para;

	free(end_para);

	if (suffix)
	{
		if (start_node)
		{
			start_node->next = suffix;
			suffix->prev = start_node;
		}
		else
		{
			start_para->text = suffix;
			suffix->prev = NULL;
		}
	}

	doc->cursor.paragraph_node = start_para;

	if (start_node)
	{
		doc->cursor.text_node = start_node;
		doc->cursor.character = start.character;

		int len = (int)strlen(start_node->data);

		if (doc->cursor.character < 0)
			doc->cursor.character = 0;

		if (doc->cursor.character > len)
			doc->cursor.character = len;
	}
	else
	{
		doc->cursor.text_node = start_para->text;
		doc->cursor.character = 0;
	}

	doc->selection_start = doc->cursor;
	doc->selecting = false;
}

void init_document(Document* doc)
{
	if (!doc)
		return;

	doc->paragraphs = NULL;

	init_paragraph_list(&doc->paragraphs);
	init_style_dictionary(&doc->dictionary);

	ParagraphNode* paragraph = malloc(sizeof(ParagraphNode));

	if (!paragraph)
		return;

	paragraph->text = NULL;
	paragraph->prev = NULL;
	paragraph->next = NULL;

	doc->paragraphs = paragraph;

	doc->cursor.paragraph_node = paragraph;
	doc->cursor.text_node = NULL;
	doc->cursor.character = 0;

	doc->selection_start = doc->cursor;
	doc->selecting = false;
}

void free_document(Document* doc)
{
	free_paragraph_list(doc->paragraphs);
	free_style_dictionary(&doc->dictionary);
}

void insert_character(Document* doc, int codepoint)
{
	if (!doc || !doc->cursor.paragraph_node)
		return;

	Cursor* cursor = &doc->cursor;

	char utf8[5] = { 0 };
	int utf8_len = 0;

	if (codepoint <= 0x7F)
	{
		utf8[0] = (char)codepoint;
		utf8_len = 1;
	}
	else if (codepoint <= 0x7FF)
	{
		utf8[0] = (char)(0xC0 | (codepoint >> 6));
		utf8[1] = (char)(0x80 | (codepoint & 0x3F));
		utf8_len = 2;
	}
	else if (codepoint <= 0xFFFF)
	{
		utf8[0] = (char)(0xE0 | (codepoint >> 12));
		utf8[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		utf8[2] = (char)(0x80 | (codepoint & 0x3F));
		utf8_len = 3;
	}
	else if (codepoint <= 0x10FFFF)
	{
		utf8[0] = (char)(0xF0 | (codepoint >> 18));
		utf8[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
		utf8[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		utf8[3] = (char)(0x80 | (codepoint & 0x3F));
		utf8_len = 4;
	}
	else
	{
		return;
	}

	if (!cursor->text_node)
	{
		TextNode* new_node = malloc(sizeof(TextNode));

		if (!new_node)
			return;

		new_node->data = malloc(utf8_len + 1);

		if (!new_node->data)
		{
			free(new_node);
			return;
		}

		memcpy(new_node->data, utf8, utf8_len);
		new_node->data[utf8_len] = '\0';

		new_node->resolved_style = malloc(sizeof(Style));

		if (!new_node->resolved_style)
		{
			free(new_node->data);
			free(new_node);
			return;
		}

		init_style(new_node->resolved_style);

		new_node->prev = NULL;
		new_node->next = NULL;

		cursor->paragraph_node->text = new_node;
		cursor->text_node = new_node;
		cursor->character = utf8_len;

		return;
	}

	char* old = cursor->text_node->data;
	int old_len = (int)strlen(old);

	if (cursor->character < 0)
		cursor->character = 0;

	if (cursor->character > old_len)
		cursor->character = old_len;

	char* new_data = malloc(old_len + utf8_len + 1);

	if (!new_data)
		return;

	memcpy(new_data, old, cursor->character);
	memcpy(new_data + cursor->character, utf8, utf8_len);
	memcpy(new_data + cursor->character + utf8_len, old + cursor->character, old_len - cursor->character + 1);

	free(old);

	cursor->text_node->data = new_data;
	cursor->character += utf8_len;
}

void merge_paragraph_nodes(ParagraphNode* pDest, ParagraphNode* pMerge)
{
	if (!pDest || !pMerge || pDest == pMerge)
		return;

	if (!pDest->text)
	{
		pDest->text = pMerge->text;
	}
	else if (pMerge->text)
	{
		TextNode* text_end = last_text_node(pDest->text);

		text_end->next = pMerge->text;
		pMerge->text->prev = text_end;
	}

	pMerge->text = NULL;

	if (pMerge->prev)
		pMerge->prev->next = pMerge->next;

	if (pMerge->next)
		pMerge->next->prev = pMerge->prev;

	free(pMerge);
}

int utf8_char_len(const char* str)
{
	unsigned char c = (unsigned char)str[0];

	if (c < 0x80)
		return 1;

	if ((c & 0xE0) == 0xC0)
		return 2;

	if ((c & 0xF0) == 0xE0)
		return 3;

	if ((c & 0xF8) == 0xF0)
		return 4;

	return 1;
}

int utf8_prev_char_len(const char* str, int byte_pos)
{
	if (!str || byte_pos <= 0)
		return 0;

	int pos = byte_pos - 1;

	while (pos > 0 && ((unsigned char)str[pos] & 0xC0) == 0x80)
		pos--;

	return byte_pos - pos;
}

void delete_character_from_node(TextNode* node, int* cursor_position)
{
	if (!node || !node->data || !cursor_position)
		return;

	int pos = *cursor_position;

	if (pos <= 0)
		return;

	int char_len = utf8_prev_char_len(node->data, pos);

	if (char_len <= 0 || char_len > pos)
		return;

	int old_len = (int)strlen(node->data);

	memmove(node->data + pos - char_len, node->data + pos, old_len - pos + 1);

	*cursor_position -= char_len;
}

TextNode* last_text_node(TextNode* first)
{
	if (!first)
		return NULL;

	while (first->next)
		first = first->next;

	return first;
}

void delete_character(Document* doc)
{
	if (!doc)
		return;

	Cursor* cursor = &doc->cursor;

	if (!cursor->paragraph_node)
		return;

	if (!cursor->text_node)
	{
		if (!cursor->paragraph_node->prev)
			return;

		ParagraphNode* current = cursor->paragraph_node;
		ParagraphNode* previous = current->prev;

		if (!previous->text)
		{
			cursor->paragraph_node = previous;
			cursor->text_node = NULL;
			cursor->character = 0;

			previous->next = current->next;

			if (current->next)
				current->next->prev = previous;

			free(current);
			return;
		}

		cursor->paragraph_node = previous;
		cursor->text_node = last_text_node(previous->text);
		cursor->character = (int)strlen(cursor->text_node->data);

		previous->next = current->next;

		if (current->next)
			current->next->prev = previous;

		free(current);
		return;
	}

	if (cursor->character > 0)
	{
		int char_len = utf8_prev_char_len(cursor->text_node->data, cursor->character);

		if (char_len <= 0)
			return;

		int old_len = (int)strlen(cursor->text_node->data);

		memmove(cursor->text_node->data + cursor->character - char_len, cursor->text_node->data + cursor->character, old_len - cursor->character + 1);

		cursor->character -= char_len;
		return;
	}

	if (cursor->text_node->prev)
	{
		cursor->text_node = cursor->text_node->prev;
		cursor->character = (int)strlen(cursor->text_node->data);

		int char_len = utf8_prev_char_len(cursor->text_node->data, cursor->character);

		if (char_len <= 0)
			return;

		int old_len = (int)strlen(cursor->text_node->data);

		memmove(cursor->text_node->data + cursor->character - char_len, cursor->text_node->data + cursor->character, old_len - cursor->character + 1);

		cursor->character -= char_len;
		return;
	}

	if (cursor->paragraph_node->prev)
	{
		ParagraphNode* current = cursor->paragraph_node;
		ParagraphNode* previous = current->prev;
		TextNode* old_last = last_text_node(previous->text);

		if (old_last)
		{
			cursor->text_node = old_last;
			cursor->character = (int)strlen(old_last->data);
		}
		else
		{
			cursor->text_node = current->text;
			cursor->character = 0;
		}

		if (!previous->text)
		{
			previous->text = current->text;

			if (current->text)
				current->text->prev = NULL;
		}
		else if (current->text)
		{
			old_last->next = current->text;
			current->text->prev = old_last;
		}

		current->text = NULL;

		previous->next = current->next;

		if (current->next)
			current->next->prev = previous;

		cursor->paragraph_node = previous;

		free(current);
	}
}

void delete_character_forward(Document* doc)
{
	if (!doc || !doc->cursor.paragraph_node)
		return;

	Cursor* cursor = &doc->cursor;
	ParagraphNode* paragraph = cursor->paragraph_node;
	TextNode* node = cursor->text_node;

	if (!node)
	{
		if (paragraph->next)
		{
			ParagraphNode* next = paragraph->next;

			paragraph->next = next->next;

			if (next->next)
				next->next->prev = paragraph;

			free(next);
		}

		return;
	}

	int node_len = (int)strlen(node->data);

	if (cursor->character < node_len)
	{
		int char_len = utf8_char_len(node->data + cursor->character);

		if (char_len <= 0 || cursor->character + char_len > node_len)
			return;

		memmove(node->data + cursor->character, node->data + cursor->character + char_len, node_len - cursor->character - char_len + 1);

		if (node->data[0] == '\0')
		{
			TextNode* next = node->next;

			if (node->prev)
				node->prev->next = next;
			else
				paragraph->text = next;

			if (next)
				next->prev = node->prev;

			free(node->data);
			free(node->resolved_style);
			free(node);

			cursor->text_node = next;
			cursor->character = 0;
		}

		return;
	}

	if (node->next)
	{
		TextNode* next = node->next;

		if (!next->data || next->data[0] == '\0')
		{
			node->next = next->next;

			if (next->next)
				next->next->prev = node;

			free(next->data);
			free(next->resolved_style);
			free(next);
			return;
		}

		int next_len = (int)strlen(next->data);
		int char_len = utf8_char_len(next->data);

		if (char_len <= 0 || char_len > next_len)
			return;

		memmove(next->data, next->data + char_len, next_len - char_len + 1);

		if (next->data[0] == '\0')
		{
			node->next = next->next;

			if (next->next)
				next->next->prev = node;

			free(next->data);
			free(next->resolved_style);
			free(next);
		}

		return;
	}

	if (paragraph->next)
		merge_paragraph_nodes(paragraph, paragraph->next);
}

TextNode* split_text_node(TextNode* node, int position)
{
	if (!node || !node->data)
		return NULL;

	int len = (int)strlen(node->data);

	if (position <= 0)
		return node;

	if (position >= len)
		return NULL;

	TextNode* new_node = malloc(sizeof(TextNode));

	if (!new_node)
		return NULL;

	new_node->data = malloc(len - position + 1);

	if (!new_node->data)
	{
		free(new_node);
		return NULL;
	}

	memcpy(new_node->data, node->data + position, len - position);
	new_node->data[len - position] = '\0';

	new_node->resolved_style = malloc(sizeof(Style));

	if (!new_node->resolved_style)
	{
		free(new_node->data);
		free(new_node);
		return NULL;
	}

	if (node->resolved_style)
		*new_node->resolved_style = *node->resolved_style;
	else
		init_style(new_node->resolved_style);

	new_node->prev = node;
	new_node->next = node->next;

	if (node->next)
		node->next->prev = new_node;

	node->next = new_node;
	node->data[position] = '\0';

	return new_node;
}

void toggle_bold_selection(Document* doc)
{
	if (!doc || !has_selection(doc))
		return;

	Cursor start;
	Cursor end;

	if (cursor_compare(doc, doc->selection_start, doc->cursor) < 0)
	{
		start = doc->selection_start;
		end = doc->cursor;
	}
	else
	{
		start = doc->cursor;
		end = doc->selection_start;
	}

	if (!start.paragraph_node || !end.paragraph_node || !start.text_node || !end.text_node)
		return;

	if (start.paragraph_node == end.paragraph_node && start.text_node == end.text_node)
	{
		TextNode* node = start.text_node;
		int len = (int)strlen(node->data);

		int a = start.character;
		int b = end.character;

		if (a < 0)
			a = 0;

		if (b < 0)
			b = 0;

		if (a > len)
			a = len;

		if (b > len)
			b = len;

		if (a >= b)
			return;

		TextNode* selected = node;

		if (a > 0)
		{
			selected = split_text_node(node, a);

			if (!selected)
				return;
		}

		int selected_len = (int)strlen(selected->data);
		int selection_len = b - a;

		if (selection_len < selected_len && !split_text_node(selected, selection_len))
			return;

		selected->resolved_style->bold = !selected->resolved_style->bold;
		doc->selection_start = doc->cursor;
		doc->selecting = false;

		return;
	}

	TextNode* first = start.text_node;
	int first_len = (int)strlen(first->data);

	if (start.character > 0 && start.character < first_len)
	{
		first = split_text_node(first, start.character);

		if (!first)
			return;
	}

	TextNode* last = end.text_node;
	int last_len = (int)strlen(last->data);

	if (end.character > 0 && end.character < last_len)
		split_text_node(last, end.character);

	bool all_bold = true;

	ParagraphNode* paragraph = start.paragraph_node;
	TextNode* text = first;

	while (paragraph)
	{
		while (text)
		{
			if (!text->resolved_style || !text->resolved_style->bold)
			{
				all_bold = false;
				break;
			}

			if (text == last)
				break;

			text = text->next;
		}

		if (!all_bold || text == last)
			break;

		paragraph = paragraph->next;

		if (!paragraph)
			break;

		text = paragraph->text;
	}

	bool new_bold = !all_bold;

	paragraph = start.paragraph_node;
	text = first;

	while (paragraph)
	{
		while (text)
		{
			if (text->resolved_style)
				text->resolved_style->bold = new_bold;

			if (text == last)
				goto finished;

			text = text->next;
		}

		paragraph = paragraph->next;

		if (!paragraph)
			goto finished;

		text = paragraph->text;
	}

finished:
	doc->selection_start = doc->cursor;
	doc->selecting = false;
}

ParagraphNode* insert_paragraph_node_at_cursor(Document* doc)
{
	if (!doc || !doc->cursor.paragraph_node)
		return NULL;

	Cursor* cursor = &doc->cursor;
	ParagraphNode* current = cursor->paragraph_node;

	ParagraphNode* new_paragraph = malloc(sizeof(ParagraphNode));

	if (!new_paragraph)
		return NULL;

	new_paragraph->text = NULL;
	new_paragraph->prev = current;
	new_paragraph->next = current->next;

	if (current->next)
		current->next->prev = new_paragraph;

	current->next = new_paragraph;

	if (!cursor->text_node)
	{
		cursor->paragraph_node = new_paragraph;
		cursor->text_node = NULL;
		cursor->character = 0;
		return new_paragraph;
	}

	TextNode* current_text = cursor->text_node;
	int position = cursor->character;
	int text_length = (int)strlen(current_text->data);

	if (position < 0)
		position = 0;

	if (position > text_length)
		position = text_length;

	TextNode* after = malloc(sizeof(TextNode));

	if (!after)
	{
		current->next = new_paragraph->next;

		if (new_paragraph->next)
			new_paragraph->next->prev = current;

		free(new_paragraph);
		return NULL;
	}

	after->data = NULL;
	after->resolved_style = NULL;
	after->next = NULL;
	after->prev = NULL;

	int after_length = text_length - position;

	after->data = malloc(after_length + 1);

	if (!after->data)
	{
		free(after);
		current->next = new_paragraph->next;

		if (new_paragraph->next)
			new_paragraph->next->prev = current;

		free(new_paragraph);
		return NULL;
	}

	memcpy(after->data, current_text->data + position, after_length);
	after->data[after_length] = '\0';

	after->resolved_style = malloc(sizeof(Style));

	if (!after->resolved_style)
	{
		free(after->data);
		free(after);

		current->next = new_paragraph->next;

		if (new_paragraph->next)
			new_paragraph->next->prev = current;

		free(new_paragraph);
		return NULL;
	}

	*after->resolved_style = *current_text->resolved_style;

	char* shortened = realloc(current_text->data, position + 1);

	if (shortened)
		current_text->data = shortened;

	current_text->data[position] = '\0';

	TextNode* following = current_text->next;

	new_paragraph->text = after;
	after->prev = NULL;
	after->next = following;

	if (following)
		following->prev = after;

	current_text->next = NULL;

	cursor->paragraph_node = new_paragraph;
	cursor->text_node = after;
	cursor->character = 0;

	return new_paragraph;
}

void print_text_list(TextNode* first)
{
	TextNode* text = first;

	while (text)
	{
		TextNode* next = text->next;

		print_style(text->resolved_style);
		printf("\n======================\n");
		printf("%s\n", text->data);

		text = next;
	}

	printf("\n======================\n");
}

void init_text_list(TextNode** first)
{
	*first = NULL;
}

void insert_text_node(TextNode** first, const char* data, Style* resolved_style, int index)
{
	TextNode* new_node = malloc(sizeof(TextNode));

	if (!new_node)
	{
		printf("Linked list allocation failed\n");
		return;
	}

	new_node->resolved_style = malloc(sizeof(Style));

	if (!new_node->resolved_style)
	{
		free(new_node);
		return;
	}

	if (resolved_style)
		*new_node->resolved_style = *resolved_style;
	else
		init_style(new_node->resolved_style);

	new_node->data = strdup(data);
	new_node->next = NULL;
	new_node->prev = NULL;

	if (*first == NULL || index == 0)
	{
		new_node->next = *first;

		if (*first)
			(*first)->prev = new_node;

		*first = new_node;
		return;
	}

	TextNode* text = *first;
	int counter = 0;

	while (text->next && counter < index)
	{
		text = text->next;
		counter++;
	}

	new_node->next = text->next;
	new_node->prev = text;

	if (text->next)
		text->next->prev = new_node;

	text->next = new_node;
}

TextNode* find_text_node(TextNode* first, int index)
{
	TextNode* text = first;

	for (int i = 0; text && i < index; i++)
		text = text->next;

	return text;
}

void delete_text_node(TextNode** first, int index)
{
	if (!first || !*first)
		return;

	TextNode* target = *first;

	for (int i = 0; target && i < index; i++)
		target = target->next;

	if (!target)
		return;

	if (target->prev)
		target->prev->next = target->next;
	else
		*first = target->next;

	if (target->next)
		target->next->prev = target->prev;

	free(target->data);
	free(target->resolved_style);
	free(target);
}

void free_text_list(TextNode* first)
{
	TextNode* text = first;

	while (text)
	{
		TextNode* next = text->next;

		free(text->data);
		free(text->resolved_style);
		free(text);

		text = next;
	}
}

void init_paragraph_list(ParagraphNode** first)
{
	*first = NULL;
}

void insert_paragraph_node(ParagraphNode** first, ParagraphNode* to_insert, int index)
{
	if (!to_insert)
		return;

	to_insert->prev = NULL;
	to_insert->next = NULL;

	if (*first == NULL || index == 0)
	{
		to_insert->next = *first;

		if (*first)
			(*first)->prev = to_insert;

		*first = to_insert;
		return;
	}

	ParagraphNode* paragraph = *first;
	int counter = 0;

	while (paragraph->next && counter < index - 1)
	{
		paragraph = paragraph->next;
		counter++;
	}

	to_insert->next = paragraph->next;
	to_insert->prev = paragraph;

	if (paragraph->next)
		paragraph->next->prev = to_insert;

	paragraph->next = to_insert;
}

ParagraphNode* find_paragraph_node(ParagraphNode* first, int index)
{
	ParagraphNode* paragraph = first;

	for (int i = 0; paragraph && i < index; i++)
		paragraph = paragraph->next;

	return paragraph;
}

void delete_paragraph_node(ParagraphNode** first, int index)
{
	if (!first || !*first)
		return;

	ParagraphNode* target = *first;

	for (int i = 0; target && i < index; i++)
		target = target->next;

	if (!target)
		return;

	if (target->prev)
		target->prev->next = target->next;
	else
		*first = target->next;

	if (target->next)
		target->next->prev = target->prev;

	free_text_list(target->text);
	free(target);
}

void free_paragraph_list(ParagraphNode* first)
{
	ParagraphNode* paragraph = first;

	while (paragraph)
	{
		ParagraphNode* next = paragraph->next;

		free_text_list(paragraph->text);
		free(paragraph);

		paragraph = next;
	}
}