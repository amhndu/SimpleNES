#pragma once

namespace sn {
    // Modeled after NES timers; which have a period of (t+1) and count from t -> 0 -> t -> ...
    struct Divider {
        public:
            explicit Divider(int period) : period(period) {}

            bool clock() {
                if (counter == 0) {
                    counter = period;
                    return true;
                }

                counter -= 1;
                return false;
            }

            void reset(int p) {
                counter = period = p;
            }

            void reset() {
                counter = period;
            }

            int get_period() const {
                return period;
            }
        private:
            int period = 0;
            int counter = 0;
    };


}
