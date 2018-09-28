#include "Log.h"

namespace sn
{
    Log::~Log()
    {
    }

    Log& Log::get()
    {
        static Log instance;
        return instance;
    }

    std::ostream& Log::getCpuTraceStream()
    {
        return *m_cpuTrace;
    }
    std::ostream& Log::getStream()
    {
        return *m_logStream;
    }

    void Log::setLogStream(std::ostream& stream)
    {
        m_logStream = &stream;
    }

    void Log::setCpuTraceStream(std::ostream& stream)
    {
        m_cpuTrace = &stream;
    }

    Log& Log::setLevel(Level level)
    {
        m_logLevel = level;
        return *this;
    }

    Level Log::getLevel()
    {
        return m_logLevel;
    }



    TeeBuf::TeeBuf(std::streambuf * sb1, std::streambuf * sb2) :
        m_sb1(sb1),
        m_sb2(sb2)
    {}
    int TeeBuf::overflow(int c)
    {
        if (c == EOF)
        {
            return !EOF;
        }
        else
        {
            int const r1 = m_sb1->sputc(c);
            int const r2 = m_sb2->sputc(c);
            return r1 == EOF || r2 == EOF ? EOF : c;
        }
    }

    int TeeBuf::sync()
    {
        int const r1 = m_sb1->pubsync();
        int const r2 = m_sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }

    TeeStream::TeeStream(std::ostream& o1, std::ostream& o2) :
        std::ostream(&m_tbuf),
        m_tbuf(o1.rdbuf(), o2.rdbuf())
    {}

}
