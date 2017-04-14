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

#include <cstdint>    // explicitly-sized integral types
#include <cstdlib>    // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>  // std::exception

#include <spdlog/spdlog.h>

#include "item.hpp"
#include "components/command_line.hpp"
#include "components/logger.hpp"
#include "config.hpp"

int main(int argc, char *argv[])
{
    using valid_opts = std::initializer_list<string>;

    /* Future work: make main, excl, exact, misc and groups constexpr. */
    const auto main = command_line::option_group("Main", "necessarily inclusive arguments; at least one required")
        ("-a", "--author",    "Specify authors", "AUTHOR")
        ("-t", "--title",     "Specify title", "TITLE")
        ("-s", "--serie",     "Specify serie", "SERIE")
        ("-p", "--publisher", "Specify publisher", "PUBLISHER");

    const auto excl = command_line::option_group("Exclusive", "cannot be combined with any other arguments")
        ("-d", "--ident",     "Specify an item identification (such as DOI, URL, etc.)", "IDENT");

    const auto exact = command_line::option_group("Exact", "all are optional")
        ("-y", "--year",      "Specify year of release", "YEAR")
        ("-L", "--language",  "Specify text language", "LANG")
        ("-e", "--edition",   "Specify item edition", "EDITION")
        ("-E", "--extension", "Specify item extension", "EXT",
            valid_opts{"epub", "pdf", "djvu"})
        ("-i", "--isbn",      "Specify item ISBN", "ISBN");

    const auto misc = command_line::option_group("Miscellaneous")
        ("-h", "--help",      "Display this text and exit")
        ("-v", "--version",   "Print version information")
        ("-D", "--debug",     "Set logging level to debug");

    const command_line::groups groups = {main, excl, exact, misc};
    uint8_t exit_code = EXIT_SUCCESS;

    auto logger = logger::create("main");
    logger->set_pattern("%l: %v");
    logger->set_level(spdlog::level::err);
    spdlog::register_logger(logger);

    try {
        /* Parse command line arguments. */
        std::string progname = argv[0];
        std::vector<std::string> args(argv + 1, argv + argc);

        auto cli = cliparser::make(std::move(progname), std::move(groups));
        cli->process_arguments(args);

        if (cli->has("debug"))
            logger->set_level(spdlog::level::debug);

        logger->debug("the mighty eldwyrm has been summoned!");

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

        cli->validate_arguments();

    } catch (const command_line::argument_error &err) {
        logger->error(err.what() + std::string("; see --help"));
        exit_code = EXIT_FAILURE;
    } catch (const std::exception &err) {
        logger->error(err.what());
        exit_code = EXIT_FAILURE;
    }

    logger->debug("dropping all loggers and exiting with return value {}", exit_code);
    spdlog::drop_all();
    return exit_code;
}
