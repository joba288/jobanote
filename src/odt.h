#ifndef ODT_H
#define ODT_H

#include "miniz.h"
#include <stdlib.h>
#include "libxml/parser.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdbool.h>
#include "odt_styles.h"


void open_odt(const char* filepath);
void export_odt(Document* doc, const char* filepath);


void parse_styles(xmlDocPtr doc, StyleDictionary* dict);
void parse_text_properties(xmlNode* node, Style* style);
void parse_content(xmlDocPtr doc);
void parse_inline_node(xmlNode* node, ParagraphNode* paragraph, Style current_style, int* index);

Document* run_program();

#endif ODT_H