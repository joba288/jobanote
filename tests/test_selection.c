#include "test_framework.h"
#include "test_helpers.h"

void test_has_selection_false_when_not_selecting(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor cursor = { doc.paragraphs, text, 2 };
	doc.cursor = cursor;
	doc.selection_start = cursor;
	doc.selecting = false;

	TEST_ASSERT_FALSE(has_selection(&doc));

	test_free_document(&doc);
}

void test_has_selection_true(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor start = { doc.paragraphs, text, 1 };
	Cursor end = { doc.paragraphs, text, 4 };

	test_set_selection(&doc, start, end);

	TEST_ASSERT_TRUE(has_selection(&doc));

	test_free_document(&doc);
}

void test_has_selection_false_for_same_cursor(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor cursor = { doc.paragraphs, text, 2 };

	test_set_selection(&doc, cursor, cursor);

	TEST_ASSERT_FALSE(has_selection(&doc));

	test_free_document(&doc);
}

void test_text_node_in_selection_partial(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	Cursor start = { doc.paragraphs, first, 3 };
	Cursor end = { doc.paragraphs, second, 2 };

	TEST_ASSERT_TRUE(text_node_in_selection(&doc, first, start, end));
	TEST_ASSERT_TRUE(text_node_in_selection(&doc, second, start, end));

	test_free_document(&doc);
}

void test_text_node_in_selection_excludes_outside_node(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");
	TextNode* third = test_add_text(doc.paragraphs, "!");

	Cursor start = { doc.paragraphs, first, 3 };
	Cursor end = { doc.paragraphs, second, 2 };

	TEST_ASSERT_TRUE(text_node_in_selection(&doc, first, start, end));
	TEST_ASSERT_TRUE(text_node_in_selection(&doc, second, start, end));
	TEST_ASSERT_FALSE(text_node_in_selection(&doc, third, start, end));

	test_free_document(&doc);
}

void test_delete_selection_inside_one_node(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello World");

	Cursor start = { doc.paragraphs, text, 5 };
	Cursor end = { doc.paragraphs, text, 6 };

	test_set_selection(&doc, start, end);
	delete_selection(&doc);

	TEST_ASSERT_EQUAL_STRING("HelloWorld", text->data);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);
	TEST_ASSERT_FALSE(doc.selecting);

	test_free_document(&doc);
}

void test_delete_selection_reversed(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello World");

	Cursor start = { doc.paragraphs, text, 6 };
	Cursor end = { doc.paragraphs, text, 5 };

	test_set_selection(&doc, start, end);
	delete_selection(&doc);

	TEST_ASSERT_EQUAL_STRING("HelloWorld", text->data);
	TEST_ASSERT_EQUAL_INT(5, doc.cursor.character);
	TEST_ASSERT_FALSE(doc.selecting);

	test_free_document(&doc);
}

void test_delete_selection_across_text_nodes(void)
{
	Document doc = test_create_document();
	TextNode* first = test_add_text(doc.paragraphs, "Hello");
	TextNode* second = test_add_text(doc.paragraphs, "World");

	Cursor start = { doc.paragraphs, first, 3 };
	Cursor end = { doc.paragraphs, second, 2 };

	test_set_selection(&doc, start, end);
	delete_selection(&doc);

	TEST_ASSERT_EQUAL_STRING("Helrld", first->data);
	TEST_ASSERT_TRUE(first->next == NULL);
	TEST_ASSERT_TRUE(doc.cursor.text_node == first);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	test_free_document(&doc);
}

void test_delete_selection_across_paragraphs(void)
{
	Document doc = test_create_document();
	ParagraphNode* first_para = doc.paragraphs;
	TextNode* first = test_add_text(first_para, "Hello");

	ParagraphNode* second_para = test_add_paragraph(&doc);
	TextNode* second = test_add_text(second_para, "World");

	Cursor start = { first_para, first, 3 };
	Cursor end = { second_para, second, 2 };

	test_set_selection(&doc, start, end);
	delete_selection(&doc);

	TEST_ASSERT_TRUE(doc.paragraphs == first_para);
	TEST_ASSERT_NULL(first_para->next);
	TEST_ASSERT_EQUAL_STRING("Helrld", first_para->text->data);
	TEST_ASSERT_FALSE(doc.selecting);
	TEST_ASSERT_EQUAL_INT(3, doc.cursor.character);

	test_free_document(&doc);
}

void test_toggle_bold_selection(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor start = { doc.paragraphs, text, 1 };
	Cursor end = { doc.paragraphs, text, 4 };

	test_set_selection(&doc, start, end);
	toggle_bold_selection(&doc);

	TextNode* before = doc.paragraphs->text;
	TextNode* selected = before->next;
	TextNode* after = selected->next;

	TEST_ASSERT_EQUAL_STRING("H", before->data);
	TEST_ASSERT_EQUAL_STRING("ell", selected->data);
	TEST_ASSERT_EQUAL_STRING("o", after->data);
	TEST_ASSERT_TRUE(selected->resolved_style->bold);
	TEST_ASSERT_FALSE(before->resolved_style->bold);
	TEST_ASSERT_FALSE(after->resolved_style->bold);
	TEST_ASSERT_FALSE(doc.selecting);

	test_free_document(&doc);
}

void test_toggle_bold_selection_twice(void)
{
	Document doc = test_create_document();
	TextNode* text = test_add_text(doc.paragraphs, "Hello");

	Cursor start = { doc.paragraphs, text, 1 };
	Cursor end = { doc.paragraphs, text, 4 };

	test_set_selection(&doc, start, end);
	toggle_bold_selection(&doc);

	TextNode* selected = doc.paragraphs->text->next;

	Cursor selected_start = { doc.paragraphs, selected, 0 };
	Cursor selected_end = { doc.paragraphs, selected, 3 };

	test_set_selection(&doc, selected_start, selected_end);
	toggle_bold_selection(&doc);

	TEST_ASSERT_FALSE(selected->resolved_style->bold);

	test_free_document(&doc);
}
