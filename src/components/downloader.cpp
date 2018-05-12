#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <sys/ioctl.h>
#include <iomanip>
#include <sstream>
#include <unistd.h>

#include <fmt/ostream.h>

#include "downloader.hpp"
#include "../runes.hpp"
#include "../string.hpp"

namespace bookwyrm {

downloader::downloader(string download_dir)
    : pbar(true, true), dldir(download_dir)
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) throw component_error("curl could not initialize");

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
           "Mozilla/5.0 (X11; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0");

    /* Complete the connection phase within 30s. */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);

    /* Consider HTTP codes >=400 as errors. This option is NOT fail-safe. */
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    /* Set callback function for writing data. */
    /* curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloader::write_data); */

    /* Set callback function for progress metering. */
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, downloader::progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    /*
     * Assume there is a connection error and abort transfer with CURLE_OPERATION_TIMEDOUT
     * if the download speed is under 30B/s for 60s.
     */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60);

    /* Enable a verbose output. */
    /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); */
    /* curl_easy_setopt(curl,CURLOPT_MAX_RECV_SPEED_LARGE, 1024 * 50); */

    /* std::cout << rune::vt100::hide_cursor; */
}

downloader::~downloader()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    /* std::cout << rune::vt100::show_cursor; */
}

fs::path downloader::generate_filename(const core::item &item)
{
    fs::path base = dldir / fmt::format("{} - {}",
            vector_to_string(item.nonexacts.authors),
            item.nonexacts.title);

    if (item.exacts.year != core::empty) {
        base += fmt::format("({})", item.exacts.year);
    }

    const auto valid_candidate = [](fs::path p) {
        return !fs::exists(p);
    };

    /* If filename.ext doesn't exists, we use that. */
    if (auto candidate = base; valid_candidate(candidate.concat("." + item.exacts.extension)))
        return candidate;

    /*
     * Otherwise, we generate a new filename on the form
     * filename.n.ext, which doesn't already exists, and where
     * n is an unsigned integer.
     *
     * That is, the filename chain generated is:
     *   filename.1.ext
     *   filename.2.ext
     *   filename.3.ext
     *   etc.
     */

    size_t i = 0;
    fs::path candidate;

    do {
        candidate = base;
        candidate.concat(fmt::format(".{}.{}", ++i, item.exacts.extension));
    } while (!valid_candidate(candidate));

    return candidate;
}

bool downloader::sync_download(vector<core::item> items)
{
    bool any_success = false;

    for (const auto &item : items) {
        auto filename = generate_filename(item);
        bool success = false;

        int mirror = 1;
        for (const auto &url : item.misc.uris) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::FILE *out = std::fopen(filename.c_str(), "wb");
            if (out == NULL) {
                /* TODO: test this output */
                throw component_error(fmt::format("unable to create this file: {}; reason: {}",
                            filename.string(), std::strerror(errno)));
            }
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

            if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
                fmt::print(stderr, "{}error: item download (mirror {}) failed: {} (CURLcode = {})\n",
                        rune::vt100::erase_line, mirror++, curl_easy_strerror(res), res);

                std::fclose(out);
                if (fs::file_size(filename) == 0) {
                    /* TODO: warn the user about file removal (or just warn that the file is empty?) */
                    fs::remove(filename);
                }

            } else {
                std::fclose(out);
                any_success = success = true;

                /* That source worked, so there is no need to download from the others. */
                break;
            }
        }

        if (!success) {
            fmt::print(stderr, "error: no good sources for this item: {} - {} ({}). Sorry!\n",
                vector_to_string(item.nonexacts.authors),
                item.nonexacts.title, item.exacts.year);
        }
    }

    return any_success;
}

int downloader::progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    /* We never upload anything. */
    std::ignore = ulnow;
    std::ignore = ultotal;

    /*
     * dltotal: the size of the file being downloaded (in bytes)
     * dlnow:   how much have been downloaded thus far (in bytes)
     */
    downloader *d = static_cast<downloader*>(clientp);

    double rate;
    if (curl_easy_getinfo(d->curl, CURLINFO_SPEED_DOWNLOAD, &rate) != CURLE_OK)
        rate = std::numeric_limits<double>::quiet_NaN();

    if (d->timer.ms_since_last_update() >= 100 || dlnow == dltotal) {
        d->timer.reset();

        time::duration eta((dltotal - dlnow) / rate);
        std::stringstream eta_ss;
        if (eta.hours() > 0) {
            eta_ss << eta.hours() << "h "
                   << std::setfill('0') << std::setw(2) << eta.minutes() << "m "
                   << std::setfill('0') << std::setw(2) << eta.seconds() << "s";
        } else if (eta.minutes() > 0) {
            eta_ss << eta.minutes() << "m "
                   << std::setfill('0') << std::setw(2) << eta.seconds() << "s";
        } else {
            eta_ss << eta.seconds() << "s";
        }

        /* Download rate unit conversion. */
        const string rate_unit = [&rate]() {
            constexpr auto k = std::pow(2, 10),
                           M = std::pow(2, 20);

            if (rate > M) {
                rate /= M;
                return "MB/s";
            } else {
                rate /= k;
                return "kB/s";
            }
        }();

        const double fraction = static_cast<double>(dlnow) / static_cast<double>(dltotal);
        fmt::print("{}\r  {:.0f}% ", rune::vt100::erase_line, fraction * 100);

        string status_text = fmt::format(" {dlnow:.2f}/{dltotal:.2f}MB @ {rate:.2f}{unit} ETA: {eta}\r",
                fmt::arg("dlnow",   static_cast<double>(dlnow) / std::pow(2, 20)),
                fmt::arg("dltotal", static_cast<double>(dltotal) / std::pow(2, 20)),
                fmt::arg("rate",    rate),
                fmt::arg("unit",    rate_unit),
                fmt::arg("eta",     eta_ss.str()));

        /* Draw the progress bar. */
        const int term_width = []() {
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            return w.ws_col;
        }();

        int bar_length = 26,
            min_bar_length = 5,
            status_text_length = status_text.length() + 6;

        if (status_text_length + bar_length > term_width)
            bar_length -= status_text_length + bar_length - term_width;

        /* Don't draw the progress bar if length is less than min_bar_length. */
        if (bar_length >= min_bar_length)
            d->pbar.draw(bar_length, fraction);

        std::cout << status_text << std::flush;
    }

    return 0;
}

string progressbar::build_bar(unsigned int length, double fraction)
{
    std::ostringstream ss;

    /* Validation */
    if (!std::isnormal(fraction) || fraction < 0.0)
        fraction = 0.0;
    else if (fraction > 1.0)
        fraction = 1.0;

    const double bar_part = fraction * length;

    /* How many whole unicode characters should we print? */
    const unsigned int whole_bar_chars = std::floor(bar_part);

    /* If the bar isn't full, which unicode character should we use? */
    const unsigned int partial_bar_char_idx = std::floor((bar_part - whole_bar_chars) * 8.0);

    using namespace rune;

    if (use_colour_)
        ss << vt100::bar_colour;

    /* The left border */
    ss << (use_unicode_ ? bar::unicode::left_border : bar::left_border);

    /* The filled-in part */
    unsigned i = 0;
    for (; i < whole_bar_chars; i++)
        ss << (use_unicode_ ? bar::unicode::fraction[8] : bar::tick);

    if (i < length) {
        /* The last filled in part, if needed. */
        ss << (use_unicode_ ? bar::unicode::fraction[partial_bar_char_idx]
                : bar::empty_fill);
    }

    /* The not-filled-in part */
    for (i = whole_bar_chars + 1; i < length; i++)
        ss << bar::empty_fill;

    /* The bar's right border */
    ss << (use_unicode_ ? bar::unicode::right_border : bar::right_border);

    if (use_colour_)
        ss << vt100::reset_colour;

    return ss.str();
}

}
