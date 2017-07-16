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

#include <cstdlib>

#include "item.hpp"
#include "utils.hpp"
#include "version.hpp"
#include "python.hpp"
#include "components/command_line.hpp"
#include "components/script_butler.hpp"
#include "components/screen_butler.hpp"
#include "screens/multiselect_menu.hpp"

namespace py = pybind11;

int main(int argc, char *argv[])
{
    const auto main = cligroup("Main", "necessarily inclusive arguments; at least one required")
        ("-a", "--author",    "Specify authors",   "AUTHOR")
        ("-t", "--title",     "Specify title",     "TITLE")
        ("-s", "--serie",     "Specify serie",     "SERIE")
        ("-p", "--publisher", "Specify publisher", "PUBLISHER");

    const auto excl = cligroup("Exclusive", "cannot be combined with any other arguments")
        ("-d", "--ident",     "Specify an item identification (such as DOI, URL, etc.)", "IDENT");

    const auto exact = cligroup("Exact", "all are optional")
        ("-y", "--year",      "Specify year of release. "
                              "A prefix modifier can be used to broaden the search. "
                              "Available prefixes are <, >, <=, >=.", "YEAR")
        ("-L", "--language",  "Specify text language",  "LANG")
        ("-e", "--edition",   "Specify item edition",   "EDITION")
        ("-E", "--extension", "Specify item extension", "EXT",
            valid_opts{"epub", "pdf", "djvu"})
        ("-i", "--isbn",      "Specify item ISBN", "ISBN");

    const auto misc = cligroup("Miscellaneous")
        ("-h", "--help",      "Display this text and exit")
        ("-v", "--version",   "Print version information (" + build_info_short + ")")
        ("-D", "--debug",     "Set logging level to debug");

    const cligroups groups = {main, excl, exact, misc};

    auto logger = logger::create("main");
    logger->set_pattern("%l: %v");
    logger->set_level(spdlog::level::warn);
    spdlog::register_logger(logger);

    try {
        /* Parse command line arguments. */
        string progname = argv[0];
        vector<string> args(argv + 1, argv + argc);

        auto cli = cliparser::make(std::move(progname), std::move(groups));
        cli->process_arguments(args);

        if (cli->has("debug"))
            logger->set_level(spdlog::level::debug);

        logger->debug("the mighty eldwurm hath been summoned!");

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
        const auto err = utils::validate_download_dir(cli->get(0));
        if (err) {
            logger->error("invalid download directory: {}.", err.message());
            return EXIT_FAILURE;
        }

        /*
         * Start the Python interpreter and keep it alive until
         * program termination.
         */
        py::scoped_interpreter interp;

        /*
         * Find and load all source scripts.
         * During runtime (after .async_search() has been called),
         * the butler will match each found item with the wanted one.
         * If it doesn't match, it is discarded.
         *
         * The returned vector must exist throughout program execution.
         */
        const bookwyrm::item wanted(cli);
        auto butler = bookwyrm::script_butler(std::move(wanted));
        auto sources = butler.load_sources();

        auto tui = bookwyrm::tui::make_with(butler, sources);
        tui->display();

    } catch (const cli_error &err) {
        logger->error(err.what() + string("; see --help"));
        return EXIT_FAILURE;
    }

    logger->debug("terminating successfully...");
    return EXIT_SUCCESS;
}
