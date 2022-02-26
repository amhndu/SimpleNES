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
        const Byte *getPagePtr(Address address);
        NameTableMirroring getNameTableMirroring();
        Byte readCHR(Address address);
        void writeCHR(Address address, Byte value);

       

    private:
        NameTableMirroring m_mirroring;
        PictureBus m_ppubus;
        std::function<void(void)> m_mirroringCallback;
        Byte trainer;
        Byte base = (trainer ? 512 : 0);
        Byte PRGBank;
        std::vector<Byte> m_characterRAM;
    };
}