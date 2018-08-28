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

find_program(cppcheck "cppcheck")
if(cppcheck)
    message(STATUS "Found ${cppcheck}, adding linting targets")

    # Set export commands on
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_custom_target(
        cppcheck
        COMMAND ${cppcheck}
        --enable=all
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        --std=c++11
        --verbose
        --quiet
        --xml-version=2
        --language=c++
        --suppress=missingIncludeSystem
        --output-file=${CMAKE_BINARY_DIR}/cppcheck_results.xml
        ${CHECK_CXX_SOURCE_FILES}
        COMMENT "Generate cppcheck report for the project"
    )

    find_program(cppcheck_html "cppcheck-htmlreport")
    if(cppcheck_html)
        add_custom_target(
            cppcheck-html
            COMMAND ${cppcheck_html}
            --title=${CMAKE_PROJECT_NAME}
            --file=${CMAKE_BINARY_DIR}/cppcheck_results.xml
            --report-dir=${CMAKE_BINARY_DIR}/cppcheck_results
            --source-dir=${CMAKE_SOURCE_DIR}
            COMMENT "Convert cppcheck report to HTML output"
        )
        add_dependencies(cppcheck-html cppcheck)
    endif()
else()
    message(STATUS "Could NOT find cppcheck")
endif()
