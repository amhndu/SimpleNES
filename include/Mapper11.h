#include "Mapper.h"

namespace sn
{
    class Mapper11 : public Mapper
    {
    public:
        Mapper11(Cartridge &cart, std::function<void(void)> mirroring_cb);
        NameTableMirroring getNameTableMirroring();
        void writePRG(Address address, Byte value);
        Byte readPRG(Address address);
        const Byte *getPagePtr(Address address);
        Byte readCHR(Address address);
        void writeCHR(Address address, Byte value);

    private:
        NameTableMirroring m_mirroring;
        Byte prgbank;
        Byte chrbank;
        std::function<void(void)> m_mirroringCallback;
    };
}
