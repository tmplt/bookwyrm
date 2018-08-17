/*
 * Runs a bunch of worker plugins and exits after half a second.
 * Arguments:
 *  #1: the directory where the plugins reside
 *  #2: path to pybookwyrm Python dynamic library
 */

#include <unistd.h>
#include <iostream>
#include <chrono>
#include "core/plugin_handler.hpp"

using namespace bookwyrm;
using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    /* Handle arguments */
    const std::vector<std::string> args(argv + 1, argv + argc);
    assert(args.size() == 2);
    const std::string plugin_path = args[0], library_path = args[1];

    /* Setup execution */
    const core::item wanted;
    core::options opts;
    opts.plugin_paths = {{ plugin_path }};
    opts.library_path = library_path;

    try {
        auto ph = core::plugin_handler(std::move(wanted), true, std::move(opts));
        ph.load_plugins();
        ph.async_search();

        /* Let plugins start working first before exiting. */
        std::this_thread::sleep_for(500ms);
    } catch (const std::runtime_error &e) {
        std::cerr << "error: " << e.what();
        return EXIT_FAILURE;
    }
}
