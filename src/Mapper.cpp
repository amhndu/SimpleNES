#include "Mapper.h"
#include "MapperNROM.h"

namespace sn
{
    std::unique_ptr<Mapper> Mapper::createMapper(Mapper::Type mapper_t, sn::Cartridge& cart)
    {
        std::unique_ptr<Mapper> ret;
        switch (mapper_t)
        {
            case NROM:
                ret.reset(new MapperNROM(cart));
                break;
            default:
                ret.reset(nullptr);
        }
        return ret;
    }
}