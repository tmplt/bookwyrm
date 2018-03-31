# Required external dependencies
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_library(${PROJECT_NAME}-tui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/screens/base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/screens/item_details.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/screens/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/screens/multiselect_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tui.cpp)

target_include_directories(${PROJECT_NAME}-tui
    PUBLIC ${PROJECT_SOURCE_DIR}/src/core
    PUBLIC ${PROJECT_SOURCE_DIR}/src/tui
    PRIVATE ${CURSES_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}-tui
    ${PROJECT_NAME}-core
    ncursesw)
