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

    std::ostream& Log::getCpuTraceStream()
    {
        return std::cout;//m_cpuTrace;
    }
    std::ostream& Log::getStream()
    {
        return m_logFile;
    }

    void Log::setLogFile(std::string path)
    {
        m_logFile.open(path);
    }

    void Log::setCpuTraceFile(std::string path)
    {
        m_cpuTrace.open(path);
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