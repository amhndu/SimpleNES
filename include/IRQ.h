#pragma once

namespace sn
{
class IRQ
{
    virtual void pullIRQ()    = 0;
    virtual void releaseIRQ() = 0;
};
}
