/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdint>
#include <exception>

#include "components/command_line.hpp"
#include "spdlog/spdlog.h"
#include "config.hpp"

using add_arg = command_line::option;

int main(int argc, char *argv[])
{
    const command_line::options opts{
        add_arg("-h", "--help",      "Display this text and exit "),
        add_arg("-v", "--version",   "Print version information"),
        add_arg("-l", "--log",       "Set logging level to info"),

        /* Exclusive arguments; cannot be combined with any other arguments. */
        add_arg("-d", "--ident",     "Specify an item identification (such as DOI, URL, etc.)"),

        /* Main arguments; at least one of these are required. */
        /* auto main = command_line::add_group( */
        /*     "main", "necessarily inclusive arguments; at least one required" */
        /* ); */
        add_arg("-a", "--author",    "Specify authors"),
        add_arg("-t", "--title",     "Specify title"),
        add_arg("-s", "--serie",     "Specify serie"),
        add_arg("-p", "--publisher", "Specify publisher"),

        /* Exact data arguments; all are optional. */
        add_arg("-y", "--year",      "Specify year of release"),
        add_arg("-L", "--language",  "Specify text language"),
        add_arg("-e", "--edition",   "Specify item edition"),
        add_arg("-E", "--extension", "Specify item extension", "EXT",
                {"epub", "pdf", "djvu"}),
        add_arg("-i", "--isbn",      "Specify item ISBN"),
    };

    uint8_t exit_code = EXIT_SUCCESS;

    auto logger = spdlog::stdout_color_mt("logger");
    spdlog::set_pattern("%l: %v");

    try {
        /* Parse command line arguments */
        std::string progname = argv[0];
        std::vector<std::string> args(argv + 1, argv + argc);

        cliparser::make_type cli = cliparser::make(std::move(progname), std::move(opts));
        cli->process_input(args);

        if (cli->has("help")) {
            cli->usage();
            return EXIT_SUCCESS;
        } else if (cli->has("version")) {
            print_build_info();
            return EXIT_SUCCESS;
        } else if (args.empty()) {
            cli->usage();
            return EXIT_FAILURE;
        }

        if (cli->has("log"))
            spdlog::set_level(spdlog::level::info);

        logger->info("The mighty eldwyrm has been summoned!");

    } catch (const std::exception &err) {
        logger->error(err.what());
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
