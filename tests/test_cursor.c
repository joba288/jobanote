#include "test_framework.h"
#include "test_helpers.h"

void test_cursor_equal_same_position(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor a = { doc.paragraphs, text, 2 };
	Cursor b = { doc.paragraphs, text, 2 };

	TEST_ASSERT_TRUE(cursor_equal(&a, &b));

	test_free_document(&doc);
}

void test_cursor_equal_different_position(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor a = { doc.paragraphs, text, 2 };
	Cursor b = { doc.paragraphs, text, 3 };

	TEST_ASSERT_FALSE(cursor_equal(&a, &b));

	test_free_document(&doc);
}

void test_cursor_compare_same_node(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor a = { doc.paragraphs, text, 1 };
	Cursor b = { doc.paragraphs, text, 4 };

	TEST_ASSERT_EQUAL_INT(-1, cursor_compare(&doc, a, b));
	TEST_ASSERT_EQUAL_INT(1, cursor_compare(&doc, b, a));
	TEST_ASSERT_EQUAL_INT(0, cursor_compare(&doc, a, a));

	test_free_document(&doc);
}

void test_cursor_compare_nodes(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	Cursor a = { doc.paragraphs, first, 5 };
	Cursor b = { doc.paragraphs, second, 0 };

	TEST_ASSERT_EQUAL_INT(-1, cursor_compare(&doc, a, b));
	TEST_ASSERT_EQUAL_INT(1, cursor_compare(&doc, b, a));

	test_free_document(&doc);
}

void test_cursor_compare_paragraphs(void)
{
	Document doc = test_create_document();
	ParagraphNode* first_para = doc.paragraphs;
	TextNode* first_text = test_add_text(first_para, "First");

	ParagraphNode* second_para = test_add_paragraph(&doc);
	TextNode* second_text = test_add_text(second_para, "Second");

	Cursor a = { first_para, first_text, 5 };
	Cursor b = { second_para, second_text, 0 };

	TEST_ASSERT_EQUAL_INT(-1, cursor_compare(&doc, a, b));
	TEST_ASSERT_EQUAL_INT(1, cursor_compare(&doc, b, a));

	test_free_document(&doc);
}

void test_increment_cursor_ascii(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "ABC");

	test_set_cursor(&doc, doc.paragraphs, text, 0);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(2, doc.cursor.character);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	test_free_document(&doc);
}

void test_increment_cursor_utf8(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Aé😀");

	test_set_cursor(&doc, doc.paragraphs, text, 0);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	increment_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(7, doc.cursor.character);

	test_free_document(&doc);
}

void test_increment_cursor_next_text_node(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	test_set_cursor(&doc, doc.paragraphs, first, 5);

	increment_cursor(&doc);

	TEST_ASSERT_TRUE(doc.cursor.text_node == second);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_increment_cursor_next_paragraph(void)
{
	Document doc = test_create_document();
	TextNode* first_text = test_add_text(doc.paragraphs, "First");

	ParagraphNode* second_para = test_add_paragraph(&doc);
	TextNode* second_text = test_add_text(second_para, "Second");

	test_set_cursor(&doc, doc.paragraphs, first_text, 5);

	increment_cursor(&doc);

	TEST_ASSERT_TRUE(doc.cursor.paragraph_node == second_para);
	TEST_ASSERT_TRUE(doc.cursor.text_node == second_text);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_decrement_cursor_utf8(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Aé😀");

	test_set_cursor(&doc, doc.paragraphs, text, 7);

	decrement_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	decrement_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(1, doc.cursor.character);

	decrement_cursor(&doc);
	TEST_ASSERT_EQUAL_INT(0, doc.cursor.character);

	test_free_document(&doc);
}

void test_decrement_cursor_previous_text_node(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	test_set_cursor(&doc, doc.paragraphs, second, 0);

	decrement_cursor(&doc);

	TEST_ASSERT_TRUE(doc.cursor.text_node == first);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);

	test_free_document(&doc);
}

void test_decrement_cursor_previous_paragraph(void)
{
	Document doc = test_create_document();
	TextNode* first_text = test_add_text(doc.paragraphs, "First");

	ParagraphNode* second_para = test_add_paragraph(&doc);
	TextNode* second_text = test_add_text(second_para, "Second");

	test_set_cursor(&doc, second_para, second_text, 0);

	decrement_cursor(&doc);

	TEST_ASSERT_TRUE(doc.cursor.paragraph_node == doc.paragraphs);
	TEST_ASSERT_TRUE(doc.cursor.text_node == first_text);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);

	test_free_document(&doc);
}
