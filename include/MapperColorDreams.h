#ifndef MAPPERCOLORDREAMS_H_INCLUDED
#define MAPPERCOLORDREAMS_H_INCLUDED

#include "Mapper.h"

namespace sn
{
    class MapperColorDreams : public Mapper
    {
    public:
        MapperColorDreams(Cartridge &cart, std::function<void(void)> mirroring_cb);
        NameTableMirroring getNameTableMirroring();
        void writePRG(Address address, Byte value);
        Byte readPRG(Address address);

        Byte readCHR(Address address);
        void writeCHR(Address address, Byte value);

    private:
        NameTableMirroring m_mirroring;
        Byte prgbank;
        Byte chrbank;
        std::function<void(void)> m_mirroringCallback;
    };
}

#endif // MAPPERCOLORDREAMS_H_INCLUDED
