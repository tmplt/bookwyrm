#include <cerrno>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include <system_error>
#include <unistd.h>

#include "components/command_line.hpp"
#include "components/downloader.hpp"
#include "core/item.hpp"
#include "core/plugin_handler.hpp"
#include "prefix.hpp"
#include "tui/tui.hpp"
#include "version.hpp"

using namespace bookwyrm;

static std::error_code validate_download_dir(const fs::path &path)
{
    constexpr auto error = [](auto ec) -> std::error_code { return {ec, std::generic_category()}; };

    /* Is the file a directory? */
    if (!fs::is_directory(path))
        return error(ENOTDIR);

    /* Does the directory exist? */
    if (!fs::exists(path))
        return error(ENOENT);

    /* Is there space available? */
    if (fs::space(path).available == 0)
        return error(ENOSPC);

    /* Can we write to the directory? */
    if (access(path.c_str(), W_OK) != 0)
        return error(EACCES);

    return {};
}

static const core::item create_item(const cliparser &cli)
{
    const core::nonexacts_t ne(
        cli.get_many("authors"), cli.get("title"), cli.get("series"), cli.get("publisher"), cli.get("journal"));

    const auto yearmod = std::invoke([&cli]() -> std::pair<core::year_mod, int> {
        const auto year_str = cli.get("year");
        if (year_str.empty())
            return {core::year_mod::equal, core::empty};

        const auto start = std::find_if(year_str.cbegin(), year_str.cend(), [](char c) { return std::isdigit(c); });

        try {
            /*
             * NOTE: this approach allows the year to be represented as a float
             * (which stoi truncates to an int) and allows appended not-digits.
             * Will this cause problems?
             */
            const auto year = std::stoi(string(start, year_str.cend()));

            if (start != year_str.cbegin()) {
                /* There is a modifier in front of the year. */
                string mod_str(year_str.cbegin(), start);
                core::year_mod mod = core::year_mod::equal;

                if (mod_str == ">=")
                    mod = core::year_mod::eq_gt;
                else if (mod_str == "<=")
                    mod = core::year_mod::eq_lt;
                else if (mod_str == ">")
                    mod = core::year_mod::gt;
                else if (mod_str == "<")
                    mod = core::year_mod::lt;
                else
                    throw value_error("unrecognised year modifier '" + mod_str + '\'');

                return {mod, year};
            }

            return {core::year_mod::equal, year};

        } catch (const std::exception &err) {
            throw value_error("malformed year");
        }
    });

    const auto parse_number = [&cli](const string &&opt) -> int {
        const auto value_str = cli.get(opt);
        if (value_str.empty())
            return core::empty;

        try {
            return std::stoi(value_str);
        } catch (std::exception &err) {
            throw value_error("malformed value '" + value_str + "' for argument --" + opt);
        }
    };

    const core::exacts_t e(yearmod, parse_number("volume"), parse_number("number"), cli.get("extension"));

    const core::item item(std::move(ne), std::move(e));

    return item;
}

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "");

    /* Install a rudimentary SIGINT handler */
    struct sigaction int_handler;
    int_handler.sa_handler = [](int) {
        std::cout << std::endl;
        std::exit(EXIT_FAILURE);
    };
    sigemptyset(&int_handler.sa_mask);
    int_handler.sa_flags = 0;
    sigaction(SIGINT, &int_handler, NULL);

    /* Define command line options */
    // clang-format off
    const auto main = cligroup("Main", "necessarily inclusive arguments; at least one required")
        ("-a", "--author",     "Specify authors",   "AUTHOR")
        ("-t", "--title",      "Specify title",     "TITLE")
        ("-s", "--series",     "Specify series",     "SERIE")
        ("-p", "--publisher",  "Specify publisher", "PUBLISHER");
    const auto excl = cligroup("Exclusive", "cannot be combined with any other arguments");
    const auto exact = cligroup("Exact", "all are optional")
        ("-y", "--year",       "Specify year of release. "
                               "A prefix modifier can be used to broaden the search. "
                               "Available prefixes are <, >, <=, >=.", "YEAR")
        ("-L", "--language",   "Specify text language",  "LANG")
        ("-e", "--edition",    "Specify item edition",   "EDITION")
        ("-E", "--extension",  "Specify item extension", "EXT")
        ("-i", "--isbn",       "Specify item ISBN", "ISBN");
    const auto misc = cligroup("Miscellaneous")
        ("-h", "--help",       "Display this text and exit")
        ("-v", "--version",    "Print version information (" + build_info_short + ") and exit")
        ("-D", "--debug",      "Set logging level to debug")
        ("-A", "--accuracy", "Set searching accuracy in percentage (default: 75)", "ACCURACY");
    // clang-format on

    /* Construct a command line parser */
    const cligroups groups = {main, excl, exact, misc};
    const auto cli = std::invoke([=]() -> cliparser {
        string progname = argv[0];
        vector<string> args(argv + 1, argv + argc);

        auto cli = cliparser::make(std::move(progname), std::move(groups));

        try {
            cli.process_arguments(args);
        } catch (const argument_error &err) {
            fmt::print(stderr, "error: {}; see --help\n", err.what());

            // evil! Return optional instead?
            std::exit(EXIT_FAILURE);
        }

        return cli;
    });

    /* Check for --help */
    if (cli.has("help")) {
        cli.usage();
        return EXIT_SUCCESS;
    } else if (cli.has("version")) {
        print_build_info();
        return EXIT_SUCCESS;
    } else if (argc == 1) {
        cli.usage();
        return EXIT_FAILURE;
    }

    /* Validate arguments */
    try {
        cli.validate_arguments();
    } catch (const argument_error &err) {
        fmt::print(stderr, "error: {}; see --help\n", err.what());
        return EXIT_FAILURE;
    }

    /* Validate download path */
    const string dl_path = cli.has(0) ? cli.get(0) : ".";
    if (const auto err = validate_download_dir(dl_path); err) {
        string msg = err.message();
        std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
        fmt::print(stderr, "error: invalid download directory: {}.\n", msg);
        return EXIT_FAILURE;
    }

    /* Start bookwyrm */
    std::optional<vector<core::item>> wanted_items;
    try {
        /* Construct options */
        const core::item wanted = create_item(cli);
        core::options opts;
#ifdef DEBUG
        /* bookwyrm must be run from build/ in DEBUG mode. */
        opts.plugin_paths = {{fs::canonical(fs::path("../src/plugins"))}};
#else
        /* Check $XDG_CONFIG_HOME or $HOME/.config/bookwyrm also. */
        opts.plugin_paths = {{fs::canonical(fs::path(std::string(INSTALL_PREFIX) + "/share/bookwyrm/plugins"))}};
#endif
        opts.accuracy = cli.has("accuracy") ? std::stoi(cli.get("accuracy")) : 75;
        opts.library_path = fmt::format("{}/usr/lib", INSTALL_PREFIX);

        /* Construct and start the plugin handler. */
        auto ph = std::make_shared<core::plugin_handler>(std::move(wanted), cli.has("debug"), std::move(opts));

        /*
         * Load plugins, search asynchronously, and wait until at least one item has been found
         * (or until all plugins have finished running).
         */
        ph->load_plugins();
        ph->async_search();
        ph->wait_for_item();

        /* Display the UI, getting wanted items if any where found and selected. */
        std::vector<core::log_pair> unread_logs;
        if (ph->items() != 0) {
            auto ui = std::make_shared<tui::tui>(ph, cli.has("debug"));
            ph->set_frontend(ui);

            wanted_items = ui->get_wanted_items();
            unread_logs = ui->unread_logs();
        }

        ph->clear_frontend();

        /* Dump unread logs to stderr */
        for (const auto &[lvl, msg] : unread_logs) {
            std::ignore = lvl;
            fmt::print(stderr, "{}\n", msg);
        }

        if (ph->items() == 0) {
            fmt::print(stderr, "Unable to find any items\n");
            return EXIT_FAILURE;
        }

        if (!wanted_items) {
            /* We have nothing else to do. */
            return EXIT_SUCCESS;
        }

        /* Download wanted selected items. */
        bookwyrm::downloader d(dl_path);

        if (wanted_items->size() == 1)
            fmt::print(stderr, "Downloading item...\n");
        else
            fmt::print(stderr, "Downloading {} items...\n", wanted_items->size());

        auto plugins = ph->get_plugins();
        ph->clear_nogil();
        const auto success = d.sync_download(*wanted_items, plugins);
        if (!success) {
            fmt::print(stderr, "No items were successfully downloaded.\n");
            return EXIT_FAILURE;
        }

    } catch (const component_error &err) {
        fmt::print(stderr, "A dependency failed: {}. Developer error? Terminating...\n", err.what());
        return EXIT_FAILURE;
    } catch (const program_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    } catch (const std::experimental::filesystem::filesystem_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
