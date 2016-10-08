/*
 * This file is part of bookwyrm.
 * Copyright 2016, Tmplt.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 */

#include <iostream>
#include <string>
#include "../3rdparty/cxxopts.hpp"

int main(int argc, char *argv[])
{
    try {
        cxxopts::Options options(argv[0], " - find books and papers online and download them.");

        bool apple = false;

        options.add_options()
            ("b,bob", "Bob", cxxopts::value<bool>(apple))
            ("i,input", "Input", cxxopts::value<std::string>())
            //("a,author", cxxopts::value<bool>());
            /* ("t,title", cxxopts::value<std::string>()) */
            /* ("p,publisher", cxxopts::value<std::string>()) */
            /* ("y,year", cxxopts::value<std::string>()) */
            /* ("l,language", "two letters denoting the item's language; e.g. 'sv' or 'en'", */
            /*     cxxopts::value<std::string>()) */
            /* ("e,extension", "filename extension without period", */
            /*     cxxopts::value<std::string>()) */
            /* ("i,isbn", cxxopts::value<std::string>()) */
            /* ("d,doi", cxxopts::value<std::string>()) */
            /* ("h,help", "print help") */
        ;

        options.parse(argc, argv);
    }
    catch(...) {
        std::cout << "oops" << std::endl;
    }
}

