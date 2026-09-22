#include "test_framework.h"
#include "test_helpers.h"

void test_insert_character_empty_paragraph(void)
{
	Document doc = test_create_document();

	insert_character(&doc, 'A');

	TEST_ASSERT_NOT_NULL(doc.paragraphs->text);
	TEST_ASSERT_EQUAL_STRING("A", doc.paragraphs->text->data);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	test_free_document(&doc);
}

void test_insert_character_existing_text(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hllo");

	test_set_cursor(&doc, doc.paragraphs, text, 1);

	insert_character(&doc, 'e');

	TEST_ASSERT_EQUAL_STRING("Hello", text->data);
	TEST_ASSERT_EQUAL_INT(2, doc.cursor.character);

	test_free_document(&doc);
}

void test_insert_character_utf8(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "A");

	test_set_cursor(&doc, doc.paragraphs, text, 1);

	insert_character(&doc, 0x1F600);

	TEST_ASSERT_EQUAL_STRING("A😀", text->data);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);

	test_free_document(&doc);
}

void test_insert_character_invalid_codepoint(void)
{
	Document doc = test_create_document();

	insert_character(&doc, 0x110000);

	TEST_ASSERT_NULL(doc.paragraphs->text);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_character_from_node_ascii(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");
	int position = 5;

	delete_character_from_node(text, &position);

	TEST_ASSERT_EQUAL_STRING("Hell", text->data);
	TEST_ASSERT_EQUAL_INT(4, position);

	test_free_document(&doc);
}

void test_delete_character_from_node_utf8(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Aé😀");
	int position = 7;

	delete_character_from_node(text, &position);

	TEST_ASSERT_EQUAL_STRING("Aé", text->data);
	TEST_ASSERT_EQUAL_INT(3, position);

	test_free_document(&doc);
}

void test_delete_character_inside_node(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	test_set_cursor(&doc, doc.paragraphs, text, 5);
	delete_character(&doc);

	TEST_ASSERT_EQUAL_STRING("Hell", text->data);
	TEST_ASSERT_EQUAL_INT(4, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_character_between_text_nodes(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	test_set_cursor(&doc, doc.paragraphs, second, 0);
	delete_character(&doc);

	TEST_ASSERT_TRUE(doc.cursor.text_node == first);
	TEST_ASSERT_EQUAL_INT(4, doc.cursor.character);
	TEST_ASSERT_EQUAL_STRING("Hell", first->data);
	TEST_ASSERT_EQUAL_STRING("World", second->data);

	test_free_document(&doc);
}

void test_delete_character_merges_paragraphs(void)
{
	Document doc = test_create_document();
	ParagraphNode* first_para = doc.paragraphs;
	TextNode* first_text = test_add_text(first_para, "Hello");

	ParagraphNode* second_para = test_add_paragraph(&doc);
	TextNode* second_text = test_add_text(second_para, "World");

	test_set_cursor(&doc, second_para, second_text, 0);
	delete_character(&doc);

	TEST_ASSERT_TRUE(doc.paragraphs == first_para);
	TEST_ASSERT_NULL(first_para->next);
	TEST_ASSERT_TRUE(first_para->text == first_text);
	TEST_ASSERT_TRUE(first_text->next == second_text);
	TEST_ASSERT_TRUE(doc.cursor.text_node == second_text);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_character_at_document_start_does_nothing(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	test_set_cursor(&doc, doc.paragraphs, text, 0);
	delete_character(&doc);

	TEST_ASSERT_EQUAL_STRING("Hello", text->data);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_character_forward(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	test_set_cursor(&doc, doc.paragraphs, text, 1);
	delete_character_forward(&doc);

	TEST_ASSERT_EQUAL_STRING("Hllo", text->data);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_character_forward_utf8(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "AéB");

	test_set_cursor(&doc, doc.paragraphs, text, 1);
	delete_character_forward(&doc);

	TEST_ASSERT_EQUAL_STRING("AB", text->data);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	test_free_document(&doc);
}

void test_split_text_node(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "HelloWorld");

	TextNode* second = split_text_node(first, 5);

	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_EQUAL_STRING("Hello", first->data);
	TEST_ASSERT_EQUAL_STRING("World", second->data);
	TEST_ASSERT_TRUE(second->prev == first);
	TEST_ASSERT_TRUE(first->next == second);

	test_free_document(&doc);
}

void test_split_text_node_copies_style(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "HelloWorld");

	first->resolved_style->bold = true;
	first->resolved_style->italic = true;

	TextNode* second = split_text_node(first, 5);

	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(second->resolved_style->bold);
	TEST_ASSERT_TRUE(second->resolved_style->italic);

	test_free_document(&doc);
}

void test_split_text_node_invalid_position(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	TEST_ASSERT_TRUE(split_text_node(text, 0) == text);
	TEST_ASSERT_NULL(split_text_node(text, 5));
	TEST_ASSERT_NULL(split_text_node(text, 10));

	test_free_document(&doc);
}

void test_merge_paragraph_nodes(void)
{
	Document doc = test_create_document();
	ParagraphNode* first = doc.paragraphs;
	ParagraphNode* second = test_add_paragraph(&doc);

	TextNode* first_text = test_add_text(first, "Hello");
	TextNode* second_text = test_add_text(second, "World");

	merge_paragraph_nodes(first, second);

	TEST_ASSERT_TRUE(first->text == first_text);
	TEST_ASSERT_TRUE(first_text->next == second_text);
	TEST_ASSERT_TRUE(second_text->prev == first_text);
	TEST_ASSERT_NULL(first->next);

	test_free_document(&doc);
}

void test_insert_paragraph_at_cursor_splits_text(void)
{
	Document doc = test_create_document();
	ParagraphNode* first = doc.paragraphs;
	TextNode* text = test_add_text(first, "Hello World");

	test_set_cursor(&doc, first, text, 5);

	ParagraphNode* second = insert_paragraph_node_at_cursor(&doc);

	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(first->next == second);
	TEST_ASSERT_EQUAL_STRING("Hello", first->text->data);
	TEST_ASSERT_EQUAL_STRING(" World", second->text->data);
	TEST_ASSERT_TRUE(doc.cursor.paragraph_node == second);
	TEST_ASSERT_TRUE(doc.cursor.text_node == second->text);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_insert_paragraph_at_empty_paragraph(void)
{
	Document doc = test_create_document();
	ParagraphNode* first = doc.paragraphs;

	ParagraphNode* second = insert_paragraph_node_at_cursor(&doc);

	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(first->next == second);
	TEST_ASSERT_NULL(first->text);
	TEST_ASSERT_NULL(second->text);
	TEST_ASSERT_TRUE(doc.cursor.paragraph_node == second);
	TEST_ASSERT_NULL(doc.cursor.text_node);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}
