#include "jobanote.h"

#include "odt.h"

Document current_doc;

Document* run_program()
{
	init_document(&current_doc);
	open_odt("resources/documents/test.odt");

	current_doc.cursor.paragraph_node = current_doc.paragraphs;
	current_doc.cursor.text_node = current_doc.paragraphs ? current_doc.paragraphs->text : NULL;
	current_doc.cursor.character = 0;

	return &current_doc;
}

void parse_styles(xmlDocPtr doc, StyleDictionary* dict)
{
	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);

	if (!ctx)
		return;

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

	// Bold
	if (weight)
	{
		style->bold = xmlStrcmp(weight, BAD_CAST "bold") == 0;
		style->hasBold = true;
	}

	// Italic
	if (italic)
	{
		style->italic = xmlStrcmp(italic, BAD_CAST "italic") == 0;
		style->hasItalic = true;
	}

	// Underline
	if (underline)
	{
		style->underline = xmlStrcmp(underline, BAD_CAST "none") != 0;

		style->hasUnderline = true;
	}

	// Strikethrough
	if (strike)
	{
		style->strikethrough = xmlStrcmp(strike, BAD_CAST "none") != 0;

		style->hasStrike = true;
	}

	// Font size
	if (size)
	{
		style->fontSize = (float)atof((char*)size);
		style->hasFontSize = true;
	}

	// Font name
	if (font)
	{
		strncpy(style->font_name, (char*)font, sizeof(style->font_name) - 1);

		style->font_name[sizeof(style->font_name) - 1] = '\0';
		style->hasFontName = true;
	}

	// Colour
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

				insert_text_node(&paragraph->text, (char*)value, &current_style, (*index)++);

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

			if (count < 1)
				continue;

			char* spaces = malloc((size_t)count + 1);

			if (spaces)
			{
				memset(spaces, ' ', count);
				spaces[count] = '\0';
				insert_text_node(&paragraph->text, spaces, &current_style, (*index)++);
				free(spaces);
			}
		}
		// tab
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "tab"))
		{
			insert_text_node(&paragraph->text, "\t", &current_style, (*index)++);
		}
		// line break
		else if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "line-break"))
		{
			insert_text_node(&paragraph->text, "\n", &current_style, (*index)++);
		}
	}
}

void parse_content(xmlDocPtr doc)
{
	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);

	if (!ctx)
		return;

	xmlXPathRegisterNs(ctx, BAD_CAST "office", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
	xmlXPathRegisterNs(ctx, BAD_CAST "text", BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0");

	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST "//text:p", ctx);

	if (result && result->nodesetval)
	{
		// loop each text:p node
		int count = result->nodesetval->nodeNr;
		for (int i = 0; i < count; i++)
		{
			ParagraphNode* paragraph_node = calloc(1, sizeof(ParagraphNode));

			if (!paragraph_node)
			{
				fprintf(stderr, "Failed to allocate paragraph node\\n");
				continue;
			}

			init_text_list(&paragraph_node->text);

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

			insert_paragraph_node(&current_doc.paragraphs, paragraph_node, i);

			//print_text_list(paragraph_node->text);

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
		fprintf(stderr, "Cannot open file\\n");
		return;
	}

	xmlDocPtr content_doc = NULL;
	xmlDocPtr styles_doc = NULL;

	// Load content.xml
	size_t content_size = 0;
	void* content_data = mz_zip_reader_extract_file_to_heap(&odt, "content.xml", &content_size, 0);

	if (!content_data)
	{
		fprintf(stderr, "Failed to extract content.xml\\n");
		goto cleanup;
	}

	content_doc = xmlReadMemory(content_data, (int)content_size, "content.xml", NULL, 0);
	mz_free(content_data);

	if (!content_doc)
	{
		fprintf(stderr, "Failed to parse content.xml\\n");
		goto cleanup;
	}

	// Load styles.xml
	size_t styles_size = 0;
	void* styles_data = mz_zip_reader_extract_file_to_heap(&odt, "styles.xml", &styles_size, 0);

	if (!styles_data)
	{
		fprintf(stderr, "Failed to extract styles.xml\\n");
		goto cleanup;
	}

	styles_doc = xmlReadMemory(styles_data, (int)styles_size, "styles.xml", NULL, 0);
	mz_free(styles_data);

	if (!styles_doc)
	{
		fprintf(stderr, "Failed to parse styles.xml\\n");
		goto cleanup;
	}

	// Parse
	parse_styles(styles_doc, &current_doc.dictionary);
	parse_styles(content_doc, &current_doc.dictionary);
	parse_content(content_doc);

	print_style_dictionary(&current_doc.dictionary);

cleanup:
	if (content_doc)
		xmlFreeDoc(content_doc);

	if (styles_doc)
		xmlFreeDoc(styles_doc);

	mz_zip_reader_end(&odt);
}

static xmlDocPtr create_content_xml(Document* doc, Style* export_styles, int style_count)
{
	xmlDocPtr xml_doc = xmlNewDoc(BAD_CAST "1.0");

	if (!xml_doc)
		return NULL;

	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "office:document-content");

	xmlDocSetRootElement(xml_doc, root);

	//Namespaces
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:office:1.0", BAD_CAST "office");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0", BAD_CAST "text");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:style:1.0", BAD_CAST "style");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", BAD_CAST "fo");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", BAD_CAST "svg");
	xmlNewNs(root, BAD_CAST "http://www.w3.org/1999/xlink", BAD_CAST "xlink");
	xmlNewProp(root, BAD_CAST "office:version", BAD_CAST "1.3");


	xmlNodePtr automatic_styles = xmlNewChild(root, NULL, BAD_CAST "office:automatic-styles", NULL);

	for (int i = 0; i < style_count; i++)
	{
		char style_name[32];

		snprintf(style_name, sizeof(style_name), "T%d", i + 1);

		xmlNodePtr style_node = xmlNewChild(automatic_styles, NULL, BAD_CAST "style:style", NULL);
		xmlNewProp(style_node, BAD_CAST "style:name", BAD_CAST style_name);
		xmlNewProp(style_node, BAD_CAST "style:family", BAD_CAST "text");

		xmlNodePtr text_properties = xmlNewChild(style_node, NULL, BAD_CAST "style:text-properties", NULL);
		add_style_properties(text_properties, &export_styles[i]);
	}

	xmlNodePtr body = xmlNewChild(root, NULL, BAD_CAST "office:body", NULL);

	xmlNodePtr office_text = xmlNewChild(body, NULL, BAD_CAST "office:text", NULL);

	for (ParagraphNode* paragraph = doc->paragraphs;paragraph;paragraph = paragraph->next)
	{
		xmlNodePtr p = xmlNewChild(office_text, NULL, BAD_CAST "text:p", NULL);

		for (TextNode* text = paragraph->text;
			text;
			text = text->next)
		{
			if (!text->data)
				continue;

			// if no style then default
			if (!text->resolved_style || style_count == 0)
			{
				xmlNodePtr text_node = xmlNewText(BAD_CAST text->data);

				xmlAddChild(p, text_node);

				continue;
			}

			int style_index = find_export_style(export_styles, style_count, text->resolved_style);

			// Couln't find style
			if (style_index < 0)
			{
				xmlNodePtr text_node = xmlNewText(BAD_CAST text->data);

				xmlAddChild(p, text_node);

				continue;
			}

			char style_name[32];

			snprintf(style_name, sizeof(style_name), "T%d", style_index + 1);

			xmlNodePtr span = xmlNewChild(p, NULL, BAD_CAST "text:span", NULL);
			xmlNewProp(span, BAD_CAST "text:style-name", BAD_CAST style_name);

			xmlNodePtr text_node = xmlNewText(BAD_CAST text->data);
			xmlAddChild(span, text_node);
		}
	}

	return xml_doc;
}

static xmlDocPtr create_styles_xml(void)
{
	xmlDocPtr xml_doc = xmlNewDoc(BAD_CAST "1.0");

	if (!xml_doc)
		return NULL;

	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "office:document-styles");

	xmlDocSetRootElement(xml_doc, root);

	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:office:1.0", BAD_CAST "office");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:style:1.0", BAD_CAST "style");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:text:1.0", BAD_CAST "text");
	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", BAD_CAST "fo");
	xmlNewProp(root, BAD_CAST "office:version", BAD_CAST "1.3");

	xmlNewChild(root, NULL, BAD_CAST "office:styles", NULL);

	xmlNewChild(root, NULL, BAD_CAST "office:automatic-styles", NULL);

	return xml_doc;
}

static void* xml_to_memory(xmlDocPtr doc, size_t* size)
{
	if (!doc || !size)
		return NULL;

	xmlChar* buffer = NULL;
	int buffer_size = 0;

	xmlDocDumpFormatMemoryEnc(doc, &buffer, &buffer_size, "UTF-8", 1);

	if (!buffer)
		return NULL;

	void* result = malloc((size_t)buffer_size);

	if (!result)
	{
		xmlFree(buffer);
		return NULL;
	}

	memcpy(result, buffer, (size_t)buffer_size);

	*size = (size_t)buffer_size;

	xmlFree(buffer);

	return result;
}

static xmlDocPtr create_manifest_xml(void)
{
	xmlDocPtr xml_doc = xmlNewDoc(BAD_CAST "1.0");

	if (!xml_doc)
		return NULL;

	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "manifest:manifest");

	xmlDocSetRootElement(xml_doc, root);

	xmlNewNs(root, BAD_CAST "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0", BAD_CAST "manifest");

	xmlNewProp(root, BAD_CAST "manifest:version", BAD_CAST "1.3");

	/*
	 * Root ODT MIME type
	 */
	xmlNodePtr file = xmlNewChild(root, NULL, BAD_CAST "manifest:file-entry", NULL);

	xmlNewProp(file, BAD_CAST "manifest:media-type", BAD_CAST "application/vnd.oasis.opendocument.text");

	xmlNewProp(file, BAD_CAST "manifest:full-path", BAD_CAST "/");

	/*
	 * content.xml
	 */
	file = xmlNewChild(root, NULL, BAD_CAST "manifest:file-entry", NULL);

	xmlNewProp(file, BAD_CAST "manifest:media-type", BAD_CAST "text/xml");

	xmlNewProp(file, BAD_CAST "manifest:full-path", BAD_CAST "content.xml");

	/*
	 * styles.xml
	 */
	file = xmlNewChild(root, NULL, BAD_CAST "manifest:file-entry", NULL);

	xmlNewProp(file, BAD_CAST "manifest:media-type", BAD_CAST "text/xml");

	xmlNewProp(file, BAD_CAST "manifest:full-path", BAD_CAST "styles.xml");

	return xml_doc;
}

void export_odt(Document* doc, const char* filepath)
{
	if (!doc || !filepath)
		return;

	Style* export_styles = NULL;
	int style_count = 0;
	int capacity = 16;

	export_styles = malloc(sizeof(Style) * capacity);

	if (!export_styles)
	{
		fprintf(stderr, "Failed to allocate export styles\\n");
		return;
	}

	// Count and create styles
	for (ParagraphNode* paragraph = doc->paragraphs; paragraph; paragraph = paragraph->next)
	{
		for (TextNode* text = paragraph->text; text; text = text->next)
		{
			if (!text->resolved_style)
				continue;

			if (find_export_style(export_styles, style_count, text->resolved_style) >= 0)
				continue;

			if (style_count >= capacity)
			{
				capacity *= 2;

				Style* temp = realloc(export_styles, sizeof(Style) * capacity);

				if (!temp)
				{
					fprintf(stderr, "Failed to resize export styles\\n");
					free(export_styles);
					return;
				}

				export_styles = temp;
			}

			export_styles[style_count++] = *text->resolved_style;
		}
	}

	// Create XML documents
	xmlDocPtr content_doc = create_content_xml(doc, export_styles, style_count);

	if (!content_doc)
	{
		fprintf(stderr, "Failed to create content.xml\\n");
		goto cleanup;
	}

	xmlDocPtr styles_doc = create_styles_xml();

	if (!styles_doc)
	{
		fprintf(stderr, "Failed to create styles.xml\\n");
		goto cleanup;
	}

	xmlDocPtr manifest_doc = create_manifest_xml();

	if (!manifest_doc)
	{
		fprintf(stderr, "Failed to create manifest.xml\\n");
		goto cleanup;
	}

	// XML to memory
	size_t content_size = 0;
	size_t styles_size = 0;
	size_t manifest_size = 0;

	void* content_data = xml_to_memory(content_doc, &content_size);
	void* styles_data = xml_to_memory(styles_doc, &styles_size);
	void* manifest_data = xml_to_memory(manifest_doc, &manifest_size);

	if (!content_data || !styles_data || !manifest_data)
	{
		fprintf(stderr, "Failed to serialise XML\\n");
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// ODT zip
	mz_zip_archive zip;
	memset(&zip, 0, sizeof(zip));

	if (!mz_zip_writer_init_file(&zip, filepath, 0))
	{
		fprintf(stderr, "Failed to create ODT file\\n");
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// mimetype must be first file of archive
	const char* mimetype = "application/vnd.oasis.opendocument.text";

	if (!mz_zip_writer_add_mem(
		&zip,
		"mimetype",
		mimetype,
		strlen(mimetype),
		MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE))
	{
		fprintf(stderr, "Failed to add mimetype\\n");
		mz_zip_writer_end(&zip);
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// content.xml
	if (!mz_zip_writer_add_mem(&zip, "content.xml", content_data, content_size, MZ_DEFAULT_COMPRESSION))
	{
		fprintf(stderr, "Failed to add content.xml\\n");
		mz_zip_writer_end(&zip);
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// styles.xml
	if (!mz_zip_writer_add_mem(&zip, "styles.xml", styles_data, styles_size, MZ_DEFAULT_COMPRESSION))
	{
		fprintf(stderr, "Failed to add styles.xml\\n");
		mz_zip_writer_end(&zip);
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// manifest.xml
	if (!mz_zip_writer_add_mem(&zip, "META-INF/manifest.xml", manifest_data, manifest_size, MZ_DEFAULT_COMPRESSION))
	{
		fprintf(stderr, "Failed to add manifest.xml\\n");
		mz_zip_writer_end(&zip);
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	// Finish zipping
	if (!mz_zip_writer_finalize_archive(&zip))
	{
		fprintf(stderr, "Failed to finalize ODT archive\\n");
		mz_zip_writer_end(&zip);
		free(content_data);
		free(styles_data);
		free(manifest_data);
		goto cleanup;
	}

	mz_zip_writer_end(&zip);

	printf("Successfully exported ODT: %s\\n", filepath);

	free(content_data);
	free(styles_data);
	free(manifest_data);

cleanup:
	if (content_doc)
		xmlFreeDoc(content_doc);

	if (styles_doc)
		xmlFreeDoc(styles_doc);

	if (manifest_doc)
		xmlFreeDoc(manifest_doc);

	free(export_styles);
}

void add_style_properties(xmlNodePtr text_properties, const Style* style)
{
	if (!text_properties || !style)
		return;

	if (style->bold)
	{
		xmlNewProp(text_properties, BAD_CAST "fo:font-weight", BAD_CAST "bold");
	}

	if (style->italic)
	{
		xmlNewProp(text_properties, BAD_CAST "fo:font-style", BAD_CAST "italic");
	}

	if (style->underline)
	{
		xmlNewProp(text_properties, BAD_CAST "style:text-underline-style", BAD_CAST "solid");

		xmlNewProp(text_properties, BAD_CAST "style:text-underline-type", BAD_CAST "single");
	}

	if (style->strikethrough)
	{
		xmlNewProp(text_properties, BAD_CAST "style:text-line-through-style", BAD_CAST "solid");
	}

	if (style->fontSize > 0.0f)
	{
		char size[32];

		snprintf(size, sizeof(size), "%.2fpt", style->fontSize);

		xmlNewProp(text_properties, BAD_CAST "fo:font-size", BAD_CAST size);
	}

	if (strcmp(style->font_name, "NULL") != 0 &&
		style->font_name[0] != '\0')
	{
		xmlNewProp(text_properties, BAD_CAST "style:font-name", BAD_CAST style->font_name);

		xmlNewProp(text_properties, BAD_CAST "fo:font-family", BAD_CAST style->font_name);
	}


	if (style->colour[0] != '\0')
	{
		char colour[32];

		if (style->colour[0] == '#')
		{
			snprintf(colour, sizeof(colour), "%s", style->colour);
		}
		else
		{
			snprintf(colour, sizeof(colour), "#%s", style->colour);
		}

		xmlNewProp(text_properties, BAD_CAST "fo:color", BAD_CAST colour);
	}
}

static int find_export_style(const Style* styles, int style_count, const Style* style)
{
	for (int i = 0; i < style_count; i++)
	{
		if (style_cmp(&styles[i], style))
			return i;
	}

	return -1;
}

