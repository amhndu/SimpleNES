#include "Mapper.h"
#include "PictureBus.h"

namespace sn
{
    class MapperAxROM : public Mapper
    {
    public:
        MapperAxROM(Cartridge &cart, std::function<void(void)> mirroring_cb);

        void writePRG(Address address, Byte value);
        Byte readPRG(Address address);

        Byte readCHR(Address address);
        void writeCHR(Address address, Byte value);

        NameTableMirroring getNameTableMirroring();

    private:
        NameTableMirroring m_mirroring;

        std::function<void(void)> m_mirroringCallback;
        Byte m_prgBank;
        std::vector<Byte> m_characterRAM;
    };
}
