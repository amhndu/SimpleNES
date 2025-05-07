#pragma once

namespace sn
{

class Irq
{
public:
    virtual void pull()    = 0;
    virtual void release() = 0;
};

}
