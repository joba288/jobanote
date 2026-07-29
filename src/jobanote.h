#ifndef JOBANOTE_H
#define JOBANOTE_H

#include "miniz.h"
#include <stdlib.h>
#include "libxml/parser.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdbool.h>
#include "odt_styles.h"



typedef struct TextNode
{
	char* data;
	Style* resolved_style;
	struct TextNode* next;

} TextNode;

typedef struct ParagraphNode
{
	TextNode* text;
	struct ParagraphNode* next;
} ParagraphNode;

typedef struct Document
{
	ParagraphNode* paragraphs;
	StyleDictionary dictionary;
} Document;

void init_text_list(TextNode** first);
void insert_text_node(TextNode* first, const char* data, Style* resolved_style, int index);
TextNode* find_text_node(TextNode* first, int index);
void delete_text_node(TextNode** first, int index);
void free_text_list(TextNode* first);
void print_text_list(TextNode* first);

// Paragraph
void init_paragraph_list(ParagraphNode** first);
void insert_paragraph_node(ParagraphNode* first, ParagraphNode** to_insert, int index);
ParagraphNode* find_paragraph_node(ParagraphNode* first, int index);
void delete_paragraph_node(ParagraphNode** first, int index);
void free_paragraph_list(ParagraphNode* first);
// Styles

// Doc
void init_document(Document* doc);
void free_document(Document* doc);



inline void init_document(Document* doc)
{
	doc->paragraphs = NULL;
	init_paragraph_list(&doc->paragraphs);
	init_style_dictionary(&doc->dictionary);
}

inline void free_document(Document* doc)
{
	free_paragraph_list(doc->paragraphs);
	free_style_dictionary(&doc->dictionary);
}




//



// Lists
//  text

inline void print_text_list(TextNode* first)
{

	TextNode* temp = first;

	while (temp != NULL)
	{

		TextNode* next = temp->next;
		print_style(temp->resolved_style);
		printf("\n======================\n");
		printf("%s\n", temp->data);
		temp = next;
	}
	printf("\n======================\n");
}

inline void init_text_list(TextNode** first)
{
	*first = malloc(sizeof(TextNode));

	if (!*first)
	{
		printf("Linked list allocation failed\n");
		return;
	}

	(*first)->data = NULL;
	(*first)->resolved_style = malloc(sizeof(Style));
	if (!(*first)->resolved_style)
	{
		printf("Style allocation failed\n");
		return;
	}

	init_style((*first)->resolved_style);
	(*first)->next = NULL;
}

inline void insert_text_node(TextNode* first, const char* data, Style* resolved_style, int index)
{
	TextNode* new_node = (TextNode*)malloc(sizeof(TextNode));

	if (!new_node)
	{
		printf("Linked list allocation failed\n");
		return;
	}

	// Allocate resolved style
	new_node->resolved_style = malloc(sizeof(Style));

	if (!new_node->resolved_style)
	{
		free(new_node->data);
		free(new_node);
		return;
	}

	if (resolved_style)
		*new_node->resolved_style = *resolved_style;
	else
		init_style(new_node->resolved_style);

	//
	new_node->data = strdup(data);
	new_node->next = NULL;

	TextNode* temp = first;
	int counter = 0;

	while (temp->next != NULL && counter < index)
	{
		temp = temp->next;
		counter++;
	}

	new_node->next = temp->next;
	temp->next = new_node;
}

inline TextNode* find_text_node(TextNode* first, int index)
{
	TextNode* temp = first;

	for (int i = 0; temp && i < index; i++)
		temp = temp->next;

	return temp;
}

inline void delete_text_node(TextNode** first, int index)
{
	if (!first || !*first)
		return;

	TextNode* temp = *first;

	if (index == 0)
	{
		*first = temp->next;
		free(temp->data);
		free(temp->resolved_style);
		free(temp);
		return;
	}

	for (int i = 0; temp && i < index - 1; i++)
		temp = temp->next;

	if (!temp || !temp->next)
		return;

	TextNode* target = temp->next;

	temp->next = target->next;
	free(target->data);
	free(target->resolved_style);
	free(target);
}

inline void free_text_list(TextNode* first)
{
	TextNode* temp = first;

	while (temp != NULL)
	{
		TextNode* next = temp->next;
		free(temp->data);
		free(temp->resolved_style);
		free(temp);
		temp = next;
	}
}

inline void init_paragraph_list(ParagraphNode** first)
{
	*first = malloc(sizeof(ParagraphNode));

	if (!*first)
	{
		printf("Linked list allocation failed\n");
		return;
	}

	(*first)->text = NULL;
	init_text_list(&(*first)->text);
	(*first)->next = NULL;
}

inline void insert_paragraph_node(ParagraphNode* first, ParagraphNode** to_insert, int index)
{
	ParagraphNode* temp = first;
	int counter = 0;

	while (temp->next != NULL && counter < index)
	{
		temp = temp->next;
		counter++;
	}

	(*to_insert)->next = temp->next;
	temp->next = *to_insert;


}

inline ParagraphNode* find_paragraph_node(ParagraphNode* first, int index)
{
	return NULL;
}

inline void delete_paragraph_node(ParagraphNode** first, int index)
{
}

inline void free_paragraph_list(ParagraphNode* first)
{
	ParagraphNode* temp = first;

	while (temp != NULL)
	{
		ParagraphNode* next = temp->next;
		free_text_list(temp->text);
		free(temp);
		temp = next;
	}
}

//--------------------------------------------------------------------

void open_odt(const char* filepath);
void parse_styles(xmlDocPtr doc, StyleDictionary* dict);
void parse_text_properties(xmlNode* node, Style* style);
void parse_content(xmlDocPtr doc);
void parse_inline_node(xmlNode* node, ParagraphNode* paragraph, Style current_style, int* index);


inline void overlay_style(Style* base, const Style* overlay)
{
	if (overlay->hasBold)
		base->bold = overlay->bold;

	if (overlay->hasItalic)
		base->italic = overlay->italic;

	if (overlay->hasUnderline)
		base->underline = overlay->underline;

	if (overlay->hasStrike)
		base->strikethrough = overlay->strikethrough;

	if (overlay->hasFontSize)
		base->fontSize = overlay->fontSize;

	if (overlay->hasFontName)
		strcpy(base->font_name, overlay->font_name);

	if (overlay->hasColour)
		strcpy(base->colour, overlay->colour);
}



// exporting to odt
void export_odt(Document* doc, const char* filepath);
void run_program();

#endif