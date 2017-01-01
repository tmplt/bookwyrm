#include <iostream>
#include "components/command_line.hpp"

int
main(int argc, char *argv[])
{
    const command_line::options opts{
        command_line::option("-t", "--test", "A testing flag")
    };

    std::string progname = argv[0];
    std::vector<std::string> args(argv + 1, argv + argc);

    cliparser::make_type cli = cliparser::make(std::move(progname), std::move(opts));

    cli->process_input(args);

    if (cli->has("test"))
        std::cout << "We're testing, alright!" << std::endl;
    else
        cli->usage();

    return 0;
}
