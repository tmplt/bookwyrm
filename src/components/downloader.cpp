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

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "components/downloader.hpp"

namespace bookwyrm {

downloader::downloader(string download_dir)
    : dldir(download_dir)
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
    /* curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, downloader::progress_callback); */
    /* curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this); */

    /*
     * Assume there is a connection error and abort transfer with CURLE_OPERATION_TIMEDOUT
     * if the download speed is under 30B/s for 60s.
     */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60);

    /* Enable a verbose output. */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
}

downloader::~downloader()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

int downloader::progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    (void)clientp;
    (void)dltotal;
    (void)dlnow;
    (void)ultotal;
    (void)ulnow;

    return 0;
}

fs::path downloader::generate_filename(const bookwyrm::item &item)
{
    // TODO: remove hardcodedness
    string ext = ".txt";

    const fs::path base = dldir / fmt::format("{} - {} ({})", item.nonexacts.authors_str,
            item.nonexacts.title, item.exacts.year_str);

    /* If filename.ext doesn't exists, we use that. */
    if (auto candidate = base; !fs::exists(candidate.concat(ext)))
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
        candidate.concat(fmt::format(".{}{}", ++i, ext));
    } while (fs::exists(candidate));

    return candidate;
}

void downloader::sync_download(vector<bookwyrm::item> items)
{
    for (const auto &item : items) {
        auto filename = generate_filename(item);

        for (const auto &url : item.misc.uris) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            std::FILE *out = std::fopen(filename.c_str(), "wb");
            if (out == NULL) {
                /* TODO: test this output */
                throw component_error(fmt::format("unable to create this file: {}; reason: {}",
                            filename.string(), std::strerror(errno)));
            }
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

            if (auto res = curl_easy_perform(curl); res != CURLE_OK) {
                fmt::print(stderr, "error: item download failed: {}\n", curl_easy_strerror(res));
                std::fclose(out);
            } else {
                // now we have the remote file
                fmt::print("\t SUCCESS!\n");
                std::fclose(out);
                break;
            }
        }

        fmt::print(stderr, "error: no good sources for this item: {} - {} ({}). Sorry!", item.nonexacts.authors_str,
            item.nonexacts.title, item.exacts.year_str);
    }
}

}
