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

#include <components/screen_butler.hpp>

namespace bookwyrm::tui {

std::shared_ptr<screen_butler> make_with(script_butler &butler, vector<py::module> &sources)
{
    auto tui = std::make_shared<screen_butler>(butler.results());
    butler.set_screens(tui);
    butler.async_search(sources); // Watch out, it's hot!
    return tui;
}

/* ns bookwyrm::tui */
}
