#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

extern int tests_run;
extern int tests_failed;

#define TEST_ASSERT_TRUE(condition) \
	do { \
		if (!(condition)) { \
			printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
			tests_failed++; \
		} \
	} while (0)

#define TEST_ASSERT_FALSE(condition) \
	TEST_ASSERT_TRUE(!(condition))

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
	do { \
		int _expected = (expected); \
		int _actual = (actual); \
		if (_expected != _actual) { \
			printf("    FAIL %s:%d: expected %d, got %d\n", \
				__FILE__, __LINE__, _expected, _actual); \
			tests_failed++; \
		} \
	} while (0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
	do { \
		const char* _expected = (expected); \
		const char* _actual = (actual); \
		if (!_expected || !_actual || strcmp(_expected, _actual) != 0) { \
			printf("    FAIL %s:%d: expected \"%s\", got \"%s\"\n", \
				__FILE__, __LINE__, \
				_expected ? _expected : "(null)", \
				_actual ? _actual : "(null)"); \
			tests_failed++; \
		} \
	} while (0)

#define TEST_ASSERT_NULL(value) \
	do { \
		if ((value) != NULL) { \
			printf("    FAIL %s:%d: expected NULL\n", __FILE__, __LINE__); \
			tests_failed++; \
		} \
	} while (0)

#define TEST_ASSERT_NOT_NULL(value) \
	do { \
		if ((value) == NULL) { \
			printf("    FAIL %s:%d: expected non-NULL\n", __FILE__, __LINE__); \
			tests_failed++; \
		} \
	} while (0)

#define RUN_TEST(test) \
	do { \
		printf("  %-45s", #test); \
		int _before = tests_failed; \
		tests_run++; \
		test(); \
		if (tests_failed == _before) \
			printf("PASS\n"); \
		else \
			printf("FAIL\n"); \
	} while (0)

#endif
