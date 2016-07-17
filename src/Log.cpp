#include "Log.h"

namespace sn
{
    std::unique_ptr<Log> Log::m_instance = nullptr;

    Log::~Log()
    {
        m_logFile.flush();
    }

    Log& Log::get()
    {
        if (!m_instance)
            m_instance.reset(new Log);
        return *m_instance;
    }

    void Log::openFile(std::string path)
    {
        m_logFile.open(path);
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

}