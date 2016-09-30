#include "Mapper.h"
#include "MapperNROM.h"
#include "MapperUxROM.h"
#include "MapperCNROM.h"

namespace sn
{
    std::unique_ptr<Mapper> Mapper::createMapper(Mapper::Type mapper_t, sn::Cartridge& cart)
    {
        std::unique_ptr<Mapper> ret(nullptr);
        switch (mapper_t)
        {
            case NROM:
                ret.reset(new MapperNROM(cart));
                break;
            case UxROM:
                ret.reset(new MapperUxROM(cart));
                break;
            case CNROM:
                ret.reset(new MapperCNROM(cart));
                break;
            default:
                break;
        }
        return ret;
    }
}