#include "test_framework.h"
#include "jobanote.h"

void test_utf8_ascii_length(void)
{
	TEST_ASSERT_EQUAL_INT(1, utf8_char_len("A"));
}

void test_utf8_two_byte_length(void)
{
	TEST_ASSERT_EQUAL_INT(2, utf8_char_len("é"));
}

void test_utf8_three_byte_length(void)
{
	TEST_ASSERT_EQUAL_INT(3, utf8_char_len("€"));
}

void test_utf8_four_byte_length(void)
{
	TEST_ASSERT_EQUAL_INT(4, utf8_char_len("😀"));
}

void test_utf8_previous_ascii(void)
{
	TEST_ASSERT_EQUAL_INT(1, utf8_prev_char_len("Hello", 5));
}

void test_utf8_previous_two_byte(void)
{
	TEST_ASSERT_EQUAL_INT(2, utf8_prev_char_len("hé", 3));
}

void test_utf8_previous_three_byte(void)
{
	TEST_ASSERT_EQUAL_INT(3, utf8_prev_char_len("€", 3));
}

void test_utf8_previous_four_byte(void)
{
	TEST_ASSERT_EQUAL_INT(4, utf8_prev_char_len("😀", 4));
}

void test_utf8_previous_invalid_position(void)
{
	TEST_ASSERT_EQUAL_INT(0, utf8_prev_char_len("Hello", 0));
	TEST_ASSERT_EQUAL_INT(0, utf8_prev_char_len(NULL, 1));
}
