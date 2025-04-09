#ifndef ALI2_H
#define ALI2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    const char* file;
    int line;
}AliLocation;

#define ali_trap() __builtin_trap()
#define ali_here() ((AliLocation) { .file = __FILE__, .line = __LINE__ })

#define ali_assert(expr) ali_assert_with_loc(#expr, expr, ali_here())
void ali_assert_with_loc(const char* expr, bool ok, AliLocation loc);

#define ali_static_assert(expr) _Static_assert(expr, #expr)

#define ali_unreachable() do { fprintf(stderr, "%s:%d: UNREACHABLE\n", __FILE__, __LINE__); ali_trap(); } while (0)
#define ali_unused(thing) (void)(thing)

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

typedef enum {
    ALI_ALLOC,
    ALI_REALLOC,
    ALI_FREE,
    ALI_FREEALL,
}AliAllocatorAction;

typedef struct {
    void* (*allocator_function)(AliAllocatorAction action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, void* user);
    void* user;
}AliAllocator;

extern AliAllocator ali_libc_allocator;

#define ali_alloc_aligned(allocator, size, alignment) (allocator).allocator_function(ALI_ALLOC, NULL, 0, size, alignment, (allocator).user)
#define ali_alloc(allocator, size) (allocator).allocator_function(ALI_ALLOC, NULL, 0, size, 8)
#define ali_realloc_aligned(allocator, old_pointer, old_size, size, alignment) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, alignment, (allocator).user)
#define ali_realloc(allocator, old_pointer, old_size, size) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, 8, (allocator).user)
#define ali_free(allocator, old_pointer) (allocator).allocator_function(ALI_FREE, old_pointer, 0, 0, 0, (allocator).user)
#define ali_freeall(allocator) (allocator).allocator_function(ALI_FREEALL, NULL, 0, 0, 0, (allocator).user)

typedef struct {
    ali_usize size, capacity;
    ali_u8* data;
}AliArena;

AliArena ali_arena_create(ali_usize capacity);
AliAllocator ali_arena_allocator(AliArena* arena);
#define ali_arena_reset(arena) (arena)->size = 0

#define DA(Type) Type* items; ali_usize count, capacity
#define ali_da_append(da, item) do { \
        if ((da)->count >= (da)->capacity) { \
            if ((da)->capacity == 0) (da)->capacity = 8; \
            while ((da)->count >= (da)->capacity) (da)->capacity *= 3; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof((da)->items[0])); \
        } \
        (da)->items[(da)->count++] = (item); \
    } while (0)
#define ali_da_append_many(da, items, item_count) do { \
        if ((da)->count + (item_count) >= (da)->capacity) { \
            if ((da)->capacity == 0) (da)->capacity = 8; \
            while ((da)->count + (item_count) >= (da)->capacity) (da)->capacity *= 3; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof((da)->items[0])); \
        } \
        memcpy((da)->items + (da)->count, items, (item_count) * sizeof((da)->items[0])); \
        (da)->count += item_count; \
    } while (0)
#define ali_da_remove_unordered(da, i) (ali_assert(i < (da)->count), (da)->items[i] = (da)->items[--(da)->count])
#define ali_da_remove_ordered(da, i) do { \
        ali_assert((da)->count > 0); \
        ali_assert(i < (da)->count); \
        memmove((da)->items + (i), (da)->items + (i) + 1, ((da)->count - (i)) * sizeof((da)->items[0])); \
        (da)->count--; \
    } while (0)
#define ali_da_clear(da) (da)->count = 0
#define ali_da_free(da) (free((da)->items), (da)->items = NULL, (da)->capacity = 0, (da)->count = 0)
#define ali_da_foreach(da, Type, ptr) for (Type* ptr = (da)->items; ptr < (da)->items + (da)->count; ++ptr)

typedef struct {
    const char* start;
    size_t len;
}AliSv;
#define SV(static_cstr) ((AliSv) { .start = static_cstr, .len = sizeof(static_cstr) - 1 })

AliSv ali_sv_from_cstr(const char* cstr);
AliSv ali_sv_from_parts(const char* start, ali_usize len);

#endif // ALI2_H

#ifdef ALI2_IMPLEMENTATION
#include <stdarg.h>

void* ali__libc_allocator_function(AliAllocatorAction action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, void* user) {
    ali_unused(user);
    ali_unused(old_size);
    ali_unused(alignment);
    switch (action) {
        case ALI_ALLOC:
            return malloc(size);
        case ALI_REALLOC:
            return realloc(old_pointer, size);
        case ALI_FREE:
            free(old_pointer);
            return NULL;
        case ALI_FREEALL:
            return NULL;
    }
    
    ali_unreachable();
}

AliAllocator ali_libc_allocator = {
    .allocator_function = ali__libc_allocator_function,
    .user = NULL,
};

void* ali__arena_allocator(AliAllocatorAction action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, void* user) {
    AliArena* arena = user;
    switch (action) {
        case ALI_ALLOC: {
            ali_assert(arena->size + size < arena->capacity);
            arena->size += arena->size % alignment;
            ali_assert(arena->size + size < arena->capacity);
            void* ptr = arena->data + arena->size;
            arena->size += size;
            return ptr;
        } break;
        case ALI_REALLOC: {
            ali_assert(arena->size + size < arena->capacity);
            arena->size += arena->size % alignment;
            ali_assert(arena->size + size < arena->capacity);
            void* ptr = arena->data + arena->size;
            arena->size += size;
            memcpy(ptr, old_pointer, old_size);
            return ptr;
        } break;
        case ALI_FREE: {
            return NULL;
        } break;
        case ALI_FREEALL: {
            free(arena->data);
            arena->size = 0;
            arena->capacity = 0;
            return NULL;
        } break;
    }

    ali_unreachable();
}

AliArena ali_arena_create(ali_usize capacity) {
    return (AliArena) {
        .data = malloc(capacity),
        .size = 0,
        .capacity = capacity,
    };
}

AliAllocator ali_arena_allocator(AliArena* arena) {
    return (AliAllocator) {
        .allocator_function = ali__arena_allocator,
        .user = arena,
    };
}

void ali_assert_with_loc(const char* expr, bool ok, AliLocation loc) {
    if (!ok) {
        ali_log_error(&ali_libc_logger, "%s:%d: Assertion failed: %s", loc.file, loc.line, expr);
        ali_trap();
    }
}

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

AliSv ali_sv_from_cstr(const char* cstr) {
    AliSv sv = {0};
    sv.start = cstr;
    sv.len = strlen(cstr);
    return sv;
}

AliSv ali_sv_from_parts(const char* start, ali_usize len) {
    return (AliSv) {
        .start = start,
        .len = len,
    };
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

#define da_append ali_da_append
#define da_append_many ali_da_append_many
#define da_remove_unordered ali_da_remove_unordered
#define da_remove_ordered ali_da_remove_ordered
#define da_clear ali_da_clear
#define da_free ali_da_free
#define da_foreach ali_da_foreach

#define arena_create ali_arena_create
#define arena_allocator ali_arena_allocator
#define arena_reset ali_arena_reset
#endif // ALI2_REMOVE_PREFIX_GUARD
#endif // ALI2_REMOVE_PREFIX
