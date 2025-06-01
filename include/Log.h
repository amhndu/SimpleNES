#ifndef LOG_H
#define LOG_H
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#define LOG(level)                                                                                                     \
    if (level > sn::Log::get().getLevel())                                                                             \
        ;                                                                                                              \
    else                                                                                                               \
        sn::Log::get().getStream() << sn::log_timestamp << '[' << __FILENAME__ << ":" << std::dec << __LINE__ << "] "

#define LOG_CPU                                                                                                        \
    if (sn::CpuTrace != sn::Log::get().getLevel())                                                                     \
        ;                                                                                                              \
    else                                                                                                               \
        sn::Log::get().getCpuTraceStream()

#define VAR_PRINT(x) " \033[0;31m" << #x << "\033[0m=" << x

namespace sn
{
inline std::ostream& log_timestamp(std::ostream& out)
{
    auto timestamp_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
    return out << '[' << timestamp_ms << ']';
}

enum Level
{
    None,
    Error,
    Info,
    InfoVerbose,
    ApuTrace,
    CpuTrace
};
class Log
{
public:
    ~Log();
    void          setLogStream(std::ostream& stream);
    void          setCpuTraceStream(std::ostream& stream);
    Log&          setLevel(Level level);
    Level         getLevel();

    std::ostream& getStream();
    std::ostream& getCpuTraceStream();

    static Log&   get();

private:
    Level         m_logLevel;
    std::ostream* m_logStream;
    std::ostream* m_cpuTrace;
};

// Courtesy of http://wordaligned.org/articles/cpp-streambufs#toctee-streams
class TeeBuf : public std::streambuf
{
public:
    // Construct a streambuf which tees output to both input
    // streambufs.
    TeeBuf(std::streambuf* sb1, std::streambuf* sb2);

private:
    // This tee buffer has no buffer. So every character "overflows"
    // and can be put directly into the teed buffers.
    virtual int overflow(int c);
    // Sync both teed buffers.
    virtual int sync();

private:
    std::streambuf* m_sb1;
    std::streambuf* m_sb2;
};

class TeeStream : public std::ostream
{
public:
    // Construct an ostream which tees output to the supplied
    // ostreams.
    TeeStream(std::ostream& o1, std::ostream& o2);

private:
    TeeBuf m_tbuf;
};

};
#endif // LOG_H
