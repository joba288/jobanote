add_executable(jobanote_tests
    test_main.c
    test_utf8.c
    test_lists.c
    test_cursor.c
    test_editing.c
    test_selection.c
    test_lifecycle.c
)

target_include_directories(jobanote_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(jobanote_tests
    PRIVATE
    jobanote_lib
)

add_test(
    NAME jobanote_tests
    COMMAND jobanote_tests
)
