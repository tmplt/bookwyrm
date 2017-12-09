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

#include <chrono>

namespace bookwyrm::time {

/* A simple ms-counting timer that can be reset. */
class timer {
    typedef std::chrono::high_resolution_clock clock;
public:
    explicit timer()
    {
        reset();
    }

    void reset()
    {
        last_update = clock::now();
    }

    double ms_since_last_update()
    {
        const auto time_now = clock::now();
        std::chrono::duration<double, std::milli> ms = time_now - last_update;
        return ms.count();
    }

private:
    std::chrono::time_point<clock> last_update;
};

/*
 * A time duration based on seconds, with an interface alike Boost's posix time library.
 * Takes a duration of seconds and divides that up in hours, minutes and seconds.
 */
class duration {
public:
    explicit duration(long duration)
    {
        const auto unit_count = [&duration](long unit) {
            auto count = duration / unit;
            duration   = duration % unit;

            return count;
        };

        hours_   = unit_count(3600);
        minutes_ = unit_count(60);
        seconds_ = duration; // remaining seconds
    }

    int hours() const
    {
        return hours_;
    }

    int minutes() const
    {
        return minutes_;
    }

    int seconds() const
    {
        return seconds_;
    }

private:
    int hours_, minutes_, seconds_;
};

}
