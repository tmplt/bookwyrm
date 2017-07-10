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

#include <screens/multiselect_menu.hpp>

namespace bookwyrm::menu {

std::shared_ptr<multiselect_menu> create_with(script_butler &butler)
{
    auto menu = std::make_shared<multiselect_menu>(butler.results());
    butler.set_menu(menu);
    butler.async_search(); // Watch out, it's hot!
    return menu;
}

/* ns bookwyrm::menu */
}
