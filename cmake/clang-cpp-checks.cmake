# Adding clang-format check and formatter if found
find_program(clang_format NAMES "clang-format-6.0" "clang-format-5.0" "clang-format-4.0" "clang-format")
if(clang_format)
    exec_program(${clang_format} ${CMAKE_CURRENT_SOURCE_DIR} ARGS --version OUTPUT_VARIABLE clang_version)
    string(REGEX REPLACE ".*([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" clang_major_version ${clang_version})

    if((${clang_major_version} GREATER "4") OR (${clang_major_version} EQUAL "4"))
        message(STATUS "Found ${clang_format}, adding formatting targets")
        add_custom_target(
            format
            COMMAND
            ${clang_format}
            -i
            -style=file
            ${check_cxx_source_files}
            COMMENT "Auto formatting of all source files"
        )

        add_custom_target(
            check-format
            COMMAND
            ${clang_format}
            -style=file
            -output-replacements-xml
            ${check_cxx_source_files}
            # print output
            | tee ${CMAKE_BINARY_DIR}/check_format_file.txt | grep -c "replacement " |
            tr -d "[:cntrl:]" && echo " replacements necessary"
            # WARNING: fix to stop with error if there are problems
            COMMAND ! grep -c "replacement "
            ${CMAKE_BINARY_DIR}/check_format_file.txt > /dev/null
            COMMENT "Checking format compliance"
        )

    else()
        message(STATUS "Could only find version ${clang_major_version} or clang-format, but version >= is required.")
    endif()
else()
    message(STATUS "Could NOT find clang-format")
endif()
