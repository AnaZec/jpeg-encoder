#pragma once

#include <iosfwd>
#include <string>

enum class LogLevel {
    Error = 0,
    Info = 1,
    Debug = 2
};

class Logger {
public:
    Logger(LogLevel level,
           std::ostream& infoStream,
           std::ostream& errorStream,
           std::ostream* reportStream = nullptr);

    static LogLevel defaultLevel();

    void setLevel(LogLevel level);
    void setReportStream(std::ostream* reportStream);

    void error(const std::string& message);
    void info(const std::string& message);
    void debug(const std::string& message);

    void separator();
    void stage(const std::string& stageName);

private:
    bool shouldLog(LogLevel messageLevel) const;
    void write(LogLevel messageLevel, const std::string& prefix, const std::string& message);

    LogLevel level_;
    std::ostream& infoStream_;
    std::ostream& errorStream_;
    std::ostream* reportStream_;
};