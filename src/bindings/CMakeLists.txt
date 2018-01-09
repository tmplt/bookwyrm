pybind11_add_module(pybookwyrm python.cpp ${SOURCE})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    # Relocation R_X86_64_PC32 against symbols declared in the following libraries
    # cannot be used when making a shared object; compilation must be done with -fPIC
    set_target_properties(termbox_lib_static PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(fuzzywuzzy PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

# Now this, this isn't very clean.
# Requiring linking to fuzzywuzzy I can understand, since the item matching function uses it,
# but termbox? curl? Why are they needed?
# TODO: investigate and clean up

### Is this because the whole object depends on them?
### Create a small class that wraps everything Python uses?


target_link_libraries(pybookwyrm PRIVATE
    stdc++fs
    termbox_lib_static
    fuzzywuzzy
    curl)
