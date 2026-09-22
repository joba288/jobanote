#ifndef JOBANOTE_H
#define JOBANOTE_H

#include <stdlib.h>
#include <stdbool.h>
#include "odt_styles.h"
#include "libxml/parser.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

// -- Types --


// Doubly-linked list style node for text spans
typedef struct TextNode
{
	char* data;
	Style* resolved_style;
	struct TextNode* next;
	struct TextNode* prev;

} TextNode;

// Doubly-linked list style node for paragraph spans (spans of text spans)
typedef struct ParagraphNode
{
	TextNode* text;
	struct ParagraphNode* next;
	struct ParagraphNode* prev;
} ParagraphNode;

// Stores position of cursor
typedef struct
{
	ParagraphNode* paragraph_node;
	TextNode* text_node;
	int character; // TODO utf8 byte offset
} Cursor;

typedef struct Document
{
	ParagraphNode* paragraphs;
	StyleDictionary dictionary;
	Cursor cursor;
	bool selecting;
	Cursor selection_start;

} Document;


// -- Data Structure Functions --
// ------------------------------

// Text Nodes

void init_text_list(TextNode** first);
void insert_text_node(TextNode** first, const char* data, Style* resolved_style, int index);
TextNode* find_text_node(TextNode* first, int index);
void delete_text_node(TextNode** first, int index);
void free_text_list(TextNode* first);
void print_text_list(TextNode* first);

// Paragraphs
void init_paragraph_list(ParagraphNode** first);
void insert_paragraph_node(ParagraphNode** first, ParagraphNode* to_insert, int index);
ParagraphNode* find_paragraph_node(ParagraphNode* first, int index);
void delete_paragraph_node(ParagraphNode** first, int index);
void free_paragraph_list(ParagraphNode* first);

// --- Document --
// ---------------

void init_document(Document* doc);
void free_document(Document* doc);
void add_style_properties(xmlNodePtr text_properties, const Style* style);

// -- Typing / Node Traversal --
// -----------------------------
void insert_character(Document* doc, int codepoint);
void delete_character(Document* doc);
void delete_character_forward(Document* doc);

void increment_cursor(Document* doc);
void decrement_cursor(Document* doc);
bool cursor_equal(Cursor* a, Cursor* b);
bool text_node_in_selection(Document* doc,TextNode* target,Cursor start,Cursor end);
bool has_selection(Document* doc);
void delete_selection(Document* doc);


// -- Utility --
// -------------

void merge_paragraph_nodes(ParagraphNode* pDest, ParagraphNode* pMerge);
int utf8_char_len(const char* str);
int utf8_prev_char_len(const char* str, int byte_pos);
void delete_character_from_node(TextNode* node, int* cursor_position);
TextNode* last_text_node(TextNode* first);
TextNode* split_text_node(TextNode* node, int position);
void toggle_bold_selection(Document* doc);
ParagraphNode* insert_paragraph_node_at_cursor(Document* doc);

#endif