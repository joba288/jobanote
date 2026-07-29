#ifndef ODT_STYLES_H
#define ODT_STYLES_H

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

Style resolve_style(const char* name, StyleDictionary* dict);


inline Style resolve_style(const char* name, StyleDictionary* dictionary)
{
	// resolve styles on use of a style

	Style resolved;
	init_style(&resolved);

	Style* style = find_style(name, dictionary);

	if (!style)
		return resolved;

	resolved = *style;

	if (strcmp(style->parent, "NULL") != 0)
	{
		Style parent = resolve_style(style->parent, dictionary);

		if (!resolved.hasBold)
			resolved.bold = parent.bold;

		if (!resolved.hasItalic)
			resolved.italic = parent.italic;

		if (!resolved.hasUnderline)
			resolved.underline = parent.underline;

		if (!resolved.hasStrike)
			resolved.strikethrough = parent.strikethrough;

		if (!resolved.hasFontSize)
			resolved.fontSize = parent.fontSize;

		if (!resolved.hasFontName)
			strcpy(resolved.font_name, parent.font_name);

		if (!resolved.hasColour)
			strcpy(resolved.colour, parent.colour);
	}

	if (!resolved.hasBold && dictionary->default_paragraph.hasBold)
		resolved.bold = dictionary->default_paragraph.bold;

	if (!resolved.hasItalic && dictionary->default_paragraph.hasItalic)
		resolved.italic = dictionary->default_paragraph.italic;

	if (!resolved.hasUnderline && dictionary->default_paragraph.hasUnderline)
		resolved.underline = dictionary->default_paragraph.underline;

	if (!resolved.hasStrike && dictionary->default_paragraph.hasStrike)
		resolved.strikethrough = dictionary->default_paragraph.strikethrough;

	if (!resolved.hasFontSize && dictionary->default_paragraph.hasFontSize)
		resolved.fontSize = dictionary->default_paragraph.fontSize;

	if (!resolved.hasFontName && dictionary->default_paragraph.hasFontName)
		strcpy(resolved.font_name, dictionary->default_paragraph.font_name);

	if (!resolved.hasColour && dictionary->default_paragraph.hasColour)
		strcpy(resolved.colour, dictionary->default_paragraph.colour);

	return resolved;
}

// Styles
inline void init_style_dictionary(StyleDictionary* dictionary)
{
	dictionary->size = 0;
	dictionary->capacity = 4;
	dictionary->dict = malloc(dictionary->capacity * sizeof(Style));

	if (dictionary->dict == NULL)
	{
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
}

inline void free_style_dictionary(StyleDictionary* dictionary)
{
	free(dictionary->dict);
	dictionary->dict = NULL;
	dictionary->size = 0;
	dictionary->capacity = 0;
}

inline void insert_style(Style* style, StyleDictionary* dictionary, char out[32])
{
	//resize array

	if (dictionary->size == dictionary->capacity)
	{
		dictionary->capacity *= 2;

		Style* temp = realloc(dictionary->dict, dictionary->capacity * sizeof(Style));
		if (temp == NULL) {
			fprintf(stderr, "Memory reallocation failed\n");
			free(dictionary->dict);
			exit(EXIT_FAILURE);
		}

		dictionary->dict = temp;
	}

	if (strcmp(style->name, "NULL") == 0)
	{
		// look into what odt naming actually means

		char str[32];
		char name[32];
		itoa(dictionary->size + 1, str, 10);
		snprintf(name, sizeof(name), "T%s", str);
		strcpy(style->name, name);
	}

	// resolve parentage

	dictionary->dict[dictionary->size++] = *style;
}
inline Style* find_style(char name[32], StyleDictionary* dictionary)
{
	if (!dictionary || !name)
		return NULL;

	for (int i = 0; i < dictionary->size; i++)
	{
		if (strcmp(name, dictionary->dict[i].name) == 0)
		{
			return &dictionary->dict[i];
		}
	}
	return NULL;
}



inline Style* index_style(int index, StyleDictionary* dictionary) {}
inline void delete_style(char name[32], StyleDictionary* dictionary) {}


inline void init_style(Style* style)
{
	memset(style, 0, sizeof(Style));
	strcpy(style->name, "NULL");
	strcpy(style->parent, "NULL");
	style->bold = false;
	style->italic = false;
	style->underline = false;
	style->strikethrough = false;

	style->fontSize = 0;
	strcpy(style->font_name, "NULL");
	strcpy(style->colour, "ffffff");

	style->hasBold = false;
	style->hasItalic = false;
	style->hasUnderline = false;
	style->hasStrike = false;
	style->hasFontSize = false;
	style->hasFontName = false;
	style->hasColour = false;
}
inline void print_style(const Style* style)
{
	if (!style)
		return;

	printf("Style: %s\n", style->name);
	printf("  Parent       : %s\n", style->parent);
	printf("  Bold         : %s\n", style->bold ? "true" : "false");
	printf("  Italic       : %s\n", style->italic ? "true" : "false");
	printf("  Underline    : %s\n", style->underline ? "true" : "false");
	printf("  Strike       : %s\n", style->strikethrough ? "true" : "false");
	printf("  Font Size    : %.2f\n", style->fontSize);
	printf("  Font Name    : %s\n", style->font_name);
	printf("  Colour       : %s\n", style->colour);
	printf("\n");
}
inline void print_style_dictionary(const StyleDictionary* dictionary)
{
	if (!dictionary)
		return;

	printf("\n========================================\n");
	printf("STYLE DICTIONARY\n");
	printf("Size     : %zu\n", dictionary->size);
	printf("Capacity : %zu\n", dictionary->capacity);
	printf("========================================\n\n");

	for (size_t i = 0; i < dictionary->size; i++)
	{
		printf("[%zu]\n", i);
		print_style(&dictionary->dict[i]);
	}

	printf("========================================\n");
}
inline bool style_cmp(const Style* a, const Style* b)
{
	if (!a || !b)
		return 0;

	return
		a->bold == b->bold &&
		a->italic == b->italic &&
		a->underline == b->underline &&
		a->strikethrough == b->strikethrough &&
		a->fontSize == b->fontSize &&
		strcmp(a->font_name, b->font_name) == 0 &&
		strcmp(a->colour, b->colour) == 0;
}


#endif