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

#pragma once

#include <vector>
#include <string>

using std::string;
using std::vector;

namespace bookwyrm {

struct nonexacts_t {
    vector<string> authors;
    string title;
    string serie;
    string publisher;
    string journal;
};

struct exacts_t {
    int year;
    int edition;
    int ext; // enum this
    int volume;
    int number;
    int pages;
    string lang;
};

struct misc_t {
    vector<string> isbns;
    vector<string> mirrors;
};

class item {
public:
    explicit item() {}

    bool matches(const item &wanted);

private:
    nonexacts_t nonexacts;
    exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
