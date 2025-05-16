#ifndef ALI2_H
#define ALI2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// macros
typedef struct {
    const char* file;
    const char* function;
    int line;
}Ali_Location;

#define ali_trap() __builtin_trap()
#define ali_here() ((Ali_Location) { .file = __FILE__, .function = __func__, .line = __LINE__ })

#ifndef ALI_REMOVE_ASSERT
#define ali_assert(expr) ali_assert_with_loc(#expr, expr, ali_here())
#define ali_assertf(expr, ...) ali_assertf_with_loc(expr, ali_here(), __VA_ARGS__)
void ali_assert_with_loc(const char* expr, bool ok, Ali_Location loc);
void ali_assertf_with_loc(bool ok, Ali_Location loc, const char* fmt, ...);
#else // ALI_REMOVE_ASSERT
#define ali_assert(...)
#define ali_assertf(...)
#endif // ALI_REMOVE_ASSERT

#define ali_static_assert(expr) _Static_assert(expr, "Static assertion failed: " #expr)

#define ali_todo() do { fprintf(stderr, "%s:%d: TODO\n", __FILE__, __LINE__); ali_trap(); } while (0)
#define ali_unreachable() do { fprintf(stderr, "%s:%d: UNREACHABLE\n", __FILE__, __LINE__); ali_trap(); } while (0)
#define ali_unused(thing) (void)(thing)
#define ali_panic(msg) do { fprintf(stderr, "PANIC: %s", msg); ali_trap(); } while (0)

#define ali_array_len(arr) (sizeof(arr) / sizeof((arr)[0]))
#define ali_shift(arr, count) (ali_assert(count > 0), (count)--, *(arr)++)

#define ali_return_defer(value) do { result = (value); goto defer; } while (0)

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

// general
char* ali_libc_get_error(void);
char* ali_static_vsprintf(const char* fmt, va_list args);
__attribute__((__format__(printf, 1, 2)))
char* ali_static_sprintf(const char* fmt, ...);
bool ali_mem_eq(const void* a, const void* b, ali_usize size);

// logging
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_COUNT_,
}Ali_Log_Level;

extern const char* ali_loglevel_to_str[LOG_COUNT_];
extern const char* ali_loglevel_color[LOG_COUNT_];

typedef ali_u32 Ali_Log_Opts;
#define ALI_LOG_OPT_LEVEL ((Ali_Log_Opts)0x1)
#define ALI_LOG_OPT_DATE ((Ali_Log_Opts)0x2)
#define ALI_LOG_OPT_TIME ((Ali_Log_Opts)0x4)
#define ALI_LOG_OPT_LOC ((Ali_Log_Opts)0x8)
#define ALI_LOG_OPT_FUNCTION ((Ali_Log_Opts)0x10)
#define ALI_LOG_OPT_TERMCOLOR ((Ali_Log_Opts)0x20)
// TODO: 
// #define ALI_LOG_OPT_THREAD_ID 0x40

#define ALI_LOG_OPTS_DEFAULT (ALI_LOG_OPT_LEVEL | ALI_LOG_OPT_LOC | ALI_LOG_OPT_TERMCOLOR)

typedef void (*Ali_Logger_Function)(Ali_Log_Level level, const char* msg, void* user, Ali_Log_Opts opts, Ali_Location loc);

typedef struct {
    void* user;
    Ali_Logger_Function function;
    Ali_Log_Level level;
    Ali_Log_Opts opts;
}Ali_Logger;

Ali_Logger ali_console_logger(void);
Ali_Logger ali_file_logger(FILE* f);

// defaults to a console logger
extern Ali_Logger ali_global_logger;

__attribute__((__format__(printf, 4, 5)))
void ali_log_log_ex(Ali_Logger logger, Ali_Log_Level level, Ali_Location loc, const char* fmt, ...);
#define ali_log_debug_ex(logger, ...) ali_log_log_ex(logger, LOG_DEBUG, ali_here(), __VA_ARGS__)
#define ali_log_info_ex(logger, ...) ali_log_log_ex(logger, LOG_INFO, ali_here(), __VA_ARGS__)
#define ali_log_warn_ex(logger, ...) ali_log_log_ex(logger, LOG_WARN, ali_here(), __VA_ARGS__)
#define ali_log_error_ex(logger, ...) ali_log_log_ex(logger, LOG_ERROR, ali_here(), __VA_ARGS__)
#define ali_log_debug(...) ali_log_log_ex(ali_global_logger, LOG_DEBUG, ali_here(), __VA_ARGS__)
#define ali_log_info(...) ali_log_log_ex(ali_global_logger, LOG_INFO, ali_here(), __VA_ARGS__)
#define ali_log_warn(...) ali_log_log_ex(ali_global_logger, LOG_WARN, ali_here(), __VA_ARGS__)
#define ali_log_error(...) ali_log_log_ex(ali_global_logger, LOG_ERROR, ali_here(), __VA_ARGS__)

typedef union {
    bool option;
    char* string;
    ali_u64 u64;
    double f64;
}Ali_Flag_As;

typedef struct {
    const char* name;
    const char* description;
    ali_isize pos;
    Ali_Flag_As default_;
    char** aliases;
    ali_usize aliases_count;
}Ali_Flag_Options;

#define ali__flag_decl(func_name, Type) Type* func_name(Ali_Flag_Options options)
#define ali__flag_def(func_name, Type, type_enum_value) ali__flag_decl(func_name, Type) { \
        Ali_Flag flag = {0}; \
        flag.type = type_enum_value; \
        flag.options = options; \
        flag.as = options.default_; \
        Ali_Flag* new_flag = &flag_state.flags_array[flag_state.flags_count]; \
        ali__flags_push(flag); \
        return (void*)&new_flag->as; \
    }

ali__flag_decl(ali_flag_option, bool);
ali__flag_decl(ali_flag_u64, ali_u64);
ali__flag_decl(ali_flag_f64, double);
ali__flag_decl(ali_flag_string, char*);

void ali_flag_reset(void);
void ali_flag_print_usage(FILE* f);
bool ali_flag_parse(int argc, char** argv);

// allocator
typedef enum {
    ALI_ALLOC,
    ALI_REALLOC,
    ALI_FREE,
    ALI_FREEALL,
}Ali_Allocator_Action;

typedef struct {
    void* (*allocator_function)(Ali_Allocator_Action action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, Ali_Location loc, void* user);
    void* user;
}Ali_Allocator;

extern Ali_Allocator ali_libc_allocator;
extern Ali_Allocator ali_global_allocator;

#define ali_alloc_aligned_ex(allocator, size, alignment) (allocator).allocator_function(ALI_ALLOC, NULL, 0, size, alignment, ali_here(), (allocator).user)
#define ali_alloc_ex(allocator, size) (allocator).allocator_function(ALI_ALLOC, NULL, 0, size, 8, ali_here(), (allocator).user)
#define ali_realloc_aligned_ex(allocator, old_pointer, old_size, size, alignment) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, alignment, ali_here(), (allocator).user)
#define ali_realloc_ex(allocator, old_pointer, old_size, size) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, 8, ali_here(), (allocator).user)
#define ali_free_ex(allocator, old_pointer) (allocator).allocator_function(ALI_FREE, old_pointer, 0, 0, 0, ali_here(), (allocator).user)
#define ali_freeall_ex(allocator) (allocator).allocator_function(ALI_FREEALL, NULL, 0, 0, 0, ali_here(), (allocator).user)

#define ali_alloc_aligned(size, alignment) ali_alloc_aligned_ex(ali_global_allocator, size, alignment)
#define ali_alloc(size) ali_alloc_ex(ali_global_allocator, size)
#define ali_realloc_aligned(size, alignment) ali_realloc_aligned_ex(ali_global_allocator, size, alignment)
#define ali_realloc(size) ali_realloc_aligned_ex(ali_global_allocator, size)
#define ali_free(old_pointer) ali_free_ex(ali_global_allocator, old_pointer)
#define ali_freeall() ali_freeall_ex(ali_global_allocator)

// temporary buffer

#ifndef ALI_TEMPBUF_SIZE
#define ALI_TEMPBUF_SIZE (4 << 10)
#endif // ALI_TEMPBUF_SIZE

ali_usize ali_tstamp(void);
void ali_trewind(ali_usize stamp);
__attribute__((__format__(printf, 1, 2)))
char* ali_tsprintf(const char* fmt, ...);

// arena
typedef struct {
    ali_usize size, capacity;
    ali_u8* data;
}Ali_Arena;

Ali_Arena ali_arena_create(ali_usize capacity);
Ali_Allocator ali_arena_allocator(Ali_Arena* arena);
#define ali_arena_reset(arena) (arena)->size = 0

// dynamic arena
#ifndef ALI_ARENA_CHUNK_INIT_CAPACITY
#define ALI_ARENA_CHUNK_INIT_CAPACITY (4 << 10)
#endif // ALI_ARENA_CHUNK_INIT_CAPACITY

typedef struct Ali_Arena_Chunk {
    ali_usize size, capacity;
    struct Ali_Arena_Chunk* next;
    ali_u8 data[];
}Ali_Arena_Chunk;

typedef struct {
    Ali_Arena_Chunk* start, *end;
}Ali_Dynamic_Arena;

typedef struct {
    Ali_Arena_Chunk* target;
    ali_usize size;
}Ali_Arena_Mark;

Ali_Arena_Mark ali_dynamic_arena_mark(Ali_Dynamic_Arena* arena);
void ali_dynamic_arena_rollback(Ali_Dynamic_Arena* arena, Ali_Arena_Mark mark);
Ali_Allocator ali_dynamic_arena_allocator(Ali_Dynamic_Arena* arena);

// dynamic arrays (da)
#define DA(Type) Type* items; ali_usize count, capacity
#define ali_da_resize_for(da, item_count) do {\
        if ((da)->count + (item_count) >= (da)->capacity) { \
            if ((da)->capacity == 0) (da)->capacity = 8; \
            while ((da)->count + (item_count) >= (da)->capacity) (da)->capacity *= 3; \
            (da)->items = realloc((da)->items, (da)->capacity * sizeof((da)->items[0])); \
        } \
    } while (0)
#define ali_da_append(da, item) do { \
        ali_static_assert(sizeof((da)->items[0]) == sizeof(item)); \
        ali_da_resize_for(da, 1); \
        (da)->items[(da)->count++] = (item); \
    } while (0)
#define ali_da_shallow_append(da, item) do { \
        ali_static_assert(sizeof((da)->items[0]) == sizeof(item)); \
        ali_da_resize_for(da, 1); \
        (da)->items[(da)->count] = (item); \
    } while (0)
#define ali_da_append_many(da, items_, item_count) do { \
        ali_static_assert(sizeof((da)->items[0]) == sizeof((items_)[0])); \
        ali_da_resize_for(da, item_count); \
        memcpy((da)->items + (da)->count, items_, (item_count) * sizeof((da)->items[0])); \
        (da)->count += item_count; \
    } while (0)
#define ali_da_append_variadic(da, Type, ...) do { \
        ali_static_assert(sizeof((da)[0]) == sizeof(Type)); \
        Type items_[] = { __VA_ARGS__ }; \
        ali_usize item_count = ali_array_len(items_); \
        if (item_count > 0) { \
            ali_da_resize_for(da, item_count); \
            memcpy((da)->items + (da)->count, items_, (item_count) * sizeof((da)->items[0])); \
            (da)->count += item_count; \
        } \
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
#define ali_da_foreach(da, Type, ptr) for (Type* ptr = (da)->items; (uintptr_t)ptr < (uintptr_t)(da)->items + (da)->count; ++ptr)

typedef struct {
    DA(char*);
}Ali_Cstrs;

// tracking allocator
typedef struct {
    Ali_Location loc;
    ali_usize size;
    void* ptr;
}Ali_Tracked_Allocation;

typedef struct {
    Ali_Allocator allocator;
    DA(Ali_Tracked_Allocation);
}Ali_Tracking_Allocator;

Ali_Tracking_Allocator ali_track_allocator(Ali_Allocator allocator);
Ali_Allocator ali_tracking_allocator(Ali_Tracking_Allocator* allocator);
void ali_log_tracked(Ali_Tracking_Allocator allocator);

// string view (sv)
typedef struct {
    const char* start;
    ali_usize len;
}Ali_Sv;
#define SV(static_cstr) ((Ali_Sv) { .start = static_cstr, .len = sizeof(static_cstr) - 1 })
#define SV_FMT "%.*s"
#define SV_F(sv) (int)(sv).len, (sv).start

Ali_Sv ali_sv_from_cstr(const char* cstr);
Ali_Sv ali_sv_from_parts(const char* start, ali_usize len);

bool ali_sv_starts_with_prefix(Ali_Sv self, Ali_Sv prefix);
Ali_Sv ali_sv_strip_prefix(Ali_Sv self, Ali_Sv prefix);
bool ali_sv_ends_with_suffix(Ali_Sv self, Ali_Sv suffix);
Ali_Sv ali_sv_strip_suffix(Ali_Sv self, Ali_Sv suffix);
bool ali_sv_eq(Ali_Sv a, Ali_Sv b);

// slices
typedef struct {
    ali_usize data_size;
    ali_usize count;
    void* data;
}Ali_Slice;

#define ali_slice_is_of_type(slice, Type) ((slice).data_size == sizeof(Type))
#define ali_slice_from_parts(ptr, count_) ((Ali_Slice) { .data_size = sizeof((ptr)[0]), .count = count_, .data = ptr })

#define ali_da_slice(da) ((Ali_Slice) { .data_size = sizeof((da).items[0]), .count = (da).count, .data = (da).items })
#define ali_sv_slice(sv) ((Ali_Slice) { .data_size = 1, .count = (sv).count, .data = (sv).items })
Ali_Slice ali_slice_slice(Ali_Slice slice, ali_usize start, ali_usize end);
Ali_Slice ali_slice_to_byte_slice(Ali_Slice slice);
void* ali_slice_get(Ali_Slice slice, ali_usize index);
#define ali_slice_foreach(slice, Type, ptr) for (Type* ptr = (slice).data; (ali_u8*)ptr < (ali_u8*)((slice).data + (slice).count * (slice).data_size); ptr++)

// string builder (sb)
typedef struct {
    DA(char);
}Ali_Sb;

Ali_Sv ali_sb_to_sv(Ali_Sb* sb);
__attribute__((__format__(printf, 2, 3)))
void ali_sb_sprintf(Ali_Sb* sb, const char* fmt, ...);
char* ali_sb_to_cstr(Ali_Sb* sb, Ali_Allocator allocator);
void ali_sb_render_cmd(Ali_Sb* sb, char** cmd, ali_usize cmd_count);

// doing stuff with filesystem
#ifndef _WIN32
bool ali_pipe2(int p[2]);
#endif // _WIN32

bool ali_is_file1_modified_after_file2(const char* filepath1, const char* filepath2);
#define ali_need_rebuild(target, source) ali_is_file1_modified_after_file2(source, target)
bool ali_rename(const char* from, const char* to);
bool ali_remove(const char* filepath);
bool ali_mkdir_if_not_exists(const char* path);
bool ali_mkdir_deep_if_not_exists(const char* path);

// jobs
#ifdef _WIN32
typedef HANDLE AliJobHandle;
#else // _WIN32
typedef pid_t AliJobHandle;
#endif // _WIN32

typedef struct {
    AliJobHandle handle;
    int in[2];
    int out[2];
}Ali_Job;

typedef ali_u32 AliJobRedirect;
#define ALI_REDIRECT_STDOUT 0x1
#define ALI_REDIRECT_STDIN 0x2
#define ALI_REDIRECT_STDERR 0x4

Ali_Job ali_job_start(char** cmd, ali_usize cmd_count, AliJobRedirect redirect);
bool ali_job_wait(Ali_Job job);
bool ali_job_run(char **cmd, ali_usize cmd_count, AliJobRedirect redirect);

typedef struct {
    DA(Ali_Job);
}Ali_Jobs;

bool ali_jobs_wait(Ali_Jobs jobs);
bool ali_jobs_wait_and_reset(Ali_Jobs* jobs);

// cmd abstraction
typedef struct {
    DA(char*);
}Ali_Cmd;

#define ali_cmd_append ali_da_append
void ali_cmd_append_many_null(Ali_Cmd* cmd, char* arg1, ...);
#define ali_cmd_append_many(cmd, ...) ali_cmd_append_many_null(cmd, __VA_ARGS__, NULL)
Ali_Job ali_cmd_run_async(Ali_Cmd cmd, AliJobRedirect redirect);
bool ali_cmd_run_sync(Ali_Cmd cmd);
Ali_Job ali_cmd_run_async_and_reset(Ali_Cmd* cmd, AliJobRedirect redirect);
bool ali_cmd_run_sync_and_reset(Ali_Cmd* cmd);

#define ALI_REBUILD_YOURSELF(cmd, argc, argv) do { \
        ali_usize stamp = ali_tstamp(); \
        char* program = (argv)[0]; \
        char* old_program = ali_tsprintf("%s.old", program); \
        char* source = __FILE__; \
        if (ali_need_rebuild(program, source)) { \
            if (!ali_rename(program, old_program)) return 1; \
            ali_cmd_append_many(cmd, "gcc", "-ggdb", "-o", program, source); \
            if (!ali_cmd_run_sync_and_reset(cmd)) return 1; \
            if (!ali_remove(old_program)) return 1; \
            ali_cmd_append(cmd, program); \
            if (!ali_cmd_run_sync_and_reset(cmd)) return 1; \
            exit(0); \
        } \
        ali_trewind(stamp); \
    } while (0)

// build

typedef enum {
    ALI_STEP_FILE,
    ALI_STEP_EXE,
    ALI_STEP_STATIC,
    ALI_STEP_DYNAMIC,
}Ali_Step_Type;

typedef enum {
    ALI_DEBUG_NONE = 0,
    ALI_DEBUG_AUTO,
    ALI_DEBUG_GDB,
    ALI_DEBUG_COUNT_,
}Ali_Debug_Type;

typedef enum {
    ALI_OPTIMIZE_NONE = 0,
    ALI_OPTIMIZE_ONE,
    ALI_OPTIMIZE_TWO,
    ALI_OPTIMIZE_THREE,
    ALI_OPTIMIZE_FAST,
    ALI_OPTIMIZE_SIZE,
    ALI_OPTIMIZE_SIZE_AGRESSIVE,
    ALI_OPTIMIZE_COUNT_,
}Ali_Optimize_Type;

typedef struct Ali_Step Ali_Step;
typedef struct { DA(Ali_Step); }Ali_Steps;

struct Ali_Step {
    Ali_Step_Type type;
    Ali_Debug_Type debug;
    Ali_Optimize_Type optimize;
    char* name;

    Ali_Steps srcs; // this WILL be passed to the compiler/ar (ex. source files)
    Ali_Steps deps; // this WILL NOT be passed to the compiler (ex. header files)
    Ali_Cstrs linker_flags; // linker flags (passed as -Wl,%s)
};

typedef struct {
    DA(Ali_Step);
    Ali_Jobs jobs;
}Ali_Build;

void ali_step_free(Ali_Step* step);
Ali_Step ali_step_file(char* name);
Ali_Step ali_step_executable(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize);
Ali_Step ali_step_dynamic(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize);
Ali_Step ali_step_static(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize);

void ali_step_add_src(Ali_Step* step, Ali_Step substep);
void ali_step_add_dep(Ali_Step* step, Ali_Step substep);

bool ali_step_need_rebuild(Ali_Step* step);
bool ali_step_build(Ali_Step* step, Ali_Jobs* jobs, ali_usize cores);

#define ali_build_install ali_da_append

void ali_build_free(Ali_Build* b);
bool ali_build_build(Ali_Build* b, ali_usize cores);

// Gets to each step and does ali_da_remove(step->name)
// !!! Except the ones that have type ALI_STEP_FILE !!!
bool ali_build_clean(Ali_Build* b);

#endif // ALI2_H

#ifdef ALI2_IMPLEMENTATION
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#else // _WIN32
#include <windows.h>
#endif // _WIN32

#ifndef ALI_REMOVE_ASSERT
void ali_assert_with_loc(const char* expr, bool ok, Ali_Location loc) {
    if (!ok) {
        ali_log_error("%s:%d: Assertion failed: %s", loc.file, loc.line, expr);
        ali_trap();
    }
}

void ali_assertf_with_loc(bool ok, Ali_Location loc, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (!ok) {
        const char* msg = ali_static_vsprintf(fmt, args);
        ali_log_error("%s:%d: Assertion failed: %s", loc.file, loc.line, msg);
        ali_trap();
    }

    va_end(args);
}
#endif // ALI_REMOVE_ASSERT

char* ali_libc_get_error(void) {
    return strerror(errno);
}

char* ali_static_vsprintf(const char* fmt, va_list args) {
#ifndef ALI_STATIC_SPRINTF_BUFFER_SIZE
#define ALI_STATIC_SPRINTF_BUFFER_SIZE (4 << 10)
#endif // ALI_STATIC_SPRINTF_BUFFER_SIZE
    static char buffer[ALI_STATIC_SPRINTF_BUFFER_SIZE] = {0};
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    return buffer;
}

char* ali_static_sprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* str = ali_static_vsprintf(fmt, args);
    va_end(args);
    return str;
}

bool ali_mem_eq(const void* a, const void* b, ali_usize size) {
    const char* ab = a;
    const char* bb = b;
    for (ali_usize i = 0; i < size; ++i) {
        if (ab[i] != bb[i]) return false;
    }
    return true;
}

void* ali__libc_allocator_function(Ali_Allocator_Action action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, Ali_Location loc, void* user) {
    ali_unused(user);
    ali_unused(old_size);
    ali_unused(alignment);
    ali_unused(loc);

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

Ali_Allocator ali_libc_allocator = {
    .allocator_function = ali__libc_allocator_function,
    .user = NULL,
};

Ali_Allocator ali_global_allocator = {
    .allocator_function = ali__libc_allocator_function,
    .user = NULL,
};

static char buffer[ALI_TEMPBUF_SIZE];
static ali_usize buffer_size = 0;

ali_usize ali_tstamp(void) {
    return buffer_size;
}

void ali_trewind(ali_usize stamp) {
    buffer_size = stamp;
}

char* ali_tsprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char* ptr = buffer + buffer_size;

    va_start(args, fmt);
    int n_ = vsnprintf(buffer + buffer_size, n + 1, fmt, args);
    ali_unused(n_);
    va_end(args);

    ali_assert(buffer_size + n < ALI_TEMPBUF_SIZE);
    buffer_size += n;

    return ptr;
}

void* ali__arena_allocator(Ali_Allocator_Action action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, Ali_Location loc, void* user) {
    ali_unused(loc);
    Ali_Arena* arena = user;
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

Ali_Arena ali_arena_create(ali_usize capacity) {
    return (Ali_Arena) {
        .data = malloc(capacity),
        .size = 0,
        .capacity = capacity,
    };
}

Ali_Allocator ali_arena_allocator(Ali_Arena* arena) {
    return (Ali_Allocator) {
        .allocator_function = ali__arena_allocator,
        .user = arena,
    };
}

Ali_Arena_Mark ali_dynamic_arena_mark(Ali_Dynamic_Arena* arena) {
    return (Ali_Arena_Mark) {
        .target = arena->end,
        .size = arena->end->size,
    };
}

void ali_dynamic_arena_rollback(Ali_Dynamic_Arena* arena, Ali_Arena_Mark mark) {
    arena->end = mark.target;
    mark.target->size = mark.size;
    Ali_Arena_Chunk* current = mark.target->next;
    for (; current != NULL; current = current->next) {
        current->size = 0;
    }
}

void* ali__dynamic_arena_function(Ali_Allocator_Action action, void* old_pointer, ali_usize old_size, ali_usize size, ali_usize alignment, Ali_Location loc, void* user) {
    ali_unused(loc);
    Ali_Dynamic_Arena* arena = user;
    switch (action) {
            case ALI_ALLOC: {
                if (arena->end == NULL) {
                    ali_assert(arena->start == NULL);

                    ali_usize capacity = ALI_ARENA_CHUNK_INIT_CAPACITY;
                    while (capacity < size) { capacity *= 2; }

                    Ali_Arena_Chunk* new_chunk = malloc(sizeof(*new_chunk) + capacity);
                    ali_assert(new_chunk != NULL);
                    new_chunk->size = 0;
                    new_chunk->next = NULL;
                    new_chunk->capacity = capacity;
                    arena->start = new_chunk;
                    arena->end = new_chunk;
                }

                arena->end->size += (arena->end->size % alignment);
                if (arena->end->size + size > arena->end->capacity) {
                    ali_usize capacity = arena->end->capacity;
                    while (capacity < size) { capacity *= 2; }

                    Ali_Arena_Chunk* new_chunk = malloc(sizeof(*new_chunk) + capacity);
                    ali_assert(new_chunk != NULL);
                    new_chunk->size = 0;
                    new_chunk->next = NULL;
                    new_chunk->capacity = capacity;
                    arena->end->next = new_chunk;
                    arena->end = new_chunk;
                }

                void* ptr = arena->end->data + arena->end->size;
                arena->end->size += size;

                return ptr;
            } break;
            case ALI_REALLOC: {
                if (arena->end == NULL) {
                    ali_assert(arena->start == NULL);
                    Ali_Arena_Chunk* new_chunk = malloc(sizeof(*new_chunk));
                    ali_assert(new_chunk != NULL);
                    new_chunk->size = 0;
                    new_chunk->next = NULL;
                    new_chunk->capacity = arena->end->capacity;
                    while (new_chunk->capacity < size) { new_chunk->capacity *= 2; }
                    arena->end->next = new_chunk;
                    arena->start = new_chunk;
                    arena->end = new_chunk;
                }

                arena->end->size += (arena->end->size % alignment);
                if (arena->end->size + size > arena->end->capacity) {
                    Ali_Arena_Chunk* new_chunk = malloc(sizeof(*new_chunk));
                    new_chunk->size = 0;
                    new_chunk->next = NULL;
                    new_chunk->capacity = arena->end->capacity;
                    while (new_chunk->capacity < size) { new_chunk->capacity *= 2; }
                    arena->end->next = new_chunk;
                    arena->end = new_chunk;
                }

                void* ptr = arena->end->data + size;
                arena->end->size += size;
                memcpy(ptr, old_pointer, old_size);

                return ptr;
            } break;
            case ALI_FREE: {
                return NULL;
            } break;
            case ALI_FREEALL: {
                Ali_Arena_Chunk* current_chunk = arena->start;
                while (current_chunk != NULL) {
                    Ali_Arena_Chunk* next_chunk = current_chunk->next;
                    free(current_chunk);
                    current_chunk = next_chunk;
                }
                arena->start = NULL;
                arena->end = NULL;
                return NULL;
            } break;
    }
    ali_unreachable();
}

Ali_Allocator ali_dynamic_arena_allocator(Ali_Dynamic_Arena* arena) {
    return (Ali_Allocator) {
        .allocator_function = ali__dynamic_arena_function,
        .user = arena,
    };
}

ali_static_assert(LOG_COUNT_ == 4);
const char* ali_loglevel_to_str[LOG_COUNT_] = {
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO]= "INFO",
    [LOG_WARN]= "WARN",
    [LOG_ERROR] = "ERROR",
};

const char* ali_loglevel_color[LOG_COUNT_] = {
    [LOG_DEBUG] = "\x1B[33m",
    [LOG_INFO]= "\x1B[0m",
    [LOG_WARN]= "\x1B[93m",
    [LOG_ERROR] = "\x1B[91m",
};

void ali__console_function(Ali_Log_Level level, const char* msg, void* user, Ali_Log_Opts opts, Ali_Location loc) {
    ali_unused(user);
    ali_unused(loc);

    time_t now = time(NULL);
    bool terminal_color = opts & ALI_LOG_OPT_TERMCOLOR != 0;

    if (opts & ALI_LOG_OPT_LEVEL) {
        if (terminal_color) {
            fputs(ali_loglevel_color[level], stderr);
            fprintf(stderr, "[%s]", ali_loglevel_to_str[level]);
            fprintf(stderr, "\x1B[0m");
            fprintf(stderr, " ");
        } else {
            fprintf(stderr, "[%s] ", ali_loglevel_to_str[level]);
        }
    }

    if (opts & ALI_LOG_OPT_TIME) {
        char buffer[4096] = {0};
        const char* DATE_FORMAT = "%Y %m. %d.";
        strftime(buffer, sizeof(buffer), DATE_FORMAT, localtime(&now));
        fprintf(stderr, "[%s] ", buffer);
    }

    if (opts & ALI_LOG_OPT_DATE) {
        char buffer[4096] = {0};
        const char* TIME_FORMAT = "%H";
        strftime(buffer, sizeof(buffer), TIME_FORMAT, localtime(&now));
        fprintf(stderr, "[%s] ", buffer);
    }

    if (opts & ALI_LOG_OPT_LOC) {
        fprintf(stderr, "[%s:%d(%s)] ", loc.file, loc.line, loc.function);
    }

    fprintf(stderr, "%s\n", msg);
}

void ali__file_function(Ali_Log_Level level, const char* msg, void* user, Ali_Log_Opts opts, Ali_Location loc) {
    ali_unused(loc);

    time_t now = time(NULL);
    static char buffer[4069] = {0};
    FILE* f = user;

    if (opts & ALI_LOG_OPT_LEVEL) {
        fprintf(f, "[%s] ", ali_loglevel_to_str[level]);
    }

    if (opts & ALI_LOG_OPT_TIME) {
        char buffer[4096] = {0};
        const char* DATE_FORMAT = "%Y %m. %d.";
        strftime(buffer, sizeof(buffer), DATE_FORMAT, localtime(&now));
        fprintf(f, "[%s] ", buffer);
    }

    if (opts & ALI_LOG_OPT_DATE) {
        char buffer[4096] = {0};
        const char* TIME_FORMAT = "%H";
        strftime(buffer, sizeof(buffer), TIME_FORMAT, localtime(&now));
        fprintf(f, "[%s] ", buffer);
    }

    if (opts & ALI_LOG_OPT_LOC) {
        fprintf(f, "[%s:%d(%s)] ", loc.file, loc.line, loc.function);
    }

    fprintf(f, "%s\n", msg);
}

Ali_Logger ali_console_logger(void) {
    return (Ali_Logger) {
        .level = LOG_INFO,
        .function = ali__console_function,
        .user = NULL,
        .opts = ALI_LOG_OPTS_DEFAULT,
    };
}

Ali_Logger ali_file_logger(FILE* f) {
    return (Ali_Logger) {
        .level = LOG_INFO,
        .function = ali__file_function,
        .user = f,
        .opts = ALI_LOG_OPTS_DEFAULT,
    };
}

Ali_Logger ali_global_logger = {
    .level = LOG_INFO,
    .function = ali__console_function,
    .user = NULL,
    .opts = ALI_LOG_OPTS_DEFAULT,
};

void ali_log_log_ex(Ali_Logger logger, Ali_Log_Level level, Ali_Location loc, const char* fmt, ...) {
    if (level < logger.level) return;
    ali_assert(logger.function != NULL);

    va_list args;
    va_start(args, fmt);

    const char* msg = ali_static_vsprintf(fmt, args);
    logger.function(level, msg, logger.user, logger.opts, loc);

    va_end(args);
}

typedef enum {
    FLAG_BOOL,
    FLAG_STRING,
    FLAG_U64,
    FLAG_F64,
}Ali_Flag_Type;

typedef struct {
    Ali_Flag_Options options;
    Ali_Flag_Type type;
    Ali_Flag_As as;
}Ali_Flag;

typedef struct {
    const char* program;

    #ifndef ALI_FLAG_MAX_COUNT
    #define ALI_FLAG_MAX_COUNT (1 << 8)
    #endif // ALI_FLAG_MAX_COUNT

    Ali_Flag flags_array[ALI_FLAG_MAX_COUNT];
    ali_usize flags_count;
}Ali_Flag_State;

Ali_Flag_State flag_state = {0};

void ali__flags_push(Ali_Flag flag) {
    ali_assert(flag_state.flags_count < ALI_FLAG_MAX_COUNT);
    flag_state.flags_array[flag_state.flags_count++] = flag;
}

ali__flag_def(ali_flag_option, bool, FLAG_BOOL);
ali__flag_def(ali_flag_u64, ali_u64, FLAG_U64);
ali__flag_def(ali_flag_f64, double, FLAG_F64);
ali__flag_def(ali_flag_string, char*, FLAG_STRING);

void ali_flag_reset(void) {
    flag_state.flags_count = 0;
}

void ali_flag_print_usage(FILE* f) {
    fprintf(f, "Usage: %s ", flag_state.program);
    {
        ali_isize pos = 0;
        for (ali_usize i = 0; i < flag_state.flags_count; ++i) {
            Ali_Flag* flag = &flag_state.flags_array[i];

            if (flag->options.pos >= 0 && flag->options.pos == pos) {
                fprintf(f, "<%s> ", flag->options.name);
                pos++;
            }
        }
    }
    fprintf(f, "[OPTIONS]\n");
    fprintf(f, "Options:\n");
    for (ali_usize i = 0; i < flag_state.flags_count; ++i) {
        Ali_Flag* flag = &flag_state.flags_array[i];
        char* prefix = strlen(flag->options.name) > 1 ? "--" : "-";
        fprintf(f, "    %s%s", prefix, flag->options.name);
        if (flag->options.aliases_count) {
            fprintf(f, ", also: ");
            for (ali_usize i = 0; i < flag->options.aliases_count; ++i) {
                if (i != 0) fprintf(f, ", ");
                char* prefix = strlen(flag->options.aliases[i]) > 1 ? "--" : "-";
                fprintf(f, "`%s%s`", prefix, flag->options.aliases[i]);
            }
        }
        if (flag->options.description != NULL) {
            fprintf(f, " (%s)", flag->options.description);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

bool ali_flag_parse(int argc, char** argv) {
    flag_state.program = ali_shift(argv, argc);

    ali_isize pos = 0;
    while (argc > 0) {
        char* arg = ali_shift(argv, argc);
        
        if (*arg == '-' && *(arg + 1) == 0) goto pos_handling;
        bool is_short = *arg == '-' && *(arg + 2) == 0;
        bool is_long = *arg == '-' && *(arg + 1) == '-';
        if (is_short || is_long) {
            if (is_short) arg++;
            else if (is_long) arg += 2;

            bool found = false;
            for (ali_usize i = 0; i < flag_state.flags_count; ++i) {
                Ali_Flag* current_flag = &flag_state.flags_array[i];

                bool match = false;
                match |= strcmp(arg, current_flag->options.name) == 0;
                for (ali_usize i = 0; !match && i < current_flag->options.aliases_count; ++i) {
                    match |= strcmp(arg, current_flag->options.aliases[i]) == 0;
                }

                if (!match) continue;
                found = true;

                switch (current_flag->type) {
                    case FLAG_BOOL: {
                                        current_flag->as.option = true;
                                    } break;
                    case FLAG_STRING: {
                                          char* arg_value = ali_shift(argv, argc);
                                          current_flag->as.string = arg_value;
                                      } break;
                    case FLAG_U64: {
                                       char* arg_value = ali_shift(argv, argc);
                                       current_flag->as.u64 = atoll(arg_value);
                                   } break;
                    case FLAG_F64: {
                                       char* arg_value = ali_shift(argv, argc);
                                       current_flag->as.f64 = atof(arg_value);
                                   } break;
                }

                break;
            }

            if (!found) {
                ali_log_error("Couldn't parse args");
                ali_flag_print_usage(stderr);
                return false;
            }
        }

pos_handling:
        for (ali_usize i = 0; i < flag_state.flags_count; ++i) {
            Ali_Flag* current_flag = &flag_state.flags_array[i];

            if (current_flag->options.pos >= 0 && current_flag->options.pos == pos) {
                switch (current_flag->type) {
                    case FLAG_STRING: {
                        current_flag->as.string = arg;
                    } break;
                    case FLAG_U64: {
                        current_flag->as.u64 = atoll(arg);
                    } break;
                    case FLAG_F64: {
                        current_flag->as.f64 = atof(arg);
                    } break;
                    case FLAG_BOOL: {}break;
                }

                pos++;
            }
        }
    }

    return true;
}

Ali_Slice ali_slice_slice(Ali_Slice slice, ali_usize start, ali_usize end) {
    ali_assert(start < slice.count);
    ali_assert(end <= slice.count);
    ali_assert(start < end);

    return (Ali_Slice) {
        .data_size = slice.data_size,
        .count = end - start,
        .data = slice.data + (slice.data_size * start),
    };
}

Ali_Slice ali_slice_to_byte_slice(Ali_Slice slice) {
    return (Ali_Slice) {
        .count = slice.count * slice.data_size,
        .data = slice.data,
        .data_size = 1,
    };
}

void* ali_slice_get(Ali_Slice slice, ali_usize index) {
    ali_assert(index < slice.count);
    return slice.data + slice.data_size * index;
}

Ali_Sv ali_sb_to_sv(Ali_Sb* sb) {
    return ali_sv_from_parts(sb->items, sb->count);
}

void ali_sb_sprintf(Ali_Sb* sb, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    ali_da_resize_for(sb, n);

    va_start(args, fmt);
    int n_ = vsnprintf(sb->items + sb->count, n + 1, fmt, args);
    ali_unused(n_);
    va_end(args);

    sb->count += n;
}

char* ali_sb_to_cstr(Ali_Sb* sb, Ali_Allocator allocator) {
    char* copy = ali_alloc_ex(allocator, sb->count + 1);
    memcpy(copy, sb->items, sb->count);
    copy[sb->count] = 0;
    return copy;
}

Ali_Sv ali_sv_from_cstr(const char* cstr) {
    Ali_Sv sv = {0};
    sv.start = cstr;
    sv.len = strlen(cstr);
    return sv;
}

Ali_Sv ali_sv_from_parts(const char* start, ali_usize len) {
    return (Ali_Sv) {
        .start = start,
        .len = len,
    };
}

bool ali_sv_starts_with_prefix(Ali_Sv self, Ali_Sv prefix) {
    if (self.len < prefix.len) return false;
    for (ali_usize i = 0; i < prefix.len; ++i) {
        if (self.start[i] != prefix.start[i]) return false;
    }
    return true;
}

Ali_Sv ali_sv_strip_prefix(Ali_Sv self, Ali_Sv prefix) {
    if (!ali_sv_starts_with_prefix(self, prefix)) return self;
    self.len -= prefix.len; 
    self.start += prefix.len; 
    return self;
}

bool ali_sv_ends_with_suffix(Ali_Sv self, Ali_Sv suffix) {
    if (self.len < suffix.len) return false;
    for (ali_usize i = 0; i < suffix.len; ++i) {
        if (self.start[self.len - 1 - i] != suffix.start[suffix.len - 1 - i]) return false;
    }
    return true;
}

Ali_Sv ali_sv_strip_suffix(Ali_Sv self, Ali_Sv suffix) {
    if (!ali_sv_ends_with_suffix(self, suffix)) return self;
    self.len -= suffix.len;
    return self;
}

bool ali_sv_eq(Ali_Sv a, Ali_Sv b) {
    if (a.len != b.len) return false;
    return ali_mem_eq(a.start, b.start, a.len);
}

void ali_sb_render_cmd(Ali_Sb* sb, char** cmd, ali_usize cmd_count) {
    for (ali_usize i = 0; i < cmd_count; ++i) {
        if (i != 0) ali_da_append(sb, (char)' ');
        if (strchr(cmd[i], ' ') == NULL) {
            int len = strlen(cmd[i]);
            ali_da_append_many(sb, cmd[i], len);
        } else {
            ali_sb_sprintf(sb, "'%s'", cmd[i]);
        }
    }
}

#ifndef _WIN32
bool ali_pipe2(int p[2]) {
    if (pipe(p) < 0) {
        ali_log_error("Couldn't create pipe: %s", ali_libc_get_error());
        return false;
    }
    return true;
}
#endif // _WIN32

bool ali_is_file1_modified_after_file2(const char* filepath1, const char* filepath2) {
    struct stat st;

    if (stat(filepath1, &st) < 0) {
        ali_log_error("Couldn't stat %s: %s", filepath1, ali_libc_get_error());
        return false;
    }

    time_t file1_time = st.st_mtim.tv_sec;
    if (stat(filepath2, &st) < 0) {
        return true;
    }

    return file1_time > st.st_mtim.tv_sec;
}

bool ali_rename(const char* from, const char* to) {
    if (rename(from, to) < 0) {
        ali_log_error("Couldn't rename %s to %s: %s", from, to, ali_libc_get_error());
        return false;
    }
    return true;
}

bool ali_remove(const char* filepath) {
    if (remove(filepath) < 0) {
        ali_log_error("Couldn't remove %s: %s", filepath, ali_libc_get_error());
        return false;
    }
    return true;
}

bool ali_mkdir_if_not_exists(const char* path) {
    if (mkdir(path, 0775) < 0) {
        switch (errno) {
            case EEXIST:
                ali_log_info("Directory %s already exists", path);
                break; // ignore
            default:
                ali_log_error("Couldn't create directory: %s", ali_libc_get_error());
                return false;
        }
    }
    return true;
}

bool ali_mkdir_deep_if_not_exists(const char* path) {
    static char buffer[1024] = {0};

    const char* slash = strchr(path, '/');
    while (slash != NULL) {
        memcpy(buffer, path, slash - path);
        buffer[slash - path] = 0;
        if (strcmp(buffer, ".") != 0) {
            if (!ali_mkdir_if_not_exists(buffer)) return false;
        }
        slash = strchr(slash + 1, '/');
    }
    if (!ali_mkdir_if_not_exists(path)) return false;

    return true;
}

Ali_Job ali_job_start_posix(char** cmd, ali_usize cmd_count, AliJobRedirect redirect) {
    Ali_Job job = {0};
    job.handle = -1;

    bool should_redirect_stdin = (redirect & ALI_REDIRECT_STDIN) != 0;
    bool should_redirect_stdout = (redirect & ALI_REDIRECT_STDOUT) != 0;
    bool should_redirect_stderr = (redirect & ALI_REDIRECT_STDERR) != 0;

    if (should_redirect_stdout || should_redirect_stderr) {
        if (ali_pipe2(job.out)) return job;
    }

    if (should_redirect_stdin) {
        if (ali_pipe2(job.in)) return job;
    }

    char** cmd_copy = ali_alloc_ex(ali_libc_allocator, sizeof(cmd[0]) * (cmd_count + 1));
    memcpy(cmd_copy, cmd, sizeof(cmd[0]) * cmd_count);
    cmd_copy[cmd_count] = NULL;

    job.handle = fork();
    if (job.handle < 0) {
        ali_log_error("Couldn't start process: %d", job.handle);
        return job;
    } else if (job.handle == 0) {
        if (should_redirect_stdin) {
            dup2(STDIN_FILENO, job.in[0]);
            close(job.in[1]);
        }

        if (should_redirect_stdout) {
            dup2(STDOUT_FILENO, job.out[1]);
            close(job.out[0]);
        }

        if (should_redirect_stderr) {
            dup2(STDERR_FILENO, job.out[1]);
            close(job.out[0]);
        }

        execvp(cmd_copy[0], cmd_copy);
        ali_log_error("Couldn't start program '%s': %s", cmd[0], ali_libc_get_error());
        exit(1);
    }

    ali_free_ex(ali_libc_allocator, cmd_copy);
    return job;
}

bool ali_job_wait_posix(Ali_Job job) {
    for (;;) {
        int wstatus;
        if (waitpid(job.handle, &wstatus, 0) < 0) {
            ali_log_error("Couldn't wait for process: %s", ali_libc_get_error());
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int estatus = WEXITSTATUS(wstatus);
            if (estatus != 0) {
                ali_log_error("Process exited with status %d", estatus);
                return false;
            }

            break; 
        }

        if (WIFSIGNALED(wstatus)) {
            ali_log_error("Process exited with signal %d", WTERMSIG(wstatus));
            return false;
        }
    }

    return true;
}

Ali_Job ali_job_start(char** cmd, ali_usize cmd_count, AliJobRedirect redirect) {
#ifdef _WIN32
#error "TODO: windows"
#else // _WIN32
    return ali_job_start_posix(cmd, cmd_count, redirect);
#endif // _WIN32
}

bool ali_job_wait(Ali_Job job) {
#ifdef _WIN32
#error "TODO: windows"
#else // _WIN32
    return ali_job_wait_posix(job);
#endif // _WIN32
}

bool ali_job_run(char **cmd, ali_usize cmd_count, AliJobRedirect redirect) {
    Ali_Job job = ali_job_start(cmd, cmd_count, redirect);
    return ali_job_wait(job);
}

bool ali_jobs_wait(Ali_Jobs jobs) {
    bool result = true;
    ali_da_foreach(&jobs, Ali_Job, job) {
        if (!ali_job_wait(*job)) result = false;
    }
    return result;
}

bool ali_jobs_wait_and_reset(Ali_Jobs* jobs) {
    bool result = true;
    ali_da_foreach(jobs, Ali_Job, job) {
        if (!ali_job_wait(*job)) result = false;
    }
    jobs->count = 0;
    return result;
}

void ali_cmd_append_many_null(Ali_Cmd* cmd, char* arg1, ...) {
    ali_cmd_append(cmd, arg1);

    va_list args;
    va_start(args, arg1);
    char* arg = va_arg(args, char*);
    while (arg != NULL) {
        ali_cmd_append(cmd, arg);
        arg = va_arg(args, char*);
    }
    va_end(args);
}

Ali_Job ali_cmd_run_async(Ali_Cmd cmd, AliJobRedirect redirect) {
    Ali_Sb sb = {0};
    ali_sb_render_cmd(&sb, cmd.items, cmd.count);

    Ali_Sv rendered = ali_sb_to_sv(&sb);
    ali_log_info("[CMD] "SV_FMT, SV_F(rendered));

    ali_da_free(&sb);
    Ali_Job job = ali_job_start(cmd.items, cmd.count, redirect);
    return job;
}

bool ali_cmd_run_sync(Ali_Cmd cmd) {
    Ali_Job job = ali_cmd_run_async(cmd, 0);
    return ali_job_wait(job);
}

Ali_Job ali_cmd_run_async_and_reset(Ali_Cmd* cmd, AliJobRedirect redirect) {
    Ali_Sb sb = {0};
    ali_sb_render_cmd(&sb, cmd->items, cmd->count);

    Ali_Sv rendered = ali_sb_to_sv(&sb);
    ali_log_info("[CMD] "SV_FMT, SV_F(rendered));

    ali_da_free(&sb);
    Ali_Job job = ali_job_start(cmd->items, cmd->count, redirect);
    cmd->count = 0;
    return job;
}

bool ali_cmd_run_sync_and_reset(Ali_Cmd* cmd) {
    Ali_Job job = ali_cmd_run_async_and_reset(cmd, 0);
    return ali_job_wait(job);
}

const char* ali__debug_to_str[ALI_DEBUG_COUNT_] = {
    [ALI_DEBUG_NONE] = "-g0",
    [ALI_DEBUG_AUTO] = "-g",
    [ALI_DEBUG_GDB] = "-ggdb",
};

const char* ali__optimize_to_str[ALI_OPTIMIZE_COUNT_] = {
    [ALI_OPTIMIZE_NONE] = "-O0",
    [ALI_OPTIMIZE_ONE] = "-O1",
    [ALI_OPTIMIZE_TWO] = "-O2",
    [ALI_OPTIMIZE_THREE] = "-O3",
    [ALI_OPTIMIZE_FAST] = "-Ofast",
    [ALI_OPTIMIZE_SIZE] = "-Os",
    [ALI_OPTIMIZE_SIZE_AGRESSIVE] = "-Oz",
};

void ali_step_free(Ali_Step* step) {
    ali_da_foreach(&step->srcs, Ali_Step, substep) {
        ali_step_free(substep);
    }
    ali_da_free(&step->srcs);
    ali_da_foreach(&step->deps, Ali_Step, substep) {
        ali_step_free(substep);
    }
    ali_da_free(&step->deps);
    ali_da_free(&step->linker_flags);
}

bool ali_step_need_rebuild(Ali_Step* step) {
    ali_da_foreach(&step->srcs, Ali_Step, substep) {
        if (ali_step_need_rebuild(substep)) return true;
        if (ali_need_rebuild(step->name, substep->name)) return true;
    }
    ali_da_foreach(&step->deps, Ali_Step, substep) {
        if (ali_step_need_rebuild(substep)) return true;
        if (ali_need_rebuild(step->name, substep->name)) return true;
    }
    return false;
}

Ali_Step ali_step_file(char* name) {
    return (Ali_Step) {
        .name = name,
        .type = ALI_STEP_FILE,
    };
}

Ali_Step ali_step_executable(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize) {
    return (Ali_Step) {
        .name = name,
        .type = ALI_STEP_EXE,
        .debug = debug,
        .optimize = optimize,
    };
}

Ali_Step ali_step_dynamic(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize) {
    return (Ali_Step) {
        .name = name,
        .type = ALI_STEP_DYNAMIC,
        .debug = debug,
        .optimize = optimize,
    };
}

Ali_Step ali_step_static(char* name, Ali_Debug_Type debug, Ali_Optimize_Type optimize) {
    return (Ali_Step) {
        .name = name,
        .type = ALI_STEP_STATIC,
        .debug = debug,
        .optimize = optimize,
    };
}

void ali_step_add_src(Ali_Step* step, Ali_Step substep) {
    ali_da_append(&step->srcs, substep);
}

void ali_step_add_dep(Ali_Step* step, Ali_Step substep) {
    ali_da_append(&step->deps, substep);
}

bool ali_step_build(Ali_Step* step, Ali_Jobs* jobs, ali_usize cores) {
    if (jobs->count >= cores) {
        if (!ali_jobs_wait_and_reset(jobs)) return false;
    }

    bool result = true;

    ali_usize stamp = ali_tstamp();
    Ali_Cmd cmd = {0};
    bool need_rebuild = ali_step_need_rebuild(step);
    if (!need_rebuild) ali_return_defer(true);

    switch (step->type) {
        case ALI_STEP_FILE: {}break; // no build step required, just a file
        case ALI_STEP_EXE: {
            ali_cmd_append_many(&cmd, "gcc", ali__debug_to_str[step->debug], ali__optimize_to_str[step->optimize]);
            ali_cmd_append_many(&cmd, "-o", step->name);
            ali_da_foreach(&step->srcs, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
                ali_da_append(&cmd, substep->name);
            }
            ali_da_foreach(&step->deps, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
            }
            ali_da_foreach(&step->linker_flags, char*, lflag) {
                char* flag = ali_tsprintf("-Wl,%s", *lflag);
                ali_cmd_append_many(&cmd, flag);
            }
            Ali_Job step_job = ali_cmd_run_async(cmd, 0);
            ali_da_append(jobs, step_job);
        }break;
        case ALI_STEP_STATIC: {
            ali_cmd_append_many(&cmd, "ar", "rcs", step->name);
            ali_da_foreach(&step->srcs, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
                ali_da_append(&cmd, substep->name);
            }
            ali_da_foreach(&step->deps, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
            }
            // ar does not do linking
            Ali_Job step_job = ali_cmd_run_async(cmd, 0);
            ali_da_append(jobs, step_job);
        }break;
        case ALI_STEP_DYNAMIC: {
            ali_cmd_append_many(&cmd, "gcc", ali__debug_to_str[step->debug], ali__optimize_to_str[step->optimize]);
            ali_cmd_append_many(&cmd, "-shared", "-fPIC");
            ali_cmd_append_many(&cmd, "-o", step->name);
            ali_da_foreach(&step->srcs, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
                ali_da_append(&cmd, substep->name);
            }
            ali_da_foreach(&step->deps, Ali_Step, substep) {
                if (!ali_step_build(substep, jobs, cores)) ali_return_defer(false);
            }
            ali_da_foreach(&step->linker_flags, char*, lflag) {
                char* flag = ali_tsprintf("-Wl,%s", *lflag);
                ali_cmd_append_many(&cmd, flag);
            }
            Ali_Job step_job = ali_cmd_run_async(cmd, 0);
            ali_da_append(jobs, step_job);
        }break;
        default:
            ali_unreachable();
    }

defer:
    if (result && step->type != ALI_STEP_FILE) {
        if (need_rebuild) {
            ali_log_info("[BUILD] Build %s", step->name);
        } else {
            ali_log_info("[BUILD] No need to build %s", step->name);
        }
    }

    ali_trewind(stamp);
    ali_da_free(&cmd);
    return result;
}

// Gets to each step and does ali_da_remove(step->name)
// !!! Except the ones that have type ALI_STEP_FILE !!!
bool ali_step_clean(Ali_Step* step) {
    if (step->type != ALI_STEP_FILE) {
        if (!ali_remove(step->name)) return false;
    }
    ali_da_foreach(&step->srcs, Ali_Step, substep) {
        if (!ali_step_clean(substep)) return false;
    }
    ali_da_foreach(&step->deps, Ali_Step, substep) {
        if (!ali_step_clean(substep)) return false;
    }
    return true;
}

void ali_build_free(Ali_Build* b) {
    ali_da_foreach(b, Ali_Step, step) {
        ali_step_free(step);
    }
    ali_jobs_wait(b->jobs);
    ali_da_free(&b->jobs);
    ali_da_free(b);
}

bool ali_build_build(Ali_Build* b, ali_usize cores) {
    ali_da_foreach(b, Ali_Step, step) {
        if (!ali_step_build(step, &b->jobs, cores)) return false;
    }
    return true;
}

bool ali_build_clean(Ali_Build* b) {
    ali_da_foreach(b, Ali_Step, step) {
        if (!ali_step_clean(step)) return false;
    }
    return true;
}

#endif // ALI_IMPLEMENTATION

#ifndef ALI2_KEEP_PREFIX
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

typedef Ali_Sb Sb;
typedef Ali_Sv Sv;
typedef Ali_Allocator Allocator;
typedef Ali_Allocator_Action Allocator_Action;
typedef Ali_Location Location;
typedef Ali_Arena Arena;
typedef Ali_Dynamic_Arena Dynamic_Arena;
typedef Ali_Slice Slice;
typedef Ali_Job Job;
typedef Ali_Logger Logger;

#define trap ali_trap
#define assert ali_assert
#define assertf ali_assertf
#define static_assert ali_static_assert
#define todo ali_todo
#define unreachable ali_unreachable
#define unused ali_unused
#define array_len ali_array_len
#define shift ali_shift
#define return_defer ali_return_defer

#define libc_get_error ali_libc_get_error

#define console_logger ali_console_logger
#define file_logger ali_file_logger

#define log_log_ex ali_log_log_ex
#define log_debug ali_log_debug
#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error
#define log_debug_ex ali_log_debug_ex
#define log_info_ex ali_log_info_ex
#define log_warn_ex ali_log_warn_ex
#define log_error_ex ali_log_error_ex

#define flag_option ali_flag_option
#define flag_string ali_flag_string
#define flag_u64 ali_flag_u64
#define flag_f64 ali_flag_f64
#define flag_parse ali_flag_parse
#define flag_print_usage ali_flag_print_usage

#define da_resize_for ali_da_resize_for
#define da_append ali_da_append
#define da_shallow_append ali_da_shallow_append
#define da_append_many ali_da_append_many
#define da_append_variadic ali_da_append_variadic
#define da_remove_unordered ali_da_remove_unordered
#define da_remove_ordered ali_da_remove_ordered
#define da_clear ali_da_clear
#define da_free ali_da_free
#define da_foreach ali_da_foreach

#define arena_create ali_arena_create
#define arena_allocator ali_arena_allocator
#define arena_reset ali_arena_reset

#define sb_to_sv ali_sb_to_sv
#define sb_to_cstr ali_sb_to_cstr
#define sb_sprintf ali_sb_sprintf

#define sv_from_cstr ali_sv_from_cstr
#define sv_from_parts ali_sv_from_parts
#define sv_strip_prefix ali_sv_strip_prefix
#define sv_strip_suffix ali_sv_strip_suffix
#define sv_starts_with_prefix ali_sv_starts_with_prefix
#define sv_ends_with_suffix ali_sv_ends_with_suffix

#define slice_is_of_type ali_slice_is_of_type
#define slice_from_parts ali_slice_from_parts
#define da_slice ali_da_slice
#define sv_slice ali_sv_slice
#define slice_slice ali_slice_slice
#define slice_get ali_slice_get
#define slice_foreach ali_slice_foreach

#define job_start ali_job_start
#define job_wait ali_job_wait
#define job_run ali_job_run
#define jobs_wait ali_jobs_wait
#define jobs_wait_and_reset ali_jobs_wait_and_reset

#define is_file1_modified_after_file2 ali_is_file1_modified_after_file2
#define need_rebuild ali_need_rebuild
#define mkdir_if_not_exists ali_mkdir_if_not_exists
#define mkdir_deep_if_not_exists ali_mkdir_deep_if_not_exists

#define cmd_append ali_cmd_append
#define cmd_append_many ali_cmd_append_many
#define cmd_run_async ali_cmd_run_async
#define cmd_run_sync ali_cmd_run_sync
#define cmd_run_async_and_reset ali_cmd_run_async_and_reset
#define cmd_run_sync_and_reset ali_cmd_run_sync_and_reset

#endif // ALI2_REMOVE_PREFIX_GUARD
#endif // ALI2_KEEP_PREFIX
