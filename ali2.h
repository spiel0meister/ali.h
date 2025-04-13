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
    int line;
}AliLocation;

#define ali_trap() __builtin_trap()
#define ali_here() ((AliLocation) { .file = __FILE__, .line = __LINE__ })

#define ali_assert(expr) ali_assert_with_loc(#expr, expr, ali_here())
void ali_assert_with_loc(const char* expr, bool ok, AliLocation loc);

#define ali_static_assert(expr) _Static_assert(expr, #expr)

#define ali_unreachable() do { fprintf(stderr, "%s:%d: UNREACHABLE\n", __FILE__, __LINE__); ali_trap(); } while (0)
#define ali_unused(thing) (void)(thing)

#define ali_array_len(arr) (sizeof(arr) / sizeof((arr)[0]))

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

// logging
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

__attribute__((__format__(printf, 3, 4)))
void ali_log_log(AliLogger* logger, AliLogLevel level, const char* fmt, ...);
#define ali_log_debug(logger, ...) ali_log_log(logger, LOG_DEBUG, __VA_ARGS__)
#define ali_log_info(logger, ...) ali_log_log(logger, LOG_INFO, __VA_ARGS__)
#define ali_log_warn(logger, ...) ali_log_log(logger, LOG_WARN, __VA_ARGS__)
#define ali_log_error(logger, ...) ali_log_log(logger, LOG_ERROR, __VA_ARGS__)

// allocator
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
#define ali_alloc(allocator, size) (allocator).allocator_function(ALI_ALLOC, NULL, 0, size, 8, (allocator).user)
#define ali_realloc_aligned(allocator, old_pointer, old_size, size, alignment) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, alignment, (allocator).user)
#define ali_realloc(allocator, old_pointer, old_size, size) (allocator).allocator_function(ALI_REALLOC, old_pointer, old_size, size, 8, (allocator).user)
#define ali_free(allocator, old_pointer) (allocator).allocator_function(ALI_FREE, old_pointer, 0, 0, 0, (allocator).user)
#define ali_freeall(allocator) (allocator).allocator_function(ALI_FREEALL, NULL, 0, 0, 0, (allocator).user)

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
}AliArena;

AliArena ali_arena_create(ali_usize capacity);
AliAllocator ali_arena_allocator(AliArena* arena);
#define ali_arena_reset(arena) (arena)->size = 0

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
        ali_da_resize_for(da, 1); \
        (da)->items[(da)->count++] = (item); \
    } while (0)
#define ali_da_shallow_append(da, item) do { \
        ali_da_resize_for(da, 1); \
        (da)->items[(da)->count] = (item); \
    } while (0)
#define ali_da_append_many(da, items_, item_count) do { \
        ali_da_resize_for(da, item_count); \
        memcpy((da)->items + (da)->count, items_, (item_count) * sizeof((da)->items[0])); \
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

// string view (sv)
typedef struct {
    const char* start;
    size_t len;
}AliSv;
#define SV(static_cstr) ((AliSv) { .start = static_cstr, .len = sizeof(static_cstr) - 1 })
#define SV_FMT "%.*s"
#define SV_F(sv) (int)(sv).count, (sv).items

AliSv ali_sv_from_cstr(const char* cstr);
AliSv ali_sv_from_parts(const char* start, ali_usize len);

bool ali_sv_starts_with_prefix(AliSv self, AliSv prefix);
AliSv ali_sv_strip_prefix(AliSv self, AliSv prefix);
bool ali_sv_ends_with_suffix(AliSv self, AliSv suffix);
AliSv ali_sv_strip_suffix(AliSv self, AliSv suffix);

// string builder (sb)
typedef struct {
    DA(char);
}AliSb;

AliSv ali_sb_to_sv(AliSb* sb);
__attribute__((__format__(printf, 2, 3)))
void ali_sb_sprintf(AliSb* sb, const char* fmt, ...);
char* ali_sb_to_cstr(AliSb* sb, AliAllocator allocator);
void ali_sb_render_cmd(AliSb* sb, char** cmd, ali_usize cmd_count);

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
}AliJob;

typedef ali_u32 AliJobRedirect;
#define ALI_REDIRECT_STDOUT 0x1
#define ALI_REDIRECT_STDIN 0x2
#define ALI_REDIRECT_STDERR 0x4

AliJob ali_job_start(char** cmd, ali_usize cmd_count, AliJobRedirect redirect);
bool ali_job_wait(AliJob job);
bool ali_job_run(char **cmd, ali_usize cmd_count, AliJobRedirect redirect);

typedef struct {
    DA(char*);
}AliCmd;

#define ali_cmd_append ali_da_append
void ali_cmd_append_many_null(AliCmd* cmd, char* arg1, ...);
#define ali_cmd_append_many(cmd, ...) ali_cmd_append_many_null(cmd, __VA_ARGS__, NULL)
AliJob ali_cmd_run_async(AliCmd cmd, AliJobRedirect redirect);
bool ali_cmd_run_sync(AliCmd cmd);
AliJob ali_cmd_run_async_and_reset(AliCmd* cmd, AliJobRedirect redirect);
bool ali_cmd_run_sync_and_reset(AliCmd* cmd);

#define ALI_REBUILD_YOURSELF(cmd, argc, argv) do { \
        ali_usize tstamp = ali_tstamp(); \
        const char* program = (argv)[0]; \
        const char* old_program = ali_tsprintf("%s.old", program); \
        const char* source = __FILE__; \
        if (ali_need_rebuild(program, source)) { \
            if (!ali_rename(program, old_program)) return 1; \
            ali_cmd_append_many(cmd, "gcc", "-o", program, source); \
            if (!ali_cmd_run_sync_and_reset(cmd)) return 1; \
            if (!ali_remove(old_program)) return 1; \
        } \
        ali_trewind(tstamp); \
    } while (0)

#endif // ALI2_H

#ifdef ALI2_IMPLEMENTATION
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#else // _WIN32
#include <windows.h>
#endif // _WIN32

char* ali_libc_get_error(void) {
    return strerror(errno);
}

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

    va_start(args, fmt);
    int n_ = vsnprintf(buffer + buffer_size, n + 1, fmt, args);
    ali_unused(n_);
    va_end(args);

    ali_assert(buffer_size + n < ALI_TEMPBUF_SIZE);
    buffer_size += n;
}

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

AliSv ali_sb_to_sv(AliSb* sb) {
    return ali_sv_from_parts(sb->items, sb->count);
}

void ali_sb_sprintf(AliSb* sb, const char* fmt, ...) {
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

char* ali_sb_to_cstr(AliSb* sb, AliAllocator allocator) {
    char* copy = ali_alloc(allocator, sb->count + 1);
    memcpy(copy, sb->items, sb->count);
    copy[sb->count] = 0;
    return copy;
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

bool ali_sv_starts_with_prefix(AliSv self, AliSv prefix) {
    if (self.len < prefix.len) return false;
    for (ali_usize i = 0; i < prefix.len; ++i) {
        if (self.start[i] != prefix.start[i]) return false;
    }
    return true;
}

AliSv ali_sv_strip_prefix(AliSv self, AliSv prefix) {
    if (!ali_sv_starts_with_prefix(self, prefix)) return self;
    self.len -= prefix.len; 
    self.start += prefix.len; 
    return self;
}

bool ali_sv_ends_with_suffix(AliSv self, AliSv suffix) {
    if (self.len < suffix.len) return false;
    for (ali_usize i = 0; i < suffix.len; ++i) {
        if (self.start[self.len - 1 - i] != suffix.start[suffix.len - 1 - i]) return false;
    }
    return true;
}

AliSv ali_sv_strip_suffix(AliSv self, AliSv suffix) {
    if (!ali_sv_ends_with_suffix(self, suffix)) return self;
    self.len -= suffix.len;
    return self;
}


void ali_sb_render_cmd(AliSb* sb, char** cmd, ali_usize cmd_count) {
    for (ali_usize i = 0; i < cmd_count; ++i) {
        if (i != 0) ali_da_append(sb, ' ');
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
        ali_log_error(&ali_libc_logger, "Couldn't create pipe: %s\n", ali_libc_get_error());
        return false;
    }
    return true;
}
#endif // _WIN32

bool ali_is_file1_modified_after_file2(const char* filepath1, const char* filepath2) {
    struct stat st;

    if (stat(filepath1, &st) < 0) {
        ali_log_error(&ali_libc_logger, "Couldn't stat %s: %s\n", filepath1, ali_libc_get_error());
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
        ali_log_error(&ali_libc_logger, "Couldn't rename %s to %s: %s\n", from, to, ali_libc_get_error());
        return false;
    }
    return true;
}

bool ali_remove(const char* filepath) {
    if (remove(filepath) < 0) {
        ali_log_error(&ali_libc_logger, "Couldn't remove %s: %s\n", filepath, ali_libc_get_error());
        return false;
    }
    return true;
}

bool ali_mkdir_if_not_exists(const char* path) {
    if (mkdir(path, 0775) < 0) {
        switch (errno) {
            case EEXIST:
                ali_log_info(&ali_libc_logger, "Directory %s already exists\n", path);
                break; // ignore
            default:
                ali_log_error(&ali_libc_logger, "Couldn't create directory: %s\n", ali_libc_get_error());
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

AliJob ali_job_start_posix(char** cmd, ali_usize cmd_count, AliJobRedirect redirect) {
    AliJob job = {0};
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

    char** cmd_copy = ali_alloc(ali_libc_allocator, sizeof(cmd[0]) * (cmd_count + 1));
    memcpy(cmd_copy, cmd, sizeof(cmd[0]) * cmd_count);
    cmd_copy[cmd_count] = NULL;

    job.handle = fork();
    if (job.handle < 0) {
        ali_log_error(&ali_libc_logger, "Couldn't start process: %d\n", job.handle);
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
        ali_log_error(&ali_libc_logger, "Couldn't start program '%s': %s\n", cmd[0], ali_libc_get_error());
        exit(1);
    }

    ali_free(ali_libc_allocator, cmd_copy);
    return job;
}

bool ali_job_wait_posix(AliJob job) {
    for (;;) {
        int wstatus;
        if (waitpid(job.handle, &wstatus, 0) < 0) {
            ali_log_error(&ali_libc_logger, "Couldn't wait for process: %s\n", ali_libc_get_error());
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int estatus = WEXITSTATUS(wstatus);
            if (estatus != 0) {
                ali_log_error(&ali_libc_logger, "Process exited with status %d\n", estatus);
                return false;
            }

            break; 
        }

        if (WIFSIGNALED(wstatus)) {
            ali_log_error(&ali_libc_logger, "Process exited with signal %d\n", WTERMSIG(wstatus));
            return false;
        }
    }

    return true;
}

AliJob ali_job_start(char** cmd, ali_usize cmd_count, AliJobRedirect redirect) {
#ifdef _WIN32
#error "TODO: windows"
#else // _WIN32
    return ali_job_start_posix(cmd, cmd_count, redirect);
#endif // _WIN32
}

bool ali_job_wait(AliJob job) {
#ifdef _WIN32
#error "TODO: windows"
#else // _WIN32
    return ali_job_wait_posix(job);
#endif // _WIN32
}

bool ali_job_run(char **cmd, ali_usize cmd_count, AliJobRedirect redirect) {
    AliJob job = ali_job_start(cmd, cmd_count, redirect);
    return ali_job_wait(job);
}

void ali_cmd_append_many_null(AliCmd* cmd, char* arg1, ...) {
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

AliJob ali_cmd_run_async(AliCmd cmd, AliJobRedirect redirect) {
    AliJob job = ali_job_start(cmd.items, cmd.count, redirect);
    return job;
}

bool ali_cmd_run_sync(AliCmd cmd) {
    return ali_job_run(cmd.items, cmd.count, 0);
}

AliJob ali_cmd_run_async_and_reset(AliCmd* cmd, AliJobRedirect redirect) {
    AliJob job = ali_job_start(cmd->items, cmd->count, redirect);
    cmd->count = 0;
    return job;
}

bool ali_cmd_run_sync_and_reset(AliCmd* cmd) {
    AliJob job = ali_cmd_run_async_and_reset(cmd, 0);
    return ali_job_wait(job);
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

#define libc_get_error ali_libc_get_error

#define log_log ali_log_log
#define log_debug ali_log_debug
#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error

#define da_resize_for ali_da_resize_for
#define da_append ali_da_append
#define da_shallow_append ali_da_shallow_append
#define da_append_many ali_da_append_many
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

#define job_start ali_job_start
#define job_wait ali_job_wait
#define job_run ali_job_run

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
#endif // ALI2_REMOVE_PREFIX
