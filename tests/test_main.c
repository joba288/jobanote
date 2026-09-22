#include "test_framework.h"

int tests_run = 0;
int tests_failed = 0;

/* UTF-8 */
void test_utf8_ascii_length(void);
void test_utf8_two_byte_length(void);
void test_utf8_three_byte_length(void);
void test_utf8_four_byte_length(void);
void test_utf8_previous_ascii(void);
void test_utf8_previous_two_byte(void);
void test_utf8_previous_three_byte(void);
void test_utf8_previous_four_byte(void);
void test_utf8_previous_invalid_position(void);

/* Lists */
void test_text_list_insert_front(void);
void test_text_list_insert_middle(void);
void test_text_list_find(void);
void test_text_list_delete_first(void);
void test_text_list_delete_middle(void);
void test_text_list_delete_last(void);
void test_paragraph_list_insert(void);
void test_paragraph_list_find(void);
void test_paragraph_list_delete(void);

/* Cursor */
void test_cursor_equal_same_position(void);
void test_cursor_equal_different_position(void);
void test_cursor_compare_same_node(void);
void test_cursor_compare_nodes(void);
void test_cursor_compare_paragraphs(void);
void test_increment_cursor_ascii(void);
void test_increment_cursor_utf8(void);
void test_increment_cursor_next_text_node(void);
void test_increment_cursor_next_paragraph(void);
void test_decrement_cursor_utf8(void);
void test_decrement_cursor_previous_text_node(void);
void test_decrement_cursor_previous_paragraph(void);

/* Editing */
void test_insert_character_empty_paragraph(void);
void test_insert_character_existing_text(void);
void test_insert_character_utf8(void);
void test_insert_character_invalid_codepoint(void);
void test_delete_character_from_node_ascii(void);
void test_delete_character_from_node_utf8(void);
void test_delete_character_inside_node(void);
void test_delete_character_between_text_nodes(void);
void test_delete_character_merges_paragraphs(void);
void test_delete_character_at_document_start_does_nothing(void);
void test_delete_character_forward(void);
void test_delete_character_forward_utf8(void);
void test_split_text_node(void);
void test_split_text_node_copies_style(void);
void test_split_text_node_invalid_position(void);
void test_merge_paragraph_nodes(void);
void test_insert_paragraph_at_cursor_splits_text(void);
void test_insert_paragraph_at_empty_paragraph(void);

/* Selection */
void test_has_selection_false_when_not_selecting(void);
void test_has_selection_true(void);
void test_has_selection_false_for_same_cursor(void);
void test_text_node_in_selection_partial(void);
void test_text_node_in_selection_excludes_outside_node(void);
void test_delete_selection_inside_one_node(void);
void test_delete_selection_reversed(void);
void test_delete_selection_across_text_nodes(void);
void test_delete_selection_across_paragraphs(void);
void test_toggle_bold_selection(void);
void test_toggle_bold_selection_twice(void);

/* Lifecycle / edge cases */
void test_document_initialisation(void);
void test_document_with_empty_paragraph(void);
void test_last_text_node(void);
void test_null_and_empty_operations(void);
void test_delete_selection_without_selection(void);
void test_insert_out_of_range_text_index_appends(void);
void test_delete_out_of_range_does_nothing(void);
void test_empty_document_character_operations(void);

int main(void)
{
	printf("\nJobanote tests\n");
	printf("==============\n\n");

	printf("UTF-8\n");
	RUN_TEST(test_utf8_ascii_length);
	RUN_TEST(test_utf8_two_byte_length);
	RUN_TEST(test_utf8_three_byte_length);
	RUN_TEST(test_utf8_four_byte_length);
	RUN_TEST(test_utf8_previous_ascii);
	RUN_TEST(test_utf8_previous_two_byte);
	RUN_TEST(test_utf8_previous_three_byte);
	RUN_TEST(test_utf8_previous_four_byte);
	RUN_TEST(test_utf8_previous_invalid_position);

	printf("\nLinked lists\n");
	RUN_TEST(test_text_list_insert_front);
	RUN_TEST(test_text_list_insert_middle);
	RUN_TEST(test_text_list_find);
	RUN_TEST(test_text_list_delete_first);
	RUN_TEST(test_text_list_delete_middle);
	RUN_TEST(test_text_list_delete_last);
	RUN_TEST(test_paragraph_list_insert);
	RUN_TEST(test_paragraph_list_find);
	RUN_TEST(test_paragraph_list_delete);

	printf("\nCursor\n");
	RUN_TEST(test_cursor_equal_same_position);
	RUN_TEST(test_cursor_equal_different_position);
	RUN_TEST(test_cursor_compare_same_node);
	RUN_TEST(test_cursor_compare_nodes);
	RUN_TEST(test_cursor_compare_paragraphs);
	RUN_TEST(test_increment_cursor_ascii);
	RUN_TEST(test_increment_cursor_utf8);
	RUN_TEST(test_increment_cursor_next_text_node);
	RUN_TEST(test_increment_cursor_next_paragraph);
	RUN_TEST(test_decrement_cursor_utf8);
	RUN_TEST(test_decrement_cursor_previous_text_node);
	RUN_TEST(test_decrement_cursor_previous_paragraph);

	printf("\nEditing\n");
	RUN_TEST(test_insert_character_empty_paragraph);
	RUN_TEST(test_insert_character_existing_text);
	RUN_TEST(test_insert_character_utf8);
	RUN_TEST(test_insert_character_invalid_codepoint);
	RUN_TEST(test_delete_character_from_node_ascii);
	RUN_TEST(test_delete_character_from_node_utf8);
	RUN_TEST(test_delete_character_inside_node);
	RUN_TEST(test_delete_character_between_text_nodes);
	RUN_TEST(test_delete_character_merges_paragraphs);
	RUN_TEST(test_delete_character_at_document_start_does_nothing);
	RUN_TEST(test_delete_character_forward);
	RUN_TEST(test_delete_character_forward_utf8);
	RUN_TEST(test_split_text_node);
	RUN_TEST(test_split_text_node_copies_style);
	RUN_TEST(test_split_text_node_invalid_position);
	RUN_TEST(test_merge_paragraph_nodes);
	RUN_TEST(test_insert_paragraph_at_cursor_splits_text);
	RUN_TEST(test_insert_paragraph_at_empty_paragraph);

	printf("\nSelection\n");
	RUN_TEST(test_has_selection_false_when_not_selecting);
	RUN_TEST(test_has_selection_true);
	RUN_TEST(test_has_selection_false_for_same_cursor);
	RUN_TEST(test_text_node_in_selection_partial);
	RUN_TEST(test_text_node_in_selection_excludes_outside_node);
	RUN_TEST(test_delete_selection_inside_one_node);
	RUN_TEST(test_delete_selection_reversed);
	RUN_TEST(test_delete_selection_across_text_nodes);
	RUN_TEST(test_delete_selection_across_paragraphs);
	RUN_TEST(test_toggle_bold_selection);
	RUN_TEST(test_toggle_bold_selection_twice);

	printf("\nLifecycle / edge cases\n");
	RUN_TEST(test_document_initialisation);
	RUN_TEST(test_document_with_empty_paragraph);
	RUN_TEST(test_last_text_node);
	RUN_TEST(test_null_and_empty_operations);
	RUN_TEST(test_delete_selection_without_selection);
	RUN_TEST(test_insert_out_of_range_text_index_appends);
	RUN_TEST(test_delete_out_of_range_does_nothing);
	RUN_TEST(test_empty_document_character_operations);

	printf("\n==============================\n");
	printf("Tests run:    %d\n", tests_run);
	printf("Assertions failed: %d\n", tests_failed);

	if (tests_failed == 0)
	{
		printf("ALL TESTS PASSED\n");
		return 0;
	}

	printf("TESTS FAILED\n");
	return 1;
}
