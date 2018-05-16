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

    inline void reset()
    {
        last_update = clock::now();
    }

    inline double ms_since_last_update()
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

    inline int hours() const
    {
        return hours_;
    }

    inline int minutes() const
    {
        return minutes_;
    }

    inline int seconds() const
    {
        return seconds_;
    }

private:
    int hours_, minutes_, seconds_;
};

}
