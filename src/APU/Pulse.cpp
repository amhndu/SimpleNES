#include "APU/Pulse.hpp"
#include "APU/Divider.hpp"
#include "Cartridge.h"

namespace sn {

    class Sweep
    {
        public:
            void set(Byte registerValue);
            int clock(int channelPeriod);
            bool gate();
        private:
            bool m_enabled = false;
            //Divider m_divider = {};
            bool m_negate = false;
            int m_shift = 0;
            bool m_reload = false;

            int m_nextValue = -1;
    };

}
