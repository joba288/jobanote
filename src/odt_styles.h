#ifndef ODT_STYLES_H
#define ODT_STYLES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Style
{
	char name[32];
	char parent[32];

	bool bold;
	bool italic;
	bool underline;
	bool strikethrough;

	bool hasBold;
	bool hasItalic;
	bool hasUnderline;
	bool hasStrike;
	bool hasFontSize;
	bool hasFontName;
	bool hasColour;

	float fontSize;
	char font_name[32];
	char colour[16];

} Style;

typedef struct StyleDictionary
{
	size_t size;
	size_t capacity;
	Style* dict; // dynamic array

	Style default_paragraph;

} StyleDictionary;


void init_style_dictionary(StyleDictionary* dictionary);
void free_style_dictionary(StyleDictionary* dictionary);
void insert_style(Style* style, StyleDictionary* dictionary, char out[32]);
Style* find_style(char name[32], StyleDictionary* dictionary);
Style* index_style(int index, StyleDictionary* dictionary);
void delete_style(char name[32], StyleDictionary* dictionary);
void init_style(Style* style);
void print_style(const Style* style);
void print_style_dictionary(const StyleDictionary* dictionary);
bool style_cmp(const Style* a, const Style* b);

// Resolve style inheritance
Style resolve_style(const char* name, StyleDictionary* dictionary);
void overlay_style(Style* base, const Style* overlay);





#endif