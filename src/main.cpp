#include "components/command_line.hpp"
#include "config.hpp"
#include <iostream>
#include <cstdint>
#include <exception>

using add_arg = command_line::option;

int
main(int argc, char *argv[])
{
    const command_line::options opts{
        add_arg("-h", "--help",      "Display this text and exit "),
        add_arg("-v", "--version",   "Print version information"),
        add_arg("-l", "--log",       "Set the logging verbosity (default: WARNING)", "LEVEL",
                {"error", "warning", "info", "trace"}),

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

    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
