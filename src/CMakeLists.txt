file(GLOB SOURCE_FILES
    "*.hpp"
    "*.cpp"
)

add_executable(bookwyrm ${SOURCE_FILES})
target_link_libraries(bookwyrm fmt)

