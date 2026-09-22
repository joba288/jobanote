#include "test_framework.h"
#include "test_helpers.h"

void test_text_list_insert_front(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "World", NULL, 0);
	insert_text_node(&list, "Hello", NULL, 0);

	TEST_ASSERT_EQUAL_STRING("Hello", list->data);
	TEST_ASSERT_EQUAL_STRING("World", list->next->data);
	TEST_ASSERT_TRUE(list->next->prev == list);

	free_text_list(list);
}

void test_text_list_insert_middle(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "One", NULL, 0);
	insert_text_node(&list, "Three", NULL, 1);
	insert_text_node(&list, "Two", NULL, 1);

	TEST_ASSERT_EQUAL_STRING("One", find_text_node(list, 0)->data);
	TEST_ASSERT_EQUAL_STRING("Two", find_text_node(list, 1)->data);
	TEST_ASSERT_EQUAL_STRING("Three", find_text_node(list, 2)->data);

	TEST_ASSERT_TRUE(find_text_node(list, 1)->prev == find_text_node(list, 0));
	TEST_ASSERT_TRUE(find_text_node(list, 1)->next == find_text_node(list, 2));

	free_text_list(list);
}

void test_text_list_find(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);
	insert_text_node(&list, "B", NULL, 1);
	insert_text_node(&list, "C", NULL, 2);

	TEST_ASSERT_EQUAL_STRING("A", find_text_node(list, 0)->data);
	TEST_ASSERT_EQUAL_STRING("B", find_text_node(list, 1)->data);
	TEST_ASSERT_EQUAL_STRING("C", find_text_node(list, 2)->data);
	TEST_ASSERT_NULL(find_text_node(list, 3));

	free_text_list(list);
}

void test_text_list_delete_first(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);
	insert_text_node(&list, "B", NULL, 1);
	insert_text_node(&list, "C", NULL, 2);

	delete_text_node(&list, 0);

	TEST_ASSERT_EQUAL_STRING("B", list->data);
	TEST_ASSERT_NULL(list->prev);
	TEST_ASSERT_EQUAL_STRING("C", list->next->data);
	TEST_ASSERT_TRUE(list->next->prev == list);

	free_text_list(list);
}

void test_text_list_delete_middle(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);
	insert_text_node(&list, "B", NULL, 1);
	insert_text_node(&list, "C", NULL, 2);

	delete_text_node(&list, 1);

	TEST_ASSERT_EQUAL_STRING("A", list->data);
	TEST_ASSERT_EQUAL_STRING("C", list->next->data);
	TEST_ASSERT_TRUE(list->next->prev == list);
	TEST_ASSERT_NULL(list->next->next);

	free_text_list(list);
}

void test_text_list_delete_last(void)
{
	TextNode* list = NULL;

	insert_text_node(&list, "A", NULL, 0);
	insert_text_node(&list, "B", NULL, 1);

	delete_text_node(&list, 1);

	TEST_ASSERT_EQUAL_STRING("A", list->data);
	TEST_ASSERT_NULL(list->next);

	free_text_list(list);
}

void test_paragraph_list_insert(void)
{
	ParagraphNode* list = NULL;

	ParagraphNode* a = test_create_paragraph();
	ParagraphNode* b = test_create_paragraph();
	ParagraphNode* c = test_create_paragraph();

	insert_paragraph_node(&list, a, 0);
	insert_paragraph_node(&list, c, 1);
	insert_paragraph_node(&list, b, 1);

	TEST_ASSERT_TRUE(list == a);
	TEST_ASSERT_TRUE(list->next == b);
	TEST_ASSERT_TRUE(b->next == c);
	TEST_ASSERT_TRUE(b->prev == a);
	TEST_ASSERT_TRUE(c->prev == b);

	free_paragraph_list(list);
}

void test_paragraph_list_find(void)
{
	ParagraphNode* list = NULL;

	ParagraphNode* a = test_create_paragraph();
	ParagraphNode* b = test_create_paragraph();

	insert_paragraph_node(&list, a, 0);
	insert_paragraph_node(&list, b, 1);

	/*
	 * This currently exposes a real bug in jobanote.c:
	 * find_paragraph_node() is implemented as "return NULL;".
	 */
	TEST_ASSERT_TRUE(find_paragraph_node(list, 0) == a);
	TEST_ASSERT_TRUE(find_paragraph_node(list, 1) == b);
	TEST_ASSERT_NULL(find_paragraph_node(list, 2));

	free_paragraph_list(list);
}

void test_paragraph_list_delete(void)
{
	ParagraphNode* list = NULL;

	ParagraphNode* a = test_create_paragraph();
	ParagraphNode* b = test_create_paragraph();
	ParagraphNode* c = test_create_paragraph();

	insert_paragraph_node(&list, a, 0);
	insert_paragraph_node(&list, b, 1);
	insert_paragraph_node(&list, c, 2);

	delete_paragraph_node(&list, 1);

	TEST_ASSERT_TRUE(list == a);
	TEST_ASSERT_TRUE(a->next == c);
	TEST_ASSERT_TRUE(c->prev == a);

	free_paragraph_list(list);
}
