#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "jobanote.h"
#include <stdlib.h>
#include <string.h>

static ParagraphNode* test_create_paragraph(void)
{
	ParagraphNode* paragraph = calloc(1, sizeof(ParagraphNode));
	return paragraph;
}

static Document test_create_document(void)
{
	Document doc;
	init_document(&doc);

	ParagraphNode* paragraph = test_create_paragraph();
	insert_paragraph_node(&doc.paragraphs, paragraph, 0);

	doc.cursor.paragraph_node = paragraph;
	doc.cursor.text_node = NULL;
	doc.cursor.character = 0;

	doc.selection_start = doc.cursor;
	doc.selecting = false;

	return doc;
}

static TextNode* test_add_text(ParagraphNode* paragraph, const char* text)
{
	int index = 0;
	TextNode* node = paragraph->text;

	while (node)
	{
		index++;
		node = node->next;
	}

	insert_text_node(&paragraph->text, text, NULL, index);
	return find_text_node(paragraph->text, index);
}

static ParagraphNode* test_add_paragraph(Document* doc)
{
	ParagraphNode* paragraph = test_create_paragraph();

	int index = 0;
	ParagraphNode* current = doc->paragraphs;

	while (current)
	{
		index++;
		current = current->next;
	}

	insert_paragraph_node(&doc->paragraphs, paragraph, index);
	return paragraph;
}

static void test_set_cursor(Document* doc, ParagraphNode* paragraph, TextNode* text, int character)
{
	doc->cursor.paragraph_node = paragraph;
	doc->cursor.text_node = text;
	doc->cursor.character = character;
}

static void test_set_selection(Document* doc, Cursor start, Cursor end)
{
	doc->selection_start = start;
	doc->cursor = end;
	doc->selecting = true;
}

static void test_free_document(Document* doc)
{
	free_document(doc);
}

#endif
