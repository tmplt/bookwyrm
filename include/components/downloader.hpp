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

#include <curl/curl.h>
#include <experimental/filesystem>

#include "common.hpp"
#include "item.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm {

class downloader {
public:
    explicit downloader(string download_dir);
    ~downloader();

    /* Downloads the given items in a blocking, synchronous order. */
    void sync_download(vector<bookwyrm::item> items);
private:
    static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

    /* Generates a relative filename in dldir to save the given item. */
    fs::path generate_filename(const bookwyrm::item &item);

    const fs::path dldir;
    CURL *curl;
};

}
