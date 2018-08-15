/*
 * Runs a single plugin with debug logs.
 * Arguments:
 *  #1: the plugin to load and run
 *  #2: path to pybookwyrm Python dynamic library
 */

#include <iostream>
#include "core/plugin_handler.hpp"

using namespace bookwyrm;

int main(int argc, char *argv[])
{
    const std::vector<std::string> args(argv + 1, argv + argc);
    assert(args.size() == 2);
    const std::string plugin_path = args[0], library_path = args[1];

    const core::item wanted;
    core::options opts;
    opts.plugin_paths = {{ plugin_path }};
    opts.library_path = library_path;

    try {
        auto ph = core::plugin_handler(std::move(wanted), true, std::move(opts));
        ph.load_plugins();
        ph.async_search();
        ph.wait();
    } catch (const std::runtime_error &e) {
        std::cerr << "error: " << e.what();
    }
}
