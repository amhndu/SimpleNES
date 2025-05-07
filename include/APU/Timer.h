#pragma once
#include <chrono>


namespace sn {
    using namespace std::chrono;

    struct Timer {

    public:
        explicit Timer(nanoseconds period) : period(period) {}
        const nanoseconds period;

        // clock the timer and return number of periods elapsed
        int clock(nanoseconds elapsed) {
            leftover += elapsed;
            if (leftover < elapsed) {
                return 0;
            }

            auto cycles = leftover / period;
            leftover = leftover % period;
            return cycles;
        }
    private:
        nanoseconds leftover = nanoseconds(0);
    };

}
