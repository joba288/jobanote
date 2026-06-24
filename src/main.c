#include "miniz.h"
#include <stdlib.h>
#include "libxml/parser.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdbool.h>
#include "odt_styles.h"
#include "jobanote.h"

Document current_doc;

void open_odt(const char* filepath);
void parse_styles(xmlDocPtr doc, StyleDictionary* dict);

void parse_text_properties(xmlNode* node, Style* style);

void parse_content(xmlDocPtr doc);

void parse_inline_node(xmlNode* node, ParagraphNode* paragraph, Style current_style, int* index);


void overlay_style(Style* base, const Style* overlay)
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


int main(void)
{
 
	init_document(&current_doc);

	open_odt("resources/documents/test.odt");

	free_document(&current_doc);
	return 0;
}

void parse_styles(xmlDocPtr doc, StyleDictionary* dict)
{
	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);

	xmlXPathRegisterNs(ctx, BAD_CAST "office", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
	xmlXPathRegisterNs(ctx, BAD_CAST "style", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
	xmlXPathRegisterNs(ctx, BAD_CAST "fo", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");

	// Default Styles

	xmlXPathObjectPtr defaults = xmlXPathEvalExpression(BAD_CAST "//style:default-style[@style:family='paragraph']", ctx);

	if (defaults && defaults->nodesetval)
	{
		for (int i = 0; i < defaults->nodesetval->nodeNr; i++)
		{
			xmlNode* node = defaults->nodesetval->nodeTab[i];

			Style style;
			init_style(&style);
			strcpy(style.name, "default-style");

			for (xmlNode* child = node->children; child; child = child->next)
			{
				if (child->type != XML_ELEMENT_NODE)
					continue;

				if (xmlStrcmp(child->name, BAD_CAST "text-properties") == 0)
				{
					parse_text_properties(child, &style);
				}


			}

		
			dict->default_paragraph = style;

			printf("Found default style: %s\n", style.name);
		}
	}

	if (defaults)
		xmlXPathFreeObject(defaults);



	// Normal styles

	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST "//style:style", ctx);

	if (result && result->nodesetval)
	{
		int count = result->nodesetval->nodeNr;

		for (int i = 0; i < count; i++)
		{
			xmlNode* node = result->nodesetval->nodeTab[i];

			Style style;
			init_style(&style);

			xmlChar* name = xmlGetProp(node, BAD_CAST "name");

			xmlChar* parent = xmlGetProp(node, BAD_CAST "parent-style-name");

			if (name)
			{
				strncpy(style.name, (char*)name, sizeof(style.name) - 1);
				style.name[sizeof(style.name) - 1] = '\0';
				xmlFree(name);
			}

			if (parent)
			{
				strncpy(style.parent, (char*)parent, sizeof(style.parent) - 1);
				style.parent[sizeof(style.parent) - 1] = '\0';
				xmlFree(parent);
			}

			for (xmlNode* child = node->children; child; child = child->next)
			{
				if (child->type != XML_ELEMENT_NODE)
					continue;

				if (xmlStrcmp(child->name, BAD_CAST "text-properties") == 0)
				{
					parse_text_properties(child, &style);
				}
			}

			insert_style(&style, dict, style.name);
			printf("Found style: %s\n", style.name);
		}
	}

	if (result)
		xmlXPathFreeObject(result);

	xmlXPathFreeContext(ctx);

}

void parse_text_properties(xmlNode* node, Style* style)
{
	xmlChar* weight = xmlGetProp(node, BAD_CAST "font-weight");
	xmlChar* italic = xmlGetProp(node, BAD_CAST "font-style");
	xmlChar* underline = xmlGetProp(node, BAD_CAST "text-underline-style");
	xmlChar* strike = xmlGetProp(node, BAD_CAST "text-line-through-style");
	xmlChar* size = xmlGetProp(node, BAD_CAST "font-size");
	xmlChar* font = xmlGetProp(node, BAD_CAST "font-name");
	xmlChar* colour = xmlGetProp(node, BAD_CAST "color");

	if (weight)
	{
		style->bold = xmlStrcmp(weight, BAD_CAST "bold") == 0;
		style->hasBold = true;
	}

	if (italic)
	{
		style->italic = xmlStrcmp(italic, BAD_CAST "italic") == 0;
		style->hasItalic = true;
	}

	if (size)
	{
		style->fontSize = (float)atof((char*)size);
		style->hasFontSize = true;
	}

	if (font)
	{
		strncpy(style->font_name, (char*)font, sizeof(style->font_name) - 1);
		style->font_name[sizeof(style->font_name) - 1] = '\0';
		style->hasFontName = true;
	}

	if (colour)
	{
		strncpy(style->colour, (char*)colour, sizeof(style->colour) - 1);
		style->colour[sizeof(style->colour) - 1] = '\0';
		style->hasColour = true;
	}

	xmlFree(weight);
	xmlFree(italic);
	xmlFree(underline);
	xmlFree(strike);
	xmlFree(size);
	xmlFree(font);
	xmlFree(colour);
}

void parse_inline_node(xmlNode* node, ParagraphNode* paragraph, Style current_style, int* index)
{

	for (xmlNode* child = node; child; child = child->next)
	{
		
		// Plain text
		
		if (child->type == XML_TEXT_NODE)
		{
			xmlChar* value = xmlNodeGetContent(child);

			if (value && xmlStrlen(value) > 0)
			{

				insert_text_node(paragraph->text, (char*)value, &current_style, (*index)++);

				xmlFree(value);
			}
		}


		// Span
		
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "span"))
		{
			Style span_style = current_style;

			xmlChar* style_name = xmlGetNsProp(child, BAD_CAST "style-name", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0");

			if (style_name)
			{
				Style resolved = resolve_style((char*)style_name, &current_doc.dictionary);
				printf("span style = %s\n", style_name);
				
				//Overlay span style on current style
				overlay_style(&span_style, &resolved);
				xmlFree(style_name);
			}

			parse_inline_node(child->children, paragraph, span_style, index); // recursive
		}
		// space
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "s"))
		{
			int count = 1;

			xmlChar* c = xmlGetProp(child, BAD_CAST "c");

			if (c)
			{
				count = atoi((char*)c);
				xmlFree(c);
			}

			char spaces[256];

			memset(spaces, ' ', count);
			spaces[count] = '\0';

			insert_text_node(paragraph->text, spaces,&current_style,(*index)++);
		}
		// tab
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "tab"))
		{
			insert_text_node(paragraph->text,"\t",&current_style, (*index)++);
		}
		// line break
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "line-break"))
		{
			insert_text_node(paragraph->text,"\n", &current_style, (*index)++);
		}
	}
}



void parse_content(xmlDocPtr doc)
{
	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathRegisterNs(ctx, BAD_CAST "office", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
	xmlXPathRegisterNs(ctx, BAD_CAST "text", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0");

	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST "//text:p", ctx);

	if (result && result->nodesetval)
	{
		// loop each text:p node
		int count = result->nodesetval->nodeNr;
		for (int i = 0; i < count; i++)
		{
			ParagraphNode* paragraph_node = (ParagraphNode*)malloc(sizeof(ParagraphNode));
			paragraph_node->text = NULL;
			init_text_list(&paragraph_node->text);
			paragraph_node->next = NULL;

			xmlNode* node = result->nodesetval->nodeTab[i];

			Style paragraph_style;
			init_style(&paragraph_style);

			xmlChar* style_name = xmlGetNsProp(node, BAD_CAST "style-name", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0");

			if (style_name)
			{
				printf("STYLE = %s\n", (char*)style_name);
				paragraph_style = resolve_style((char*)style_name, &current_doc.dictionary);
				xmlFree(style_name);
			}

			int text_index = 0;
			parse_inline_node(node->children, paragraph_node, paragraph_style, &text_index);

			insert_paragraph_node(current_doc.paragraphs, &paragraph_node, i);

			print_text_list(paragraph_node->text);

		}

	}

	if (result)
		xmlXPathFreeObject(result);
	xmlXPathFreeContext(ctx);
}

void open_odt(const char* filepath)
{
	// TODO: Ensure filepath is an odt file 

	mz_zip_archive odt;
	memset(&odt, 0, sizeof(odt));

	if (!mz_zip_reader_init_file(&odt, filepath, 0))
	{
		printf("Cannot open file\n");
		return;
	}

	xmlDocPtr content_doc = NULL;
	xmlDocPtr styles_doc = NULL;

	// Load content.xml
	size_t content_size;
	void* content_data = mz_zip_reader_extract_file_to_heap(&odt, "content.xml", &content_size, 0);

	if (content_data)
	{
		content_doc = xmlReadMemory(content_data, (int)content_size, "content.xml", NULL, 0);
		mz_free(content_data);

		if (!content_doc)
		{
			fprintf(stderr, "Parse failed\n");
			mz_zip_reader_end(&odt);
			return;
		}

	}

	// Load styles.xml
	size_t styles_size;
	void* styles_data = mz_zip_reader_extract_file_to_heap(&odt, "styles.xml", &styles_size, 0);

	if (styles_data)
	{
		styles_doc = xmlReadMemory(styles_data, (int)styles_size, "styles.xml", NULL, 0);

		mz_free(styles_data);

		if (!styles_doc)
		{
			fprintf(stderr, "Failed to parse styles.xml\n");
			mz_zip_reader_end(&odt);
			return;
		}
	}

	// Parse 

	if (styles_doc)
	{
		parse_styles(styles_doc, &current_doc.dictionary);
	}

	if (content_doc)
	{
		parse_styles(content_doc, &current_doc.dictionary);
		parse_content(content_doc);
	}

	print_style_dictionary(&current_doc.dictionary);

	// Cleanup

	if (content_doc)
		xmlFreeDoc(content_doc);

	if (styles_doc)
		xmlFreeDoc(styles_doc);

	xmlCleanupParser();
	mz_zip_reader_end(&odt);
}