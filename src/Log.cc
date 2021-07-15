// -*- LSST-C++ -*-
/*
 * LSST Data Management System
 * Copyright 2013-2014 LSST Corporation.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */

/**
 * @file Log.cc
 * @brief LSST DM logging module built on log4cxx.
 *
 * @author Bill Chickering
 * Contact: chickering@cs.stanford.edu
 *
 */

// System headers
#include <mutex>
#include <pthread.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Third-party headers
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/helpers/bytearrayinputstream.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>

// Local headers
#include "lsst/log/Log.h"
#include "lwpID.h"


// Max message length for varargs/printf style logging
#define MAX_LOG_MSG_LEN 1024

namespace {

// name of the env. variable pointing to logging config file
const char configEnv[] = "LSST_LOG_CONFIG";

// dafault message layout pattern
const char layoutPattern[] = "%c %p: %m%n";

/*
 * Configure LOG4CXX from file, file must exist. If file extension is .xml
 * then DOMConfigurator is used, otherwise PropertyConfigurator is called.
 *
 * If file parsing fails then error messages are printed to standard error,
 * but execution continues. LOG4CXX will likely stay un-configured in this
 * case.
 */
void configFromFile(std::string const& filename) {
    // find position of extension
    size_t dotpos = filename.find_last_of(".");
    if (dotpos != std::string::npos && filename.compare(dotpos, std::string::npos, ".xml") == 0) {
        log4cxx::xml::DOMConfigurator::configure(filename);
    } else {
        log4cxx::PropertyConfigurator::configure(filename);
    }
}

/*
 *  Default configuration.
 *
 *  If LSST_LOG_CONFIG envvar is defined and points to existing file then
 *  use that file to configure (can be either XML or Properties file).
 *  Otherwise use pre-defined configuration - add console appender to root
 *  logger using pattern layout with the above pattern, level set to INFO.
 *
 *  IF LOG4CXX was initialized already and you want to reset it then call
 *  `log4cxx::BasicConfigurator::resetConfiguration()` first.
 */
void defaultConfig() {
    // if LSST_LOG_CONFIG is set then use that file
    if (const char* env = getenv(::configEnv)) {
        if (env[0] and access(env, R_OK) == 0) {
            configFromFile(env);
            return;
        }
    }

    // use pre-defined configuration
    log4cxx::LogString pattern(layoutPattern);
    log4cxx::LayoutPtr layout(new log4cxx::PatternLayout(pattern));
    log4cxx::AppenderPtr appender(new log4cxx::ConsoleAppender(layout));
    auto root = log4cxx::Logger::getRootLogger();
    root->addAppender(appender);
    root->setLevel(log4cxx::Level::getInfo());
}

// Protects concurrent configuration
std::mutex configMutex;

// global initialization flag (protected by configMutex)
bool initialized = false;

/*
 * This method is called exactly once to initialize LOG4CXX configuration.
 * If `initialized` is set to true then default configuration is skipped.
 */
log4cxx::LoggerPtr log4cxxInit() {

    std::lock_guard<std::mutex> lock(configMutex);
    if (!initialized) {
        initialized = true;
        // do default configuration if no one done any configuration yet
        ::defaultConfig();
    }

    // returns root logger to be used as default logger
    return log4cxx::Logger::getRootLogger();
}

// List of the MDC initialization functions
std::vector<std::function<void()>> mdcInitFunctions;
std::mutex mdcInitMutex;

// more efficient alternative to pthread_once
struct PthreadKey {
    PthreadKey() {
        // we don't need destructor for a key
        pthread_key_create(&key, nullptr);
    }
    pthread_key_t key;
} pthreadKey;

} // namespace


namespace lsst {
namespace log {


// Log class

/**
 *  Returns default LOG4CXX logger.
 */
log4cxx::LoggerPtr const& Log::_defaultLogger() {

    // initialize on the first call (skips initialization if someone else did that)
    static log4cxx::LoggerPtr _default(::log4cxxInit());

    return _default;
}

/** Explicitly configures log4cxx and initializes logging system.
  *
  * Configuration can be specified via environment variable LSST_LOG_CONFIG,
  * if it is set and specifies existing file name then this file name is
  * used for configuration. Otherwise pre-defined configuration is used,
  * which is hardwired to add to the root logger a ConsoleAppender. In this
  * case, the output will be formatted using a PatternLayout set to the
  * pattern "%c %p: %m%n".
  */
void Log::configure() {
    std::lock_guard<std::mutex> lock(::configMutex);

    // Make sure other threads know that default configuration is not needed
    ::initialized = true;

    // This removes all defined appenders, resets level to DEBUG,
    // existing loggers are not deleted, only reset.
    log4cxx::BasicConfigurator::resetConfiguration();

    // Do default configuration (only if not configured already?)
    ::defaultConfig();
}

/** Configures log4cxx from specified file.
  *
  * If file name ends with ".xml", it is passed to
  * log4cxx::xml::DOMConfigurator::configure(). Otherwise, it assumed to be
  * a log4j Java properties file and is passed to
  * log4cxx::PropertyConfigurator::configure(). See
  * http://logging.apache.org/log4cxx/usage.html for additional details.
  *
  * @param filename  Path to configuration file.
  */
void Log::configure(std::string const& filename) {
    std::lock_guard<std::mutex> lock(::configMutex);

    // Make sure other threads know that default configuration is not needed
    ::initialized = true;

    // This removes all defined appenders, resets level to DEBUG,
    // existing loggers are not deleted, only reset.
    log4cxx::BasicConfigurator::resetConfiguration();

    ::configFromFile(filename);
}

/** Configures log4cxx using a string containing the list of properties,
  * equivalent to configuring from a file containing the same content
  * but without creating temporary files.
  *
  * @param properties  Configuration properties.
  */
void Log::configure_prop(std::string const& properties) {
    std::lock_guard<std::mutex> lock(::configMutex);

    // Make sure other threads know that default configuration is not needed
    ::initialized = true;

    // This removes all defined appenders, resets level to DEBUG,
    // existing loggers are not deleted, only reset.
    log4cxx::BasicConfigurator::resetConfiguration();

    std::vector<unsigned char> data(properties.begin(), properties.end());
    log4cxx::helpers::InputStreamPtr inStream(new log4cxx::helpers::ByteArrayInputStream(data));
    log4cxx::helpers::Properties prop;
    prop.load(inStream);
    log4cxx::PropertyConfigurator::configure(prop);
}

/** Get the logger name associated with the Log object.
  * @return String containing the logger name.
  */
std::string Log::getName() const {
    std::string name = _logger->getName();
    if (name == "root") {
        name.clear();
    }
    return name;
}

/** Returns logger object for a given name.
  *
  * If name is empty then current logger is returned and not
  * a root logger.
  *
  * @param loggername  Name of logger to return.
  * @return Log instance corresponding to logger name.
  */
Log Log::getLogger(std::string const& loggername) {
    if (loggername.empty()){
        return getDefaultLogger();
    } else {
        return Log(log4cxx::Logger::getLogger(loggername));
    }
}

/** Places a KEY/VALUE pair in the Mapped Diagnostic Context (MDC) for the
  * current thread. The VALUE may then be included in log messages by using
  * the following the `X` conversion character within a pattern layout as
  * `%X{KEY}`. Note that unlike `log4cxx::MDC::put()` this method overwrites
  * any previously existing mapping.
  *
  * @param key    Unique key.
  * @param value  String value.
  * @return Previous value for the key in the MDC.
  */
std::string Log::MDC(std::string const& key, std::string const& value) {
    // put() does not remove existing mapping, to make it less confusing
    // for clients which expect that MDC() always overwrites existing mapping
    // we explicitly remove it first if it exists.
    std::string const oldValue = log4cxx::MDC::get(key);
    log4cxx::MDC::remove(key);
    log4cxx::MDC::put(key, value);
    return oldValue;
}

/** Remove the value associated with KEY within the MDC.
  *
  * @param key  Key identifying value to remove.
  */
void Log::MDCRemove(std::string const& key) {
    log4cxx::MDC::remove(key);
}

int Log::MDCRegisterInit(std::function<void()> function) {

    std::lock_guard<std::mutex> lock(mdcInitMutex);

    // logMsg may have been called already in this thread, to make sure that
    // this function is executed in this thread call it explicitly
    function();

    // store function for later use
    ::mdcInitFunctions.push_back(std::move(function));

    // return arbitrary number
    return 1;
}

/** Set the logging threshold to LEVEL.
  *
  * @param level   New logging threshold.
  */
void Log::setLevel(int level) {
    _logger->setLevel(log4cxx::Level::toLevel(level));
}

/** Retrieve the logging threshold.
  * @return int Indicating the logging threshold.
  */
int Log::getLevel() const {
    log4cxx::LevelPtr level = _logger->getLevel();
    int levelno = -1;
    if (level != NULL) {
        levelno = level->toInt();
    }
    return levelno;
}

/** Retrieve the effective logging threshold.
  * @return int Indicating the effective logging threshold.
  */
int Log::getEffectiveLevel() const {
    log4cxx::LevelPtr level = _logger->getEffectiveLevel();
    int levelno = -1;
    if (level != NULL) {
        levelno = level->toInt();
    }
    return levelno;
}

/** Return whether the logging threshold of the logger is less than or equal
  * to LEVEL.
  * @return Bool indicating whether or not logger is enabled.
  *
  * @param level   Logging threshold to check.
  */
bool Log::isEnabledFor(int level) const {
    if (_logger->isEnabledFor(log4cxx::Level::toLevel(level))) {
        return true;
    } else {
        return false;
    }
}

/**
  * Return a logger which is a descendant to this logger.
  *
  * If for example name of this logger is "main.task" and suffix is
  * "subtask1.algorithm" then this method will return logger with the name
  * "main.task.subtask1.algorithm". If this logger is root logger then
  * suffix name is used for returned logger name. If suffix is empty
  * then this instance is returned.
  *
  * @param suffix Suffix for tha name of returned logger, can include dot
  *               (but not at leading position) and can be empty.
  * @return Log instance.
 */
Log Log::getChild(std::string const& suffix) const {
    // strip leading dots and spaces from suffix
    auto pos = suffix.find_first_not_of(" .");
    if (pos == std::string::npos) {
        // empty, just return myself
        return *this;
    }
    std::string name = getName();
    if (name.empty()) {
        name = suffix.substr(pos);
    } else {
        name += '.';
        name += suffix.substr(pos);
    }
    return getLogger(name);
}

/** Method used by LOG_INFO and similar macros to process a log message
  * with variable arguments along with associated metadata.
  */
void Log::log(log4cxx::LevelPtr level,     ///< message level
              log4cxx::spi::LocationInfo const& location,  ///< message origin location
              char const* fmt,             ///< message format string
              ...                          ///< message arguments
             ) {
    va_list args;
    va_start(args, fmt);
    char msg[MAX_LOG_MSG_LEN];
    vsnprintf(msg, MAX_LOG_MSG_LEN, fmt, args);
    logMsg(level, location, msg);
}

/** Method used by LOGS_INFO and similar macros to process a log message.
  */
void Log::logMsg(log4cxx::LevelPtr level,     ///< message level
                 log4cxx::spi::LocationInfo const& location,  ///< message origin location
                 std::string const& msg       ///< message string
                 ) {

    // do one-time per-thread initialization, this was implemented
    // with thread_local initially but clang on OS X did not support it
    void *ptr = pthread_getspecific(::pthreadKey.key);
    if (ptr == nullptr) {

        // use pointer value as a flag, don't care where it points to
        ptr = static_cast<void*>(&::pthreadKey);
        pthread_setspecific(::pthreadKey.key, ptr);

        std::lock_guard<std::mutex> lock(mdcInitMutex);
		// call all functions in the MDC init list
        for (auto& fun: mdcInitFunctions) {
            fun();
        }
    }

    // forward everything to logger
    _logger->forcedLog(level, msg, location);
}

unsigned lwpID() {
    return detail::lwpID();
}

}} // namespace lsst::log
