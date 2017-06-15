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

#include <termbox.h>
#include <spdlog/spdlog.h>
#include <pybind11/embed.h>

#include "components/menu.hpp"
#include "components/logger.hpp"

namespace py = pybind11;

namespace bookwyrm {

menu::menu(vector<item> &items)
    : items_(items)
{
    std::lock_guard<std::mutex> guard(menu_mutex_);

    int code = tb_init();
    if (code < 0) {
        spdlog::get("main")->error("termbox init failed with code: %d", code);
    }
}

menu::~menu()
{
    tb_shutdown();
}

void menu::display()
{
}

void menu::update()
{
}

} /* ns bookwyrm */
