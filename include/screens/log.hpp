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

#include <spdlog/details/log_msg.h>

#include "screens/base.hpp"

namespace screen {

class log : public base {
public:
    explicit log();

    bool action(const key &key, const uint32_t &ch) override;
    void update() override;
    void on_resize() override;
    string footer_info() const override;
    int scrollperc() const override;

    string footer_controls() const override
    {
        return "[j/k d/u]Navigation [SPACE]attach/detach";
    }

    void log_entry(spdlog::level::level_enum level, string entry);

private:
    using entry_t = std::pair<spdlog::level::level_enum, string>;
    vector<entry_t> entries_;

    void print_entry(int &y, entry_t &entry);
    static colour to_colour(spdlog::level::level_enum e);
};

/* ns screen */
}
