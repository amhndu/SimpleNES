#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <cstring>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#define LOG(level) \
if (level > sn::Log::get().getLevel()) ; \
else sn::Log::get().getStream() << __FILENAME__ << ":" << std::dec << __LINE__ << " -- "

#define LOG_CPU \
if (sn::CpuTrace != sn::Log::get().getLevel()) ; \
else sn::Log::get().getCpuTraceStream()

namespace sn
{
    enum Level
    {
        Error,
        Info,
        CpuTrace
    };
    class Log
    {
    public:
        ~Log();
        void setLogFile(std::string path);
        void setCpuTraceFile(std::string path);
        Log& setLevel(Level level);
        Level getLevel();

        std::ostream& getStream();
        std::ostream& getCpuTraceStream();

        static Log& get();
    private:
        Level m_logLevel;
        std::ofstream m_logFile;
        std::ofstream m_cpuTrace;
        static std::unique_ptr<Log> m_instance;
    };
};
#endif // LOG_H
