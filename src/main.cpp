#include <unistd.h>
#include <cerrno>
#include <clocale>
#include <system_error>

#include "core/plugin_handler.hpp"
#include "core/item.hpp"
#include "version.hpp"
#include "components/command_line.hpp"
#include "tui/tui.hpp"
#include "components/downloader.hpp"

using namespace bookwyrm;

static std::error_code validate_download_dir(const fs::path &path)
{
    constexpr auto error = [](auto ec) -> std::error_code {
        return {ec, std::generic_category()};
    };

    if (!fs::exists(path))
        return error(ENOENT);

    if (fs::space(path).available == 0)
        return error(ENOSPC);

    if (!fs::is_directory(path))
        return error(ENOTDIR);

    /* Can we write to the directory? */
    if (access(path.c_str(), W_OK) != 0)
        return error(EACCES);

    return {};
}

static const core::item create_item(const cliparser &cli)
{
    const core::nonexacts_t ne(
        cli.get_many("authors"),
        cli.get("title"),
        cli.get("series"),
        cli.get("publisher"),
        cli.get("journal")
    );

    const auto yearmod = [&cli]() -> std::pair<core::year_mod, int> {
        const auto year_str = cli.get("year");
        if (year_str.empty()) return {core::year_mod::equal, core::empty};

        const auto start = std::find_if(year_str.cbegin(), year_str.cend(), [](char c) {
            return std::isdigit(c);
        });

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
                core::year_mod mod;

                if (mod_str == "=>")
                    mod = core::year_mod::eq_gt;
                else if (mod_str == "=<")
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
    }();

    const auto parse_number = [&cli](const string &&opt) -> int {
        const auto value_str = cli.get(opt);
        if (value_str.empty()) return core::empty;

        try {
            return std::stoi(value_str);
        } catch (std::exception &err) {
            throw value_error("malformed value '" + value_str + "' for argument --" + opt);
        }
    };

    const core::exacts_t e(
        yearmod,
        parse_number("volume"),
        parse_number("number"),
        cli.get("extension")
    );

    const core::item item(std::move(ne), std::move(e));

    return std::move(item);
}

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "");

    const auto main = cligroup("Main", "necessarily inclusive arguments; at least one required")
        ("-a", "--author",     "Specify authors",   "AUTHOR")
        ("-t", "--title",      "Specify title",     "TITLE")
        ("-s", "--series",     "Specify series",     "SERIE")
        ("-p", "--publisher",  "Specify publisher", "PUBLISHER");

    const auto excl = cligroup("Exclusive", "cannot be combined with any other arguments")
        ("-d", "--ident",      "Specify an item identification (such as DOI, URL, etc.)", "IDENT");

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
        ("-D", "--debug",      "Set logging level to debug");

    const cligroups groups = {main, excl, exact, misc};

    const auto cli = [=]() -> cliparser {
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
    }();

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

    try {
        cli.validate_arguments();
    } catch (const argument_error &err) {
        fmt::print(stderr, "error: {}; see --help\n", err.what());
        return EXIT_FAILURE;
    }

    const string dl_path = cli.has(0) ? cli.get(0) : ".";

    if (const auto err = validate_download_dir(dl_path); err) {
        string msg = err.message();
        std::transform(msg.begin(), msg.end(), msg.begin(), ::tolower);
        fmt::print(stderr, "error: invalid download directory: {}.\n", msg);
        return EXIT_FAILURE;
    }

    vector<core::item> wanted_items;

    try {
        const core::item wanted = create_item(cli);
        auto ph = core::plugin_handler(std::move(wanted), cli.has("debug"));

        ph.load_plugins();
        auto ui = std::make_shared<tui::tui>(ph.results(), cli.has("debug"));
        ph.set_frontend(ui);
        ph.async_search();

        if (ui->display())
            wanted_items = ui->get_wanted_items();

    } catch (const component_error &err) {
        fmt::print(stderr, "A dependency failed: {}. Developer error? Terminating...\n", err.what());
        return EXIT_FAILURE;
    } catch (const program_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    } catch (const std::experimental::filesystem::filesystem_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    }

    if (wanted_items.empty()) {
        /* We have nothing else to do. */
        return EXIT_SUCCESS;
    }

    try {
        bookwyrm::downloader d(dl_path);

        if (wanted_items.size() == 1)
            fmt::print("Downloading item...\n");
        else
            fmt::print("Downloading {} items...\n", wanted_items.size());

        auto success = d.sync_download(wanted_items);

        if (!success && wanted_items.size() > 1) {
            fmt::print("No items were successfully downloaded\n");
            return EXIT_FAILURE;
        }

    } catch (const component_error &err) {
        fmt::print(stderr, "Fatal program error: {}; I can't continue! Terminating...\n", err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
