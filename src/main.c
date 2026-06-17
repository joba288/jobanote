#include "miniz.h"
#include <stdlib.h>
#include "libxml/parser.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdbool.h>


typedef struct Style
{
	char name[32];
	char parent[32];
	
	bool bold;
	bool italic;
	bool underline;
	
	float fontSize;
	char fontName[32];
	char colour[16];

} Style;

typedef struct TextNode
{
	char* data;
	int attr_key;
	struct TextNode* next;

} TextNode;

typedef struct ParagraphNode
{
	TextNode* text;
	struct Paragraph* next;
} ParagraphNode;

typedef struct Document
{
	ParagraphNode* paragraphs;
} Document;

void init_text_list(TextNode** first);
void insert_text_node(TextNode* first, const char* data, int attr_key, int index);
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

Document current_doc;

void open_odt(const char* filepath);

int main(void)
{
 
	init_document(&current_doc);

	open_odt("resources/documents/test.odt");
	free_document(&current_doc);
	return 0;
}

void init_document(Document* doc)
{
	doc->paragraphs = NULL;
	init_paragraph_list(&doc->paragraphs);
}

void free_document(Document* doc)
{
	free_paragraph_list(doc->paragraphs);
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

	size_t size;
	void* data = mz_zip_reader_extract_file_to_heap(&odt, "content.xml", &size, 0);

	if (data)
	{
		xmlDocPtr contentDoc = xmlReadMemory(data, (int)size, "content.xml", NULL, 0);

		if (!contentDoc)
		{
			fprintf(stderr, "Parse failed\n");
			mz_free(data);
		}

		// Use data

		xmlXPathContextPtr ctx = xmlXPathNewContext(contentDoc);
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

				int paragraph_child_index = 0;
				for (xmlNode* child = node->children; child != NULL; child = child->next)
				{
					xmlChar* value = xmlNodeGetContent(child);
					//printf("%s = %s\n", (char*)child->name, value);

					insert_text_node(paragraph_node->text, (char*)value, 0, paragraph_child_index);
					paragraph_child_index++;

					xmlFree(value);
					
				}


				
				print_text_list(paragraph_node->text);

				insert_paragraph_node(current_doc.paragraphs, &paragraph_node, i);

			}

		}

		xmlXPathFreeObject(result);
		xmlXPathFreeContext(ctx);
	

		xmlFreeDoc(contentDoc);
		xmlCleanupParser();
		mz_free(data);
	}
	
	mz_zip_reader_end(&odt);
}






// Lists
//  text

void print_text_list(TextNode* first)
{
	TextNode* temp = first;

	while (temp != NULL)
	{
		TextNode* next = temp->next;
		printf("%s\n", temp->data);
		temp = next;
	}
	printf("======================\n");
}

void init_text_list(TextNode** first)
{
	*first = malloc(sizeof(TextNode));

	if (!*first)
	{
		printf("Linked list allocation failed\n");
		return;
	}
	
	(*first)->data = NULL;
	(*first)->attr_key = 0;
	(*first)->next = NULL;
}

void insert_text_node(TextNode* first, const char* data, int attr_key, int index)
{
	TextNode* new_node = (TextNode*)malloc(sizeof(TextNode));

	if (!new_node)
	{
		printf("Linked list allocation failed\n");
		return;
	}

	new_node->attr_key = attr_key;
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

TextNode* find_text_node(TextNode* first, int index)
{
	TextNode* temp = first;

	for (int i = 0; temp && i < index; i++)
		temp = temp->next;

	return temp;
}

void delete_text_node(TextNode** first, int index)
{
	if (!first || !*first)
		return;

	TextNode* temp = *first;

	if (index == 0)
	{
		*first = temp->next;
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
	free(target);
}

void free_text_list(TextNode* first)
{
	TextNode* temp = first;

	while (temp != NULL)
	{
		TextNode* next = temp->next;
		free(temp->data);
		free(temp);
		temp = next;
	}
}

void init_paragraph_list(ParagraphNode** first)
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

void insert_paragraph_node(ParagraphNode* first, ParagraphNode** to_insert, int index)
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

ParagraphNode* find_paragraph_node(ParagraphNode* first, int index)
{
	return NULL;
}

void delete_paragraph_node(ParagraphNode** first, int index)
{
}

void free_paragraph_list(ParagraphNode* first)
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

