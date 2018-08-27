/*
 * A modified version of Scott Shurr's str_const:
 * <https://github.com/boostcon/cppnow_presentations_2012/blob/master/wed/schurr_cpp11_tools_for_class_authors.pdf?raw=true>
 *
 * No clue how (and if) I should add a copyright clause to this file; please
 * create an issue on the topic if you know.
 */

#pragma once

#include <iostream>

class str_const {
private:
    const char *const p_;
    const std::size_t sz_;

public:
    template <std::size_t N> constexpr str_const(const char (&a)[N]) : p_(a), sz_(N - 1) {}

    constexpr const char *c_str() const { return p_; }

    friend std::ostream &operator<<(std::ostream &os, const str_const &str)
    {
        os << str.c_str();
        return os;
    }
};
