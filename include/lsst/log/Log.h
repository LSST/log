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
 * @file Log.h
 * @brief LSST DM logging module built on log4cxx.
 *
 * @author Bill Chickering
 * Contact: chickering@cs.stanford.edu
 *
 */

#ifndef LSST_LOG_LOG_H
#define LSST_LOG_LOG_H


// System headers
#include <functional>
#include <sstream>
#include <stdarg.h>
#include <string>
#include <vector>

// Third-party headers
#include <log4cxx/logger.h>
#include <boost/format.hpp>

/**
  * @def LOG_CONFIG(filename)
  * Configures log4cxx and initializes logging module.
  *
  * @param filename  Path to configuration file.
  */
#define LOG_CONFIG(filename) lsst::log::Log::configure(filename)

/**
  * @def LOG_CONFIG_PROP(string)
  * Configures log4cxx from a string containing list of properties.
  * This is equivalent to configuring with a file name containing the same
  * data as a string.
  *
  * @note Use of this macro will likely produce hard-coded configuration
  * which is not advised for general-use code. It may be useful where
  * pre-defined hard-coded configuration is necessary, e.g. in unit tests.
  *
  * @param string  List of properties (lines separated by new line character)
  */
#define LOG_CONFIG_PROP(string) lsst::log::Log::configure_prop(string)

/**
  * @def LOG_DEFAULT_NAME()
  * Get the current default logger name. Returns empty string for root logger.
  * @return String containing the default logger name.
  */
#define LOG_DEFAULT_NAME() lsst::log::Log::getDefaultLoggerName()

/**
  * @def LOG_GET(logger)
  * Returns a Log object associated with logger.
  * @return Log object corresponding to logger.
  *
  * @param logger  Either a logger name or a Log object.
  */
#define LOG_GET(logger) lsst::log::Log::getLogger(logger)

/**
  * @def LOG_PUSHCTX(name)
  * Pushes name onto the global hierarchical default logger name.
  * Note that we only allow simple non-dotted names to be used for
  * context names, multi-level context name (e.g. "componen1.component2")
  * will result in exception. Empty context names are disallowed as
  * well, exception will be raised for empty name.
  *
  * @note Call to this macro is not thread-safe, moreover context is
  * global and applies to all threads (which means you want to avoid
  * using this in multi-threaded applications).
  *
  * @param name  String to push onto logging context.
  * @throw std::invalid_argument raised for empty name or when name contains dot.
  */
#define LOG_PUSHCTX(name) lsst::log::Log::pushContext(name)

/**
  * @def LOG_POPCTX()
  * Pops the last pushed name off the global hierarchical default logger
  * name.
  *
  * @note Call to this macro is not thread-safe.
  */
#define LOG_POPCTX() lsst::log::Log::popContext()

/**
  * @def LOG_MDC(key, value)
  * Places a KEY/VALUE pair in the Mapped Diagnostic Context (MDC) for the
  * current thread. The VALUE may then be included in log messages by using
  * the following the `X` conversion character within a pattern layout as
  * `%X{KEY}`.
  *
  * @param key    Unique key.
  * @param value  String value.
  */
#define LOG_MDC(key, value) lsst::log::Log::MDC(key, value)

/**
  * @def LOG_MDC_REMOVE(key)
  * Remove the value associated with KEY within the MDC.
  *
  * @param key  Key identifying value to remove.
  */
#define LOG_MDC_REMOVE(key) lsst::log::Log::MDCRemove(key)

/**
  * @def LOG_MDC_INIT(function)
  * Register function for initialization of MDC. This function will be called
  * for current thread and every new thread (but not for other existing
  * threads) before any message is logged using one of the macros below. Its
  * main purpose is to initialize MDC (using LOG_MDC macro). In some cases the
  * function may be called more than once per thread.
  *
  * This macro is thread safe, but typically it will be called from main
  * thread before any other LOG macro.
  *
  * Macro returns an integer number, the value is not specified, but this
  * allows it to be used in one-time initialization constructs like:
  *
  *     @code
  *     static int dummyMdcInit = LOG_MDC_INIT(some_init_func);
  *     @endcode
  *
  * @param func Any function object which takes no arguments and returns void.
  */
#define LOG_MDC_INIT(func) lsst::log::Log::MDCRegisterInit(std::function<void()>(func))

/**
  * @def LOG_SET_LVL(logger, level)
  * Set the logging threshold for LOGGER to LEVEL.
  *
  * @param logger  Logger with threshold to adjust.
  * @param level   New logging threshold.
  */
#define LOG_SET_LVL(logger, level) \
    lsst::log::Log::getLogger(logger).setLevel(level)

/**
  * @def LOG_GET_LVL(logger)
  * Retrieve the logging threshold for LOGGER.
  * @return int Indicating the logging threshold.
  *
  * @param logger  Either a logger name or a Log object with threshold
  *                to return.
  */
#define LOG_GET_LVL(logger) \
    lsst::log::Log::getLogger(logger).getLevel()

/**
  * @def LOG_CHECK_LVL(logger, level)
  * Return whether the logging threshold of LOGGER is less than or equal
  * to LEVEL.
  * @return Bool indicating whether or not logger is enabled.
  *
  * @param logger  Either a logger name or a Log object being queried.
  * @param level   Logging threshold to check.
  */
#define LOG_CHECK_LVL(logger, level) \
    lsst::log::Log::getLogger(logger).isEnabledFor(level)

/**
  * @def LOG_CHECK_TRACE()
  * Return whether the logging threshold of the default logger is less
  * than or equal to TRACE.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_TRACE() \
    LOG4CXX_UNLIKELY(lsst::log::Log::getDefaultLogger().isTraceEnabled())

/**
  * @def LOG_CHECK_DEBUG()
  * Return whether the logging threshold of the default logger is less
  * than or equal to DEBUG.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_DEBUG() \
    LOG4CXX_UNLIKELY(lsst::log::Log::getDefaultLogger().isDebugEnabled())

/**
  * @def LOG_CHECK_INFO()
  * Return whether the logging threshold of the default logger is less
  * than or equal to INFO.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_INFO() \
        lsst::log::Log::getDefaultLogger().isInfoEnabled()

/**
  * @def LOG_CHECK_VERBOSE()
  * Return whether the logging threshold of the default logger is less
  * than or equal to VERBOSE.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_VERBOSE() \
        lsst::log::Log::getDefaultLogger().isVerboseEnabled()

/**
  * @def LOG_CHECK_WARN()
  * Return whether the logging threshold of the default logger is less
  * than or equal to WARN.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_WARN() \
        lsst::log::Log::getDefaultLogger().isWarnEnabled()

/**
  * @def LOG_CHECK_ERROR()
  * Return whether the logging threshold of the default logger is less
  * than or equal to ERROR.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_ERROR() \
        lsst::log::Log::getDefaultLogger().isErrorEnabled()

/**
  * @def LOG_CHECK_FATAL()
  * Return whether the logging threshold of the default logger is less
  * than or equal to FATAL.
  * @return Bool indicating whether or not logger is enabled.
  */
#define LOG_CHECK_FATAL() \
        lsst::log::Log::getDefaultLogger().isFatalEnabled()

/**
  * @def LOG(logger, level, message...)
  * Log a message using a varargs/printf style interface.
  *
  * @param logger      Either a logger name or a Log object.
  * @param level       Logging level associated with message.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG(logger, level, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isEnabledFor(level)) { \
            log.log(log4cxx::Level::toLevel(level), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_TRACE(message...)
  * Log a trace-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_TRACE(message...) \
    do { \
        lsst::log::Log log; \
        if (LOG4CXX_UNLIKELY(log.isTraceEnabled())) { \
            log.log(log4cxx::Level::getTrace(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_DEBUG(message...)
  * Log a debug-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_DEBUG(message...) \
    do { \
        lsst::log::Log log; \
        if (LOG4CXX_UNLIKELY(log.isDebugEnabled())) { \
            log.log(log4cxx::Level::getDebug(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_INFO(message...)
  * Log a info-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_INFO(message...) \
    do { \
        lsst::log::Log log; \
        if (log.isInfoEnabled()) { \
            log.log(log4cxx::Level::getInfo(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_VERBOSE(message...)
  * Log a info-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_VERBOSE(message...) \
    do { \
        lsst::log::Log log; \
        if (log.isVerboseEnabled()) { \
            log.log(log4cxx::Level::getVerbose(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_WARN(message...)
  * Log a warn-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_WARN(message...) \
    do { \
        lsst::log::Log log; \
        if (log.isWarnEnabled()) { \
            log.log(log4cxx::Level::getWarn(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_ERROR(message...)
  * Log a error-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_ERROR(message...) \
    do { \
        lsst::log::Log log; \
        if (log.isErrorEnabled()) { \
            log.log(log4cxx::Level::getError(), LOG4CXX_LOCATION, message); } \
    } while (false)

/**
  * @def LOG_FATAL(message...)
  * Log a fatal-level message to the default logger using a varargs/printf
  * style interface.
  *
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOG_FATAL(message...) \
    do { \
        lsst::log::Log log; \
        if (log.isFatalEnabled()) { \
            log.log(log4cxx::Level::getFatal(), LOG4CXX_LOCATION, message); } \
    } while (false)


// small internal utility macro, not for regular clients
#define LOG_MESSAGE_VIA_STREAM_(logger, level, message) \
    std::ostringstream stream_; \
    stream_ << message; \
    logger.logMsg(level, LOG4CXX_LOCATION, stream_.str())

/**
  * @def LOGS(logger, level, message)
  * Log a message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS("logger", LOG_LVL_DEBUG, "coordinates: x=" << x << " y=" << y);`.
  * Usual caveat regarding commas inside macro arguments applies to
  * message argument.
  *
  * @param logger   Either a logger name or a Log object.
  * @param level    Logging level associated with message.
  * @param message  Message to be logged.
  */
#define LOGS(logger, level, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isEnabledFor(level)) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::toLevel(level), message); \
        } \
    } while (false)

/**
  * @def LOGS_TRACE(message)
  * Log a trace-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_TRACE("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_TRACE(message) \
    do { \
        lsst::log::Log log; \
        if (LOG4CXX_UNLIKELY(log.isTraceEnabled())) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getTrace(), message); \
        } \
    } while (false)

/**
  * @def LOGS_DEBUG(message)
  * Log a debug-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_DEBUG("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_DEBUG(message) \
    do { \
        lsst::log::Log log; \
        if (LOG4CXX_UNLIKELY(log.isDebugEnabled())) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getDebug(), message); \
        } \
    } while (false)

/**
  * @def LOGS_INFO(message)
  * Log a info-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_INFO("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_INFO(message) \
    do { \
        lsst::log::Log log; \
        if (log.isInfoEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getInfo(), message); \
        } \
    } while (false)

/**
  * @def LOGS_VERBOSE(message)
  * Log verbose-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_VERBOSE("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_VERBOSE(message) \
    do { \
        lsst::log::Log log; \
        if (log.isVerboseEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getVerbose(), message); \
        } \
    } while (false)

/**
  * @def LOGS_WARN(message)
  * Log a warning-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_WARN("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_WARN(message) \
    do { \
        lsst::log::Log log; \
        if (log.isWarnEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getWarn(), message); \
        } \
    } while (false)

/**
  * @def LOGS_ERROR(message)
  * Log a error-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_ERROR("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_ERROR(message) \
    do { \
        lsst::log::Log log; \
        if (log.isErrorEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getError(), message); \
        } \
    } while (false)

/**
  * @def LOGS_FATAL(message)
  * Log a fatal-level message to the default logger using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGS_FATAL("coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param message Message to be logged.
  */
#define LOGS_FATAL(message) \
    do { \
        lsst::log::Log log; \
        if (log.isFatalEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getFatal(), message); \
        } \
    } while (false)

/**
  * @def LOGL_TRACE(logger, message...)
  * Log a trace-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_TRACE(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (LOG4CXX_UNLIKELY(log.isTraceEnabled())) { \
            log.log(log4cxx::Level::getTrace(), LOG4CXX_LOCATION, message);\
        } \
    } while (false)

/**
  * @def LOGL_DEBUG(logger, message...)
  * Log a debug-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_DEBUG(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (LOG4CXX_UNLIKELY(log.isDebugEnabled())) { \
            log.log(log4cxx::Level::getDebug(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGL_INFO(logger, message...)
  * Log a info-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_INFO(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isInfoEnabled()) { \
            log.log(log4cxx::Level::getInfo(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGL_VERBOSE(logger, message...)
  * Log a verbose-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_VERBOSE(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isVerboseEnabled()) { \
            log.log(log4cxx::Level::getVerbose(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGL_WARN(logger, message...)
  * Log a warn-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_WARN(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isWarnEnabled()) { \
            log.log(log4cxx::Level::getWarn(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGL_ERROR(logger, message...)
  * Log a error-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_ERROR(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isErrorEnabled()) { \
            log.log(log4cxx::Level::getError(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGL_FATAL(logger, message...)
  * Log a fatal-level message using a varargs/printf style interface.
  *
  * @param logger   Either a logger name or a Log object.
  * @param message  An sprintf-compatible format string followed by zero,
  *                    one, or more comma-separated arguments.
  */
#define LOGL_FATAL(logger, message...) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isFatalEnabled()) { \
            log.log(log4cxx::Level::getFatal(), LOG4CXX_LOCATION, message); \
        } \
    } while (false)

/**
  * @def LOGLS_TRACE(logger, message)
  * Log a trace-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_TRACE(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_TRACE(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (LOG4CXX_UNLIKELY(log.isTraceEnabled())) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getTrace(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_DEBUG(logger, message)
  * Log a debug-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_DEBUG(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_DEBUG(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (LOG4CXX_UNLIKELY(log.isDebugEnabled())) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getDebug(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_INFO(logger, message)
  * Log a info-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_INFO(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_INFO(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isInfoEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getInfo(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_VERBOSE(logger, message)
  * Log a verbose-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_VERBOSE(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_VERBOSE(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isVerboseEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getVerbose(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_WARN(logger, message)
  * Log a warn-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_WARN(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_WARN(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isWarnEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getWarn(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_ERROR(logger, message)
  * Log a error-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_ERROR(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_ERROR(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isErrorEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getError(), message); \
        } \
    } while (false)

/**
  * @def LOGLS_FATAL(logger, message)
  * Log a fatal-level message using an iostream-based interface.
  *
  * Message is any expression which can appear on the right side of the
  * stream insertion operator, e.g.
  * `LOGLS_FATAL(logger, "coordinates: x=" << x << " y=" << y);`. Usual caveat regarding
  * commas inside macro arguments applies to message argument.
  *
  * @param logger  Either a logger name or a Log object.
  * @param message Message to be logged.
  */
#define LOGLS_FATAL(logger, message) \
    do { \
        lsst::log::Log log(lsst::log::Log::getLogger(logger)); \
        if (log.isFatalEnabled()) { \
            LOG_MESSAGE_VIA_STREAM_(log, log4cxx::Level::getFatal(), message); \
        } \
    } while (false)

#define LOG_LVL_TRACE static_cast<int>(log4cxx::Level::TRACE_INT)
#define LOG_LVL_DEBUG static_cast<int>(log4cxx::Level::DEBUG_INT)
#define LOG_LVL_INFO static_cast<int>(log4cxx::Level::INFO_INT)
#define LOG_LVL_VERBOSE static_cast<int>(log4cxx::Level::VERBOSE_INT)
#define LOG_LVL_WARN static_cast<int>(log4cxx::Level::WARN_INT)
#define LOG_LVL_ERROR static_cast<int>(log4cxx::Level::ERROR_INT)
#define LOG_LVL_FATAL static_cast<int>(log4cxx::Level::FATAL_INT)

#define LOG_LOGGER lsst::log::Log
#define LOG_CTX lsst::log::LogContext

namespace lsst {
namespace log {

/** This static class includes a variety of methods for interacting with the
  * the logging module. These methods are not meant for direct use. Rather,
  * they are used by the LOG* macros and the SWIG interface declared in
  * logInterface.h.
  */
class Log {
public:

    /***
     *  Default constructor creates an instance of "current" logger.
     *
     *  Initially current logger is the same as root logger, but
     *  current can be changed via pushContext()/popContext().
     */
    Log() : _logger(_defaultLogger()) { }

    /**
     *  Check whether the logger is enabled for the DEBUG Level
     */
    bool isDebugEnabled() const { return _logger->isDebugEnabled(); }
    /**
     *  Check whether the logger is enabled for the ERROR Level
     */
    bool isErrorEnabled() const { return _logger->isErrorEnabled(); }
    /**
     *  Check whether the logger is enabled for the FATAL Level
     */
    bool isFatalEnabled() const { return _logger->isFatalEnabled(); }
    /**
     *  Check whether the logger is enabled for the INFO Level
     */
    bool isInfoEnabled() const { return _logger->isInfoEnabled(); }
    /**
     *  Check whether the logger is enabled for the VERBOSE Level
     */
    bool isVerboseEnabled() const { return _logger->isVerboseEnabled(); }
    /**
     *  Check whether the logger is enabled for the TRACE Level
     */
    bool isTraceEnabled() const { return _logger->isTraceEnabled(); }
    /**
     *  Check whether the logger is enabled for the WARN Level
     */
    bool isWarnEnabled() const { return _logger->isWarnEnabled(); }

    std::string getName() const;
    void setLevel(int level);
    int getLevel() const;
    bool isEnabledFor(int level) const;

    /// Return default logger instance, same as default constructor.
    static Log getDefaultLogger() { return Log(_defaultLogger()); }
    static std::string getDefaultLoggerName();

    static void configure();
    static void configure(std::string const& filename);
    static void configure_prop(std::string const& properties);

    static Log getLogger(Log const& logger) { return logger; }
    static Log getLogger(std::string const& loggername);

    static void pushContext(std::string const& name);
    static void popContext();
    static void MDC(std::string const& key, std::string const& value);
    static void MDCRemove(std::string const& key);
    static int MDCRegisterInit(std::function<void()> function);

    void log(log4cxx::LevelPtr level,
             log4cxx::spi::LocationInfo const& location,
             char const* fmt, ...);
    void logMsg(log4cxx::LevelPtr level,
                log4cxx::spi::LocationInfo const& location,
                std::string const& msg);

private:

    /**
     *  Returns default LOG4CXX logger.
     *
     *  @param newDefault if non-zero then default is set to this value first.
     */
    static log4cxx::LoggerPtr const& _defaultLogger(log4cxx::LoggerPtr const& newDefault=log4cxx::LoggerPtr());

    /**
     *  Construct a Log using a LOG4CXX logger.
     *
     *  The default constructor is called to ensure the default logger is initialized and LOG4CXX is configured.
     */
    Log(log4cxx::LoggerPtr const& logger) : Log() { _logger = logger; }

    log4cxx::LoggerPtr _logger;
};

/** This class handles the default logger name of a logging context.
  */
class LogContext {
public:
    /** Create a logging context associated with a default logger name
      * constructed by pushing \p name onto the pre-existing hierarchical default
      * logger name. See comment to \c LOG_PUSHCTX about allowed names.
      *
      * @param name  String to push onto logging context.
      */
    explicit LogContext(std::string const& name) {
        Log::pushContext(name);
    }
    ~LogContext() {
        Log::popContext();
    }

private:
    // cannot copy instances
    LogContext(const LogContext&);
    LogContext& operator=(const LogContext&);
};


/**
 * Function which returns LWP ID on platforms which support it.
 *
 * On all other platforms a small incremental integer number (counting number
 * of threads) is returned. This function can be used to produce more
 * human-friendly thread ID for logging instead of regular %t format.
 */
unsigned lwpID();

}} // namespace lsst::log

#endif // LSST_LOG_LOG_H
