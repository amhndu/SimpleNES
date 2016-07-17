#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <string>
#include <fstream>
#include <memory>

#define LOG(level) \
if (level > sn::Log::get().getLevel()) ; \
else sn::Log::get().getStream()

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
        void openFile(std::string path);
        Log& setLevel(Level level);
        Level getLevel();

        std::ostream& getStream()
        {
            return m_logFile;
        }

        static Log& get();
    private:
        Level m_logLevel;
        std::ofstream m_logFile;
        static std::unique_ptr<Log> m_instance;
    };
};
#endif // LOG_H
