#include <curl/curl.h>
#include <experimental/filesystem>
#include <iostream>

#include "../common.hpp"
#include "core/item.hpp"
#include "time.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm {

    class progressbar {
    public:
        progressbar(bool use_unicode, bool use_colour) : use_unicode_(use_unicode), use_colour_(use_colour) {}

        inline void draw(unsigned int length, double fraction) { std::cout << build_bar(length, fraction); }

    private:
        string build_bar(unsigned int length, double fraction);

        const bool use_unicode_, use_colour_;
    };

    class downloader {
    public:
        explicit downloader(string download_dir);
        ~downloader();

        /*
         * Downloads the given items in a blocking, synchronous order.
         * Returns true if at least one item was downloaded.
         */
        bool sync_download(vector<core::item> items, vector<py::module> &plugins);

        auto resolve_mirror(const string &mirror, const core::item &item, vector<py::module> &plugins);

        time::timer timer;
        progressbar pbar;

    private:
        static int
        progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

        /* Generates a relative filename in dldir to save the given item. */
        fs::path generate_filename(const core::item &item);

        const fs::path dldir;
        CURL *curl;
    };

} // namespace bookwyrm
