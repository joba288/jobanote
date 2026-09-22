#include "test_framework.h"
#include "test_helpers.h"

void test_document_initialisation(void)
{
	Document doc;
	init_document(&doc);

	TEST_ASSERT_NULL(doc.paragraphs);

	free_document(&doc);
}

void test_document_with_empty_paragraph(void)
{
	Document doc = test_create_document();

	TEST_ASSERT_NOT_NULL(doc.paragraphs);
	TEST_ASSERT_NULL(doc.paragraphs->text);
	TEST_ASSERT_NULL(doc.paragraphs->prev);
	TEST_ASSERT_NULL(doc.paragraphs->next);

	test_free_document(&doc);
}

void test_last_text_node(void)
{
	Document doc = test_create_document();

	TextNode* first = test_add_text(doc.paragraphs, "One");
	TextNode* second = test_add_text(doc.paragraphs, "Two");
	TextNode* third = test_add_text(doc.paragraphs, "Three");

	TEST_ASSERT_TRUE(last_text_node(first) == third);
	TEST_ASSERT_TRUE(last_text_node(second) == third);
	TEST_ASSERT_NULL(last_text_node(NULL));

	test_free_document(&doc);
}

void test_null_and_empty_operations(void)
{
	delete_text_node(NULL, 0);
	delete_paragraph_node(NULL, 0);
	free_text_list(NULL);
	free_paragraph_list(NULL);
	delete_character_from_node(NULL, NULL);

	TEST_ASSERT_TRUE(true);
}

void test_delete_selection_without_selection(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	doc.cursor = (Cursor){ doc.paragraphs, text, 2 };
	doc.selection_start = doc.cursor;
	doc.selecting = false;

	delete_selection(&doc);

	TEST_ASSERT_EQUAL_STRING("Hello", text->data);
	TEST_ASSERT_EQUAL_INT(2, doc.cursor.character);
	TEST_ASSERT_FALSE(doc.selecting);

	test_free_document(&doc);
}

void test_insert_out_of_range_text_index_appends(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);
	insert_text_node(&list, "B", NULL, 100);

	TEST_ASSERT_EQUAL_STRING("A", list->data);
	TEST_ASSERT_EQUAL_STRING("B", list->next->data);

	free_text_list(list);
}

void test_delete_out_of_range_does_nothing(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);

	delete_text_node(&list, 100);

	TEST_ASSERT_EQUAL_STRING("A", list->data);
	TEST_ASSERT_NULL(list->next);

	free_text_list(list);
}

void test_empty_document_character_operations(void)
{
	Document doc = test_create_document();

	increment_cursor(&doc);
	decrement_cursor(&doc);
	delete_character(&doc);
	delete_character_forward(&doc);

	TEST_ASSERT_TRUE(doc.cursor.paragraph_node == doc.paragraphs);
	TEST_ASSERT_NULL(doc.cursor.text_node);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}
