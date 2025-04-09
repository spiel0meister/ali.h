#ifndef ALI2_H
#define ALI2_H

#include <stdio.h>
#include <stdint.h>

#define ali_static_assert(expr) _Static_assert(expr, #expr)

#ifndef ALI_TYPE_ALIASES
#define ALI_TYPE_ALIASES

typedef int8_t ali_i8;
typedef int16_t ali_i16;
typedef int32_t ali_i32;
typedef int64_t ali_i64;

typedef uint8_t ali_u8;
typedef uint16_t ali_u16;
typedef uint32_t ali_u32;
typedef uint64_t ali_u64;

typedef ali_i64 ali_isize;
typedef ali_u64 ali_usize;

#endif // ALI_TYPE_ALIASES

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_COUNT_,
}AliLogLevel;

typedef struct {
    AliLogLevel level;
    FILE* f;
}AliLogger;

extern AliLogger ali_libc_logger;

void ali_log_log(AliLogger* logger, AliLogLevel level, const char* fmt, ...);
#define ali_log_debug(logger, ...) ali_log_log(logger, LOG_DEBUG, __VA_ARGS__)
#define ali_log_info(logger, ...) ali_log_log(logger, LOG_INFO, __VA_ARGS__)
#define ali_log_warn(logger, ...) ali_log_log(logger, LOG_WARN, __VA_ARGS__)
#define ali_log_error(logger, ...) ali_log_log(logger, LOG_ERROR, __VA_ARGS__)

#endif // ALI2_H

#ifdef ALI2_IMPLEMENTATION
#include <stdarg.h>

AliLogger ali_libc_logger = {
    .level = LOG_INFO,
    .f = NULL,
};

void ali_log_log(AliLogger* logger, AliLogLevel level, const char* fmt, ...) {
    if (logger == NULL) logger = &ali_libc_logger;
    if (logger->f == NULL) logger->f = stdout;
    if (level < logger->level) return;

    ali_static_assert(LOG_COUNT_ == 4);
    static const char* level_to_str[LOG_COUNT_] = {
        [LOG_INFO] = "[INFO]",
        [LOG_WARN] = "[WARN]",
        [LOG_ERROR] = "[ERROR]",
        [LOG_DEBUG] = "[DEBUG]",
    };

    printf("%s ", level_to_str[level]);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#endif // ALI_IMPLEMENTATION

#ifdef ALI2_REMOVE_PREFIX
#ifndef ALI2_REMOVE_PREFIX_GUARD
#define ALI2_REMOVE_PREFIX_GUARD

typedef ali_i8 i8;
typedef ali_i16 i16;
typedef ali_i32 i32;
typedef ali_i64 i64;

typedef ali_u8 u8;
typedef ali_u16 u16;
typedef ali_u32 u32;
typedef ali_u64 u64;

typedef ali_isize isize;
typedef ali_usize usize;

#define log_log ali_log_log
#define log_debug ali_log_debug
#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error

#endif // ALI2_REMOVE_PREFIX_GUARD
#endif // ALI2_REMOVE_PREFIX
