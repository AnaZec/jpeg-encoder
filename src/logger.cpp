#include "logger.hpp"

#include <iostream>
#include <ostream>

Logger::Logger(LogLevel level,
               std::ostream& infoStream,
               std::ostream& errorStream,
               std::ostream* reportStream)
    : level_(level),
      infoStream_(infoStream),
      errorStream_(errorStream),
      reportStream_(reportStream) {}

LogLevel Logger::defaultLevel() {
#ifdef NDEBUG
    return LogLevel::Error;
#else
    return LogLevel::Info;
#endif
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

void Logger::setReportStream(std::ostream* reportStream) {
    reportStream_ = reportStream;
}

void Logger::error(const std::string& message) {
    write(LogLevel::Error, "[ERROR] ", message);
}

void Logger::info(const std::string& message) {
    write(LogLevel::Info, "[INFO] ", message);
}

void Logger::debug(const std::string& message) {
    write(LogLevel::Debug, "[DEBUG] ", message);
}

void Logger::separator() {
    info("**************************************************\n");
}

void Logger::stage(const std::string& stageName) {
    info(stageName + "\n");
}

bool Logger::shouldLog(LogLevel messageLevel) const {
    return static_cast<int>(messageLevel) <= static_cast<int>(level_);
}

void Logger::write(LogLevel messageLevel,
                   const std::string& prefix,
                   const std::string& message) {
    if (!shouldLog(messageLevel)) {
        return;
    }

    std::ostream& stream =
        (messageLevel == LogLevel::Error) ? errorStream_ : infoStream_;

    stream << prefix << message;

    if (reportStream_) {
        *reportStream_ << prefix << message;
    }
}