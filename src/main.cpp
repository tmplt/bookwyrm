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

