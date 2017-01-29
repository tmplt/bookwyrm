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
    using add_arg = command_line::option;
    using add_group = command_line::option_group;

    const auto main = add_group("Main", "necessarily inclusive arguments; at least one required", {
        add_arg("-a", "--author",    "Specify authors", "AUTHOR"),
        add_arg("-t", "--title",     "Specify title", "TITLE"),
        add_arg("-s", "--serie",     "Specify serie", "SERIE"),
        add_arg("-p", "--publisher", "Specify publisher", "PUBLISHER")
    });

    const auto excl = add_group("Exclusive", "cannot be combined with any other arguments", {
        add_arg("-d", "--ident",     "Specify an item identification (such as DOI, URL, etc.)", "IDENT")
    });

    const auto exact = add_group("Exact", "matched fuzzily; all are optional", {
        add_arg("-y", "--year",      "Specify year of release", "YEAR"),
        add_arg("-L", "--language",  "Specify text language", "LANG"),
        add_arg("-e", "--edition",   "Specify item edition", "EDITION"),
        add_arg("-E", "--extension", "Specify item extension", "EXT",
                {"epub", "pdf", "djvu"}),
        add_arg("-i", "--isbn",      "Specify item ISBN", "ISBN")
    });

    const auto misc = add_group("Misc", "miscellaneous arguments", {
        add_arg("-h", "--help",      "Display this text and exit "),
        add_arg("-v", "--version",   "Print version information"),
        add_arg("-l", "--log",       "Set logging level to info"),
    });

    const command_line::groups groups = {main, excl, exact, misc};
    uint8_t exit_code = EXIT_SUCCESS;

    auto logger = logger::create("main");
    logger->set_pattern("%l: %v");
    logger->set_level(spdlog::level::err);

    try {
        /* Parse command line arguments. */
        std::string progname = argv[0];
        std::vector<std::string> args(argv + 1, argv + argc);

        auto cli = cliparser::make(std::move(progname), std::move(groups));
        cli->process_arguments(args);

        if (cli->has("log"))
            logger->set_level(spdlog::level::info);

        logger->info("the mighty eldwyrm has been summoned!");

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
        logger->error(err.what() + string("; see --help"));
        exit_code = EXIT_FAILURE;
    } catch (const std::exception &err) {
        logger->error(err.what());
        exit_code = EXIT_FAILURE;
    }

    logger->info("dropping all loggers and returning exitval = {}", exit_code);
    spdlog::drop_all();
    return exit_code;
}
