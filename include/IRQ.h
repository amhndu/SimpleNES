#pragma once

namespace sn
{

class IRQHandle
{
public:
    virtual void pull()    = 0;
    virtual void release() = 0;
};

}
