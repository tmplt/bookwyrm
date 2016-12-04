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
#include <vector>
#include "../3rdparty/cxxopts.hpp"
#include "printf.hpp"

/* void */
/* validate_arguments(cxxopts::Options options) */
/* { */
/*     std::array<void*, 5> required_arg= { */
/*         options["author"].as<std::vector<std::string>>(), */
/*         options["title"].as<std::string>(), */
/*         options["serie"].as<std::string>(), */
/*         options["publisher"].as<std::string>(), */
/*         options["ident"].as<std::string>() */
/*     }; */
/* } */

cxxopts::Options
parse_command_line(int argc, char *argv[])
{
    cxxopts::Options options(argv[0], "- find books and papers online and download them.");

    options.add_options("Necessarily inclusive arguments; at least one is required")
        ("a,author", "Author", cxxopts::value<std::vector<std::string>>())
        ("t,title", "Title", cxxopts::value<std::string>())
        ("s,serie", "Serie", cxxopts::value<std::string>())
        ("p,publisher", "Publisher", cxxopts::value<std::string>())
        ("d,ident", "Unique identifier, such as an URL or DOI", cxxopts::value<std::string>())
    ;

    options.add_options("Exact arguments; optional")
        ("y,year", "Year", cxxopts::value<int>())
        ("l,language", "Language", cxxopts::value<std::string>())
        ("e,edition", "Edition", cxxopts::value<std::string>())
        ("E,extention", "File extension without period, e.g. 'pdf'.", cxxopts::value<std::string>())
        ("i,isbn", "ISBN string", cxxopts::value<std::string>())
    ;

    options.add_options()
        ("version", "Print program version")
    #ifdef DEBUG
        ("debug", "Debug")
    #endif
    ;

    options.parse(argc, argv);
    /* validate_arguments(options); */

    return options;
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fmt::fprintf(stderr, "Print usage here.");
        return 0;
    }

    try {
        cxxopts::Options options = parse_command_line(argc, argv);
    }
    catch (cxxopts::OptionException &e) {
        fmt::printf("%s: %s\n", argv[0], e.what());
        return 1;
    }

    /* try { */
    /*     cxxopts::Options options(argv[0], "find books and papers online and download them."); */

    /*     bool apple = false; */

    /*     options.add_options() */
    /*         ("v,verbose", "Verbose mode") */
    /*         ("d,debug", "Enable debugging") */
    /*         ("t,test", "test flag") */
    /*         /1* ("b,bob", "Bob", cxxopts::value<bool>(apple)) *1/ */
    /*         /1* ("i,input", "Input", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("a,author", cxxopts::value<bool>()) *1/ */
    /*         /1* ("t,title", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("p,publisher", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("y,year", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("l,language", "two letters denoting the item's language; e.g. 'sv' or 'en'", *1/ */
    /*         /1*     cxxopts::value<std::string>()) *1/ */
    /*         /1* ("e,extension", "filename extension without period", *1/ */
    /*         /1*     cxxopts::value<std::string>()) *1/ */
    /*         /1* ("i,isbn", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("d,doi", cxxopts::value<std::string>()) *1/ */
    /*         /1* ("h,help", "print help") *1/ */
    /*     ; */

    /*     options.parse(argc, argv); */

    /*     if (options["debug"].as<bool>()) */
    /*         std::cout << "yeah" << std::endl; */

    /* } */
    /* catch(cxxopts::option_not_exists_exception &e) { */
    /*     fmt::printf("%s: %s\n", argv[0], e.what()); */
    /* } */

    return 0;
}

