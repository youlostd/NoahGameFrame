#ifndef METIN2_SPDLOG_HPP
#define METIN2_SPDLOG_HPP

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifndef SPDLOG_ACTIVE_LEVEL_USER_LEVEL
#   undef SPDLOG_ACTIVE_LEVEL
#   ifdef _DEBUG
#       define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#   else
#       define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#   endif
#else
#   undef SPDLOG_ACTIVE_LEVEL
#   define SPDLOG_ACTIVE_LEVEL SPDLOG_ACTIVE_LEVEL_USER_LEVEL
#endif


#ifdef SPDLOG_LOGGER_CALL
#undef SPDLOG_LOGGER_CALL
#endif
#define SPDLOG_LOGGER_CALL(logger, level, ...) (logger)->log(level, __VA_ARGS__)

#ifdef SPDLOG_TRACE
#undef SPDLOG_TRACE
#undef SPDLOG_LOGGER_TRACE
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#define SPDLOG_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define SPDLOG_TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_TRACE(logger, ...) (void)0
#define SPDLOG_TRACE(...) (void)0
#endif

#ifdef SPDLOG_DEBUG
#undef SPDLOG_DEBUG
#undef SPDLOG_LOGGER_DEBUG
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define SPDLOG_LOGGER_DEBUG(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::debug, __VA_ARGS__)
#define SPDLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_DEBUG(logger, ...) (void)0
#define SPDLOG_DEBUG(...) (void)0
#endif

#ifdef SPDLOG_INFO
#undef SPDLOG_INFO
#undef SPDLOG_LOGGER_INFO
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#define SPDLOG_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#define SPDLOG_INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_INFO(logger, ...) (void)0
#define SPDLOG_INFO(...) (void)0
#endif

#ifdef SPDLOG_WARN
#undef SPDLOG_WARN
#undef SPDLOG_LOGGER_WARN
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#define SPDLOG_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#define SPDLOG_WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_WARN(logger, ...) (void)0
#define SPDLOG_WARN(...) (void)0
#endif

#ifdef SPDLOG_ERROR
#undef SPDLOG_ERROR
#undef SPDLOG_LOGGER_ERROR
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#define SPDLOG_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define SPDLOG_ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_ERROR(logger, ...) (void)0
#define SPDLOG_ERROR(...) (void)0
#endif

#ifdef SPDLOG_CRITICAL
#undef SPDLOG_CRITICAL
#undef SPDLOG_LOGGER_CRITICAL
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define SPDLOG_LOGGER_CRITICAL(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#define SPDLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define SPDLOG_LOGGER_CRITICAL(logger, ...) (void)0
#define SPDLOG_CRITICAL(...) (void)0
#endif






#endif /* METIN2_SPDLOG_HPP */
