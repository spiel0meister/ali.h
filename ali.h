/**
	ali.h - A single header file consisting of things the C std lib is missing.
	For now, it contains:
        - some util stuff (ali_util)
        - some simple types and aliases (ali_types)
        - replacements for libc functions (ali_libc_replace)
        - logging (ali_log)
        - cli flags parsing (ali_flag)
        - interface for allocators (ali_allocator)
        - arena allocator (ali_arena)
        - bump allocator (ali_arena)
        - testing (ali_testing)
        - dynamic array (ali_da)
        - slices (ali_slice)
        - temp allocator (ali_temp_alloc)
        - utf8 (ali_utf8)
        - string view (ali_sv)
        - string builder (ali_sb)
        - zlib (ali_zlib)
        - measure code (ali_measure)
        - cmd (ali_cmd)

	This is a stb-style header file, which means you use this file like it's a normal
	header file, ex.:
        #include "ali.h"
	To include implementations, do this in ONE of your translation units:
        #define ALI_IMPLEMENTATION
        #include "ali.h"

	ali.h also supports removing the ali_* prefix of functions and macros by defining ALI_REMOVE_PREFIX.

	License:
	MIT License
	Copyright 2024 spiel_meister <soviczan7@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify,
	merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ALI_H_
#define ALI_H_
#include <stdio.h>
#include <stdlib.h>

#define ALI_VERSION "0.1.0"

#ifdef _WIN32
#define ALI_WINDOWS
#endif // _WIN32

#ifdef ALI_WINDOWS
#error "TODO: Windows"
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#endif // ALI_WINDOWS

// Customizable functions
#ifndef ALI_ABORT
#include <stdlib.h>
#define ALI_ABORT abort
#endif // ALI_ABORT

// @module ali_types
#ifndef ALI_TYPES_
#define ALI_TYPES_
#include <stdint.h>
typedef uint8_t  ali_u8;
typedef uint16_t ali_u16;
typedef uint32_t ali_u32;
typedef uint64_t ali_u64;

typedef int8_t  ali_i8;
typedef int16_t ali_i16;
typedef int32_t ali_i32;
typedef int64_t ali_i64;

typedef float ali_f32;

#ifdef __x86_64__
typedef double ali_f64;
#endif // __x86_64__

typedef ali_u64 ali_usize;
typedef ali_i64 ali_isize;
#endif // ALI_TYPES_

// @module ali_types end

// @module ali_util
typedef enum {
    ali_false = 0,
    ali_true = 1,
}ali_bool;
#define false ali_false
#define true ali_true

typedef struct {
    const char* file;
    ali_usize line;
}AliLocation;

#define ALI_HERE ((AliLocation) { .line = __LINE__, .file = __FILE__ })

#define ALI_TRAP() __builtin_trap()
#define ALI_BREAKPOINT() ALI_TRAP()
#define ALI_BREAKPOINT_IF(expr) if (expr) { ALI_TRAP() }

#define ALI_STRINGIFY(x) #x
#define ALI_STRINGIFY_2(x) ALI_STRINGIFY(x)

#define ALI_ARRAY_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))
#define ALI_INLINE_ARRAY(Type, ...) ( (Type[]) { __VA_ARGS__ } )
#define ALI_INLINE_ARRAY_WITH_SIZE(Type, ...) (Type[]){ __VA_ARGS__ }, (sizeof((Type[]){ __VA_ARGS__ })/sizeof(Type))
#define ALI_INLINE_EMPTY_ARRAY_WITH_SIZE(Type) NULL, 0

#define ALI_UNUSED(thing) (void)(thing)
#define ALI_UNREACHABLE() do { fprintf(stderr, "%s:%d: UNREACABLE\n", __FILE__, __LINE__); ALI_ABORT(); } while (0)
#define ALI_TODO() do { fprintf(stderr, "%s:%d: TODO: %s not implemented\n", __FILE__, __LINE__, __func__); ALI_ABORT(); } while (0)
#define ALI_PANIC(...) do { fprintf(stderr, __VA_ARGS__); ALI_ABORT(); } while (0)

#define ALI_SWAP(Type, a, b) do { \
        Type __tmp = *(a); \
        *(a) = *(b); \
        *(b) = (__tmp); \
	} while (0)

#define ALI_RETURN_DEFER(value) do { result = value; goto defer; } while (0)

#define ALI_FORMAT_ATTRIBUTE(str, vstart) __attribute__((__format__(printf, str, vstart)))

#define ali_add_u64_checked(a, b, ptr) (*(ptr) = (a) + (b), b <= UINT64_MAX - a)
#define ali_sub_u64_checked(a, b, ptr) (*(ptr) = (a) - (b), b <= a)

#define ali_shift(ptr, size) (ali_assert((size) > 0), (size)--, *(ptr)++)

// 'path/to/file.c' -> 'file.c', '/path/to/dir' -> 'dir'
const char* ali_path_name(const char* path);
char* ali_shift_args(int* argc, char*** argv);

// @module ali_util end

// @module ali_libc_replace

void* ali_memcpy(void* to, const void* from, ali_usize size);
char* ali_strchr(char* cstr, char c);

void ali__assert(ali_bool ok, const char* expr, AliLocation loc);
#define ali_assert(expr) ali__assert(expr, #expr, ALI_HERE)

// @module ali_libc_replace end

// @module ali_log
#ifndef ALI_NO_LOG
typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,

	LOG_COUNT_
}AliLogLevel;

extern FILE* ali_global_logfile;
extern AliLogLevel ali_global_loglevel;

void ali_init_global_log(void);

void ali_log_logn_va(AliLogLevel level, const char* fmt, va_list args);
void ali_log_log_va(AliLogLevel level, const char* fmt, va_list args);

ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_log_logn(AliLogLevel level, const char* fmt, ...);
ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_log_log(AliLogLevel level, const char* fmt, ...);

#define ali_logn_info(...) ali_log_logn(LOG_INFO, __VA_ARGS__)
#define ali_logn_warn(...) ali_log_logn(LOG_WARN, __VA_ARGS__)
#define ali_logn_error(...) ali_log_logn(LOG_ERROR, __VA_ARGS__)

#else // ALI_LOG_END
#define ali_log_log(_level, ...) do { printf(__VA_ARGS__); } while (0)
#define ali_log_logn_va(_level, fmt, args) do { vprintf(fmt, args); printf("\n"); } while (0)
#define ali_logn_info(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define ali_logn_warn(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define ali_logn_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#endif // ALI_NO_LOG
// @module ali_log end

// @module ali_flag

typedef enum {
    FLAG_STRING = 0,
    FLAG_U64,
    FLAG_F64,
    FLAG_OPTION,
}FlagType;
typedef union {
    const char* string;
    ali_u64 num_u64;
    ali_f64 num_f64;
    ali_bool option;
}FlagAs;

typedef struct {
    const char* name;
    const char** aliases;
    ali_usize aliases_count;
    const char* description;

    FlagType type;
    FlagAs as;
}AliFlag;

const char** ali_flag_string_ex(const char* name, const char* desc, const char* default_, const char** aliases, ali_usize aliases_count);
#define ali_flag_string(name, desc, default_) ali_flag_string_ex(name, desc, default_, NULL, 0)

ali_u64* ali_flag_u64_ex(const char* name, const char* desc, ali_u64 default_, const char** aliases, ali_usize aliases_count);
#define ali_flag_u64(name, desc, default_) ali_flag_u64_ex(name, desc, default_, NULL, 0)

ali_f64* ali_flag_f64_ex(const char* name, const char* desc, ali_f64 default_, const char** aliases, ali_usize aliases_count);
#define ali_flag_f64(name, desc, default_) ali_flag_f64_ex(name, desc, default_, NULL, 0)

ali_bool* ali_flag_option_ex(const char* name, const char* desc, ali_bool default_, const char** aliases, ali_usize aliases_count);
#define ali_flag_option(name, desc, default_) ali_flag_option_ex(name, desc, default_, NULL, 0)

void ali_flag_reset(void);

void ali_flag_print_help(FILE* sink, const char* program);
ali_bool ali_flag_parse(int* argc, char*** argv, const char* program);

// @module ali_flag end

// @module ali_allocator

typedef void* (*ali_alloc_t)(void* data, ali_usize size, ali_usize alignment);
typedef void* (*ali_realloc_t)(void* data, void* ptr, ali_usize old_size, ali_usize new_size, ali_usize alignment);
typedef void (*ali_free_t)(void* data, void* ptr);

typedef struct {
    ali_alloc_t alloc;
    ali_realloc_t realloc;
    ali_free_t free;
    void* data;
}AliAllocator;

void* ali_alloc_ex(AliAllocator allocator, ali_usize size, ali_usize alignment);
#define ali_alloc(allocator, size) ali_alloc_ex(allocator, size, 8)
void* ali_realloc_ex(AliAllocator allocator, void* ptr, ali_usize old_size, ali_usize new_size, ali_usize alignemnt);
#define ali_realloc(allocator, ptr, old_size, new_size) ali_realloc_ex(allocator, ptr, old_size, new_size, 8)
void ali_free(AliAllocator allocator, void* ptr);

void* ali_memdup(AliAllocator allocator, void* from, ali_usize len);
void* ali_sprintf(AliAllocator allocator, const char* fmt, ...);

extern AliAllocator ali_libc_allocator;

// @module ali_allocator end

// @module ali_bump

typedef struct {
    void* buffer;
    size_t size;
    size_t capacity;
}AliBump;

AliBump ali_bump_from_buffer(void* buffer, ali_usize buffer_size);
AliAllocator ali_bump_allocator(AliBump* bump);

// @module ali_bump end

// @module ali_arena 

#ifndef ALI_REGION_DEFAULT_CAP
#define ALI_REGION_DEFAULT_CAP (4 << 10)
#endif // ALI_REGION_DEFAULT_CAP

typedef struct AliRegion {
	ali_usize count;
	ali_usize capacity;
	struct AliRegion* next;

	ali_u8 data[];
}AliRegion;

typedef struct {
    ali_usize region_capacity;
	AliRegion *start, *end;
}AliArena;

typedef struct {
	AliRegion* r;
	ali_usize count;
}AliArenaMark;

AliAllocator ali_arena_allocator(AliArena* arena);

AliArenaMark ali_arena_mark(AliArena* self);
void ali_arena_rollback(AliArena* self, AliArenaMark mark);
void ali_arena_reset(AliArena* self);
void ali_arena_free(AliArena* self);

// @module ali_arena end

// @module ali_testing

typedef struct {
    // Use this if you need random numbers
    ali_u64 seed;

    // Use this arena, if you need dynamic memory
    AliAllocator allocator;

    ali_usize error_count;
}AliTesting;

typedef void (*ali_test_t)(AliTesting* t);
void ali_testing_run(AliTesting* t, ali_test_t test);
void ali_testing_print(AliTesting* t);
void ali_testing__fail(AliTesting* t, const char* file, int line, const char* func);
ali_bool ali_testing__expect(AliTesting* t, ali_bool ok, const char* file, int line, const char* func, const char* fmt, ...);

#define ali_testing_fail(t) ali_testing__fail(t, __FILE__, __LINE__, __func__)
#define ali_testing_expect(t, ok, ...) ali_testing__expect(t, ok, __FILE__, __LINE__, __func__, __VA_ARGS__)

// @module ali_testing end

// @module ali_da

#ifndef ALI_DA_DEFAULT_INIT_CAPACITY
#define ALI_DA_DEFAULT_INIT_CAPACITY 8
#endif // ALI_DA_DEFAULT_INIT_CAPACITY

typedef struct {
    AliAllocator allocator;

	ali_usize count;
	ali_usize capacity;
	ali_u8 data[];
}AliDaHeader;

void* ali_da_create(AliAllocator allocator, ali_usize init_capacity, ali_usize item_size);

AliDaHeader* ali_da_new_header_with_size(AliAllocator allocator, ali_usize init_capacity, ali_usize item_size);
AliDaHeader* ali_da_get_header(void* da);
void* ali_da_maybe_resize_with_size(void* da, ali_usize to_add, ali_usize item_size);
void* ali_da_free(void* da);

#define ali_da_maybe_resize(da, to_add) ((da) = ali_da_maybe_resize_with_size(da, to_add, sizeof(*(da))))

#define ali_da_getlen(da) ((da) == NULL ? 0 : ali_da_get_header(da)->count)
#define ali_da_reset(da) if ((da) != NULL) { ali_da_get_header(da)->count = 0; }

#define ali_da_append(da, item) (ali_da_maybe_resize(da, 1), (da)[ali_da_get_header(da)->count++] = item)
#define ali_da_shallow_append(da, item) (ali_da_maybe_resize(da, 1), (da)[ali_da_get_header(da)->count] = item)
#define ali_da_append_many(da, items, item_count) do { \
    ali_da_maybe_resize(da, 1); \
    memcpy(da + da_getlen(da), items, (item_count) * sizeof(*(da))); \
    ali_da_get_header(da)->count += item_count; \
} while (0)

#define ali_da_remove_unordered(da, i) (ali_assert(i >= 0), (da)[i] = (da)[--ali_da_get_header(da)->count])
#define ali_da_remove_ordered(da, i) (ali_assert(i >= 0), memmove(da + i, da + i + 1, (ali_da_get_header(da)->count - i - 1) * sizeof(*(da))), ali_da_get_header(da)->count--)

#define ali_da_for(da, iter_name) for (ali_usize iter_name = 0; iter_name < ali_da_getlen(da); ++iter_name)
#define ali_da_foreach(da, Type, iter_name) for (Type* iter_name = da; iter_name < da + ali_da_getlen(da); ++iter_name)

// @module ali_da end

// @module ali_sv
typedef struct {
	char* start;
	ali_usize len;
}AliSv;

#define ALI_SV_FMT "%.*s"
#define ALI_SV_F(sv) (int)(sv).len, (sv).start

AliSv ali_sv_from_parts(char* start, ali_usize len);
AliSv ali_sv_from_cstr(char* cstr);

void ali_sv_step(AliSv* self);
AliSv ali_sv_trim_left(AliSv self);
AliSv ali_sv_trim_right(AliSv self);
AliSv ali_sv_trim(AliSv self);

AliSv ali_sv_chop_left(AliSv* self, ali_usize n);
AliSv ali_sv_chop_right(AliSv* self, ali_usize n);
AliSv ali_sv_chop_by_c(AliSv* self, char c);

ali_bool ali_sv_chop_u64_bin(AliSv* sv, ali_u64* out);
ali_bool ali_sv_chop_u64_oct(AliSv* sv, ali_u64* out);
ali_bool ali_sv_chop_u64_dec(AliSv* sv, ali_u64* out);
ali_bool ali_sv_chop_u64_hex(AliSv* sv, ali_u64* out);
ali_bool ali_sv_chop_u64(AliSv* sv, ali_u64* out);

ali_bool ali_sv_chop_f64(AliSv* sv, ali_f64* out);

ali_bool ali_sv_chop_prefix(AliSv* self, AliSv prefix);
ali_bool ali_sv_chop_suffix(AliSv* self, AliSv suffix);

ali_bool ali_sv_eq(AliSv left, AliSv right);
ali_bool ali_sv_starts_with(AliSv self, AliSv prefix);
ali_bool ali_sv_ends_with(AliSv self, AliSv suffix);

char* ali_temp_sv_to_cstr(AliSv sv);

// @module ali_sv end

// @module ali_slice

typedef struct {
    ali_usize count;

    ali_usize item_size;
    ali_u8* data;
}AliSlice;

AliSlice ali_slice_from_parts_with_size(void* data, ali_usize count, ali_usize item_size);
#define ali_slice_from_parts(data, count) ali_slice_from_parts_with_size(data, count, sizeof((data)[0]))
AliSlice ali_slice_from_da_with_size(void* da, ali_usize item_size);
#define ali_slice_from_da(da) ali_slice_from_da_with_size(da, sizeof(*(da)))
AliSlice ali_da_slice_with_size(void* da, ali_usize start, ali_usize end_exclusive, ali_usize item_size);
#define ali_da_slice(da, start, end_exclusive) ali_da_slice_with_size(da, start, end_exclusive, sizeof(*(da)))
AliSlice ali_slice_cstr(char* str, ali_usize start, ali_usize end_exclusive);
AliSlice ali_slice_sv(AliSv sv, ali_usize start, ali_usize end_exclusive);
AliSlice ali_slice_slice(AliSlice slice, ali_usize start, ali_usize end_exclusive);
void* ali_slice_get(AliSlice slice, ali_usize i);

#define ali_slice_for(slice, i) for (ali_usize i = 0; i < slice.count; ++i)
#define ali_slice_foreach(slice, Type, ptr) for (Type* ptr = slice.data; ptr < slice.data + (slice.count * slice.item_size); ++ptr)

AliSv ali_sv_from_slice(AliSlice slice);

// @module ali_slice end

// @module ali_temp_alloc
#ifndef ALI_TEMP_BUF_SIZE
#define ALI_TEMP_BUF_SIZE (8 << 20)
#endif // ALI_TEMP_BUF_SIZE

void* ali_temp_alloc(ali_usize size);
char* ali_temp_strdup(const char* str);
char* ali_temp_strndup(const char* start, ali_usize count);
void* ali_temp_memdup(void* data, ali_usize data_size);
ALI_FORMAT_ATTRIBUTE(1, 2) char* ali_temp_sprintf(const char* fmt, ...);
void* ali_temp_get_cur();
void ali_temp_push(char c);
void ali_temp_push_str(const char* str);

ali_usize ali_temp_stamp(void);
void ali_temp_rewind(ali_usize stamp);
void ali_temp_reset(void);

// @module ali_temp_alloc end

// @module ali_utf8
typedef ali_u8 ali_utf8;
typedef ali_u32 ali_rune;

#define ALI_UTF8(cstr) (ali_utf8*)(cstr)

ali_usize ali_utf8len(const ali_utf8* utf8);
ali_rune ali_utf8c_to_rune(const ali_utf8* utf8c, ali_usize* rune_size);
ali_rune* ali_utf8_to_runes(AliAllocator allocator, const ali_utf8* utf8, ali_usize* count);

ali_bool ali_is_rune_valid(ali_rune rune);
ali_usize ali_rune_size(ali_rune rune);
const ali_utf8* ali_rune_to_utf8(ali_rune rune);
ali_utf8* ali_runes_to_utf8(AliAllocator allocator, ali_rune* runes, ali_usize len);

ali_rune* ali_temp_utf8_to_runes(const ali_utf8* utf8, ali_usize* count);
ali_utf8* ali_temp_runes_to_utf8(ali_rune* runes, ali_usize len);

// @module ali_utf8 end

// @module ali_sb
typedef struct {
	char* data;
	ali_usize count;
	ali_usize capacity;
}AliSb;

void ali_sb_maybe_resize(AliSb* self, ali_usize to_add);
void ali_sb_push_strs_null(AliSb* self, ...);
void ali_sb_push_nstr(AliSb* self, char* start, ali_usize n);
ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...);
void ali_sb_free(AliSb* self);

ali_bool ali_sb_read_file(AliSb* self, const char* path);
ali_bool ali_sb_read_file_by_chunks(AliSb* self, const char* path);

ali_bool ali_sb_write_file(AliSb* self, const char* path);

#define ali_sb_push_strs(...) ali_sb_push_strs_null(__VA_ARGS__, NULL)

#define ali_sb_to_sv(sb) ali_sv_from_parts((sb).items, (sb).count)

// @module ali_sb end

// @module ali_zlib
#ifdef ALI_ZLIB

ali_bool ali_zlib_compress(AliSb* out, const ali_u8* to_compress, ali_usize to_compress_size);
ali_bool ali_zlib_decompress(AliSb* out, const ali_u8* to_decompress, ali_usize to_decompress_size);

#endif // ALI_ZLIB
// @module ali_zlib end

// @module ali_measure
double ali_get_now();

#ifndef ALI_MEASUREMENTS_COUNT
#define ALI_MEASUREMENTS_COUNT 1024
#endif // ALI_MEASUREMENTS_COUNT

void ali_measure_start(const char* name);
void ali_measure_end(const char* name);

void ali_print_measurements(void);
// @module ali_measure end

// @module ali_cmd

typedef struct {
    ali_u8 redirect_bitmask;
    int in_pipe[2];
    int out_pipe[2];
}AliCmdRedirect;
#define ALI_REDIRECT_STDIN (1 << 0)
#define ALI_REDIRECT_STDOUT (1 << 1)
#define ALI_REDIRECT_STDERR (1 << 2)

#define ali_cmd_append_arg ali_da_append
#define ali_cmd_shallow_append_arg ali_da_shallow_append

void ali_cmd_append_args_(char*** cmd, ...);
#define ali_cmd_append_args(...) ali_cmd_append_args_(__VA_ARGS__, NULL)

char* ali_cmd_render(char** cmd);
pid_t ali_cmd_run_async_redirect(char** cmd, AliCmdRedirect* redirect);
#define ali_cmd_run_async(cmd) ali_cmd_run_async_redirect(cmd, NULL)
ali_bool ali_wait_for_process(pid_t pid);
ali_bool ali_cmd_run_sync(char** cmd);
ali_bool ali_cmd_run_sync_and_reset(char** cmd);

ali_bool ali_needs_rebuild(const char* output, const char** inputs, ali_usize input_count);
ali_bool ali_needs_rebuild1(const char* output, const char* input);

ali_bool ali_dup2_logged(int fd1, int fd2);
ali_bool ali_rename(char*** cmd, const char* from, const char* to);
ali_bool ali_remove(char*** cmd, const char* path);

ali_bool ali_create_dir_if_not_exists(const char* path);
ali_bool ali_create_dir_all_if_not_exists(char* path);

#define ali_rebuild_yourself(cmd, argc, argv) do { \
    const char* src = __FILE__; \
    const char* dst = argv[0]; \
    if (ali_needs_rebuild1(dst, src)) { \
        const char* old_dst = ali_temp_sprintf("%s.prev", dst); \
        if (!ali_rename(cmd, dst, old_dst)) { \
            exit(1); \
        } \
        ali_cmd_append_args(cmd, "gcc", "-ggdb", "-o", dst, src); \
        if (!ali_cmd_run_sync_and_reset(*(cmd))) { \
            ali_rename(cmd, old_dst, dst); \
            exit(1); \
        } \
        ali_cmd_append_args(cmd, dst); \
        for(ali_isize i = 1; i < argc; ++i) { ali_cmd_append_arg(*(cmd), argv[i]); } \
        if (!ali_cmd_run_sync_and_reset(*(cmd))) exit(1); \
        exit(0); \
    } \
} while (0)

// @module ali_cmd end

#endif // ALI_H_

#ifdef ALI_IMPLEMENTATION
#undef ALI_IMPLEMENTATION
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

// @module ali_util

const char* ali_path_name(const char* path) {
	const char* slash = strrchr(path, '/');
	return slash != NULL ? slash + 1 : path;
}

char* ali_shift_args(int* argc, char*** argv) {
    if (*argc < 1) return NULL;
    char* arg = **argv;
    (*argv)++;
    (*argc)--;
    return arg;
}

// @module ali_util end

// @module ali_libc_replacement

char* ali_strchr(char* cstr, char c) {
    while (*cstr != c && *cstr != 0) cstr++;
    return cstr;
}

void* ali_memcpy(void* to, const void* from, ali_usize size) {
    for (ali_usize i = 0; i < size; ++i) {
        ((ali_u8*)to)[i] = ((ali_u8*)from)[i];
    }
    return to;
}

void ali__assert(ali_bool ok, const char* expr, AliLocation loc) {
    if (!ok) {
        ali_logn_error("[ASSERT] %s:%lu: Assertion failed: %s", loc.file, loc.line, expr);
        ALI_TRAP();
    }
}

// @module ali_libc_replacement end

// @module ali_log
#ifndef ALI_NO_LOG

_Static_assert(LOG_COUNT_ == 3, "Log level was added");
const char* loglevel_to_str[LOG_COUNT_] = {
	[LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",
	[LOG_ERROR] = "ERROR",
};

FILE* ali_global_logfile = NULL;
AliLogLevel ali_global_loglevel = LOG_INFO;

void ali_log_logn_va(AliLogLevel level, const char* fmt, va_list args) {
	if (ali_global_logfile == NULL) ali_global_logfile = stdout;

	if (ali_global_loglevel <= level) {
        fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
        vfprintf(ali_global_logfile, fmt, args);
        fprintf(ali_global_logfile, "\n");
	}
}

void ali_log_log_va(AliLogLevel level, const char* fmt, va_list args) {
	if (ali_global_logfile == NULL) ali_global_logfile = stdout;

	if (ali_global_loglevel <= level) {
        fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
        vfprintf(ali_global_logfile, fmt, args);
	}
}

void ali_log_logn(AliLogLevel level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

    ali_log_logn_va(level, fmt, args);

	va_end(args);
}

void ali_log_log(AliLogLevel level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

    ali_log_log_va(level, fmt, args);

	va_end(args);
}

#endif // ALI_LOG_END
// @module ali_log end

// @module ali_flag
#define ALI_FLAG_LIST_MAX_SIZE 128
AliFlag ali_flag_list[ALI_FLAG_LIST_MAX_SIZE] = {0};
ali_usize ali_flag_list_start = 0;
ali_usize ali_flag_list_size = 0;

void ali_flag_reset(void) {
    ali_flag_list_start = ali_flag_list_size;
    ali_flag_list_size = 0;
}

AliFlag* ali_flag_push(AliFlag flag) {
     ali_assert(ali_flag_list_start + ali_flag_list_size < ALI_FLAG_LIST_MAX_SIZE);
     AliFlag* flag_ = &ali_flag_list[ali_flag_list_start + ali_flag_list_size++];
     *flag_ = flag;
     return flag_;
}

const char** ali_flag_string_ex(const char* name, const char* desc, const char* default_, const char** aliases, ali_usize aliases_count) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .aliases = aliases,
        .aliases_count = aliases_count,
        .type = FLAG_STRING,
        .as.string = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.string;
}

ali_u64* ali_flag_u64_ex(const char* name, const char* desc, ali_u64 default_, const char** aliases, ali_usize aliases_count) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .aliases = aliases,
        .aliases_count = aliases_count,
        .type = FLAG_U64,
        .as.num_u64 = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.num_u64;
}

ali_f64* ali_flag_f64_ex(const char* name, const char* desc, ali_f64 default_, const char** aliases, ali_usize aliases_count) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .aliases = aliases,
        .aliases_count = aliases_count,
        .type = FLAG_F64,
        .as.num_f64 = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.num_f64;
}

ali_bool* ali_flag_option_ex(const char* name, const char* desc, ali_bool default_, const char** aliases, ali_usize aliases_count) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .aliases = aliases,
        .aliases_count = aliases_count,
        .type = FLAG_OPTION,
        .as.option = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.option;
}

void ali_flag_print_help(FILE* sink, const char* program) {
    fprintf(sink, "%s [OPTIONS]\n", program);
    fprintf(sink, "Options:\n");

    for (ali_usize i = ali_flag_list_start; i < ali_flag_list_start + ali_flag_list_size; ++i) {
        AliFlag* flag = &ali_flag_list[i];
        fprintf(sink, "%s ", flag->name);
        if (ali_flag_list[i].aliases != NULL) {
            fprintf(sink, "(aliases: ");
            for (ali_usize j = 0; j < ali_flag_list[i].aliases_count; ++j) {
                fprintf(sink, "'%s'", ali_flag_list[i].aliases[j]);
                if (j != ali_flag_list[i].aliases_count - 1) fprintf(sink, ", ");
            }
            fprintf(sink, ")");
        }
        fprintf(sink, ":\n");
        fprintf(sink, "    %s", flag->description);
        switch (flag->type) {
            case FLAG_STRING:
                if (flag->as.string != NULL) {
                    fprintf(sink, " (default: %s)\n", flag->as.string);
                }
                break;
            case FLAG_U64:
                fprintf(sink, " (default: %lu)\n", flag->as.num_u64);
                break;
            case FLAG_F64:
                fprintf(sink, " (default: %lf)\n", flag->as.num_f64);
                break;
            case FLAG_OPTION:
                fprintf(sink, " (default: %s)\n", flag->as.option ? "true" : "false");
                break;
        }
    }
}

ali_bool ali_flag_parse(int* argc, char*** argv, const char* program) {
while_loop: while (*argc > 0) {
        char* arg = ali_shift_args(argc, argv);

        if (*arg == '-') {
            ali_bool found = false;
            for (ali_usize j = ali_flag_list_start; j < ali_flag_list_start + ali_flag_list_size; ++j) {
                ali_bool found_ = false;
                if (strcmp(arg, ali_flag_list[j].name) == 0) found_ |= true;
                if (ali_flag_list[j].aliases != NULL) {
                    for (ali_usize k = 0; !found_ && k < ali_flag_list[j].aliases_count; ++k) {
                        found_ |= strcmp(ali_flag_list[j].aliases[k], arg) == 0;
                    }
                }
                if (!found_) continue;

                switch (ali_flag_list[j].type) {
                    case FLAG_STRING:
                        if (argc == 0) {
                            ali_logn_error("%s requires an arguement", ali_flag_list[j].name);
                            return false;
                        }
                        ali_flag_list[j].as.string = ali_shift_args(argc, argv);
                        found = true;
                        break;
                    case FLAG_U64: {
                        if (argc == 0) {
                            ali_logn_error("%s requires an arguement", ali_flag_list[j].name);
                            return false;
                        }
                        AliSv sv = ali_sv_from_cstr(ali_shift_args(argc, argv));
                        if (!ali_sv_chop_u64(&sv, &ali_flag_list[j].as.num_u64)) return false;
                        found = true;
                    }break;
                    case FLAG_F64: {
                        if (argc == 0) {
                            ali_logn_error("%s requires an arguement", ali_flag_list[j].name);
                            return false;
                        }
                        AliSv sv = ali_sv_from_cstr(ali_shift_args(argc, argv));
                        if (!ali_sv_chop_f64(&sv, &ali_flag_list[j].as.num_f64)) return false;
                        found = true;
                    }break;
                    case FLAG_OPTION:
                        ali_flag_list[j].as.option = true;
                        found = true;
                        break;
                }
                goto while_loop;
            }

            if (strcmp(arg, "-h") == 0) {
                ali_flag_print_help(stdout, program);
                exit(0);
            }

            if (!found) {
                ali_logn_error("Unknown flag %s", arg);
                return false;
            }
        } else {
            return true;
        }
    }

    return true;
}

// @module ali_flag end

// @module ali_allocator

void* ali_alloc_ex(AliAllocator allocator, ali_usize size, ali_usize alignment) {
    return allocator.alloc(allocator.data, size, alignment);
}

void* ali_realloc_ex(AliAllocator allocator, void* ptr, ali_usize old_size, ali_usize new_size, ali_usize alignment) {
    return allocator.realloc(allocator.data, ptr, old_size, new_size, alignment);
}

void ali_free(AliAllocator allocator, void* ptr) {
    allocator.free(allocator.data, ptr);
}

void* ali_memdup(AliAllocator allocator, void* from, ali_usize len) {
    void* to = ali_alloc(allocator, len);
    ali_memcpy(from, to, len);
    return to;
}

void* ali_sprintf(AliAllocator allocator, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    ali_isize n = vsnprintf(NULL, 0, fmt, args);
    ali_assert(n >= 0);

    va_end(args);
    va_start(args, fmt);

    char* str = ali_alloc(allocator, n + 1);
    vsprintf(str, fmt, args);
    str[n] = 0;

    va_end(args);
    return str;
}

void* ali_libc_alloc(void* data, ali_usize size, ali_usize alignment) {
    ALI_UNUSED(data);
    ALI_UNUSED(alignment);
    return malloc(size);
}

void* ali_libc_realloc(void* data, void* ptr, ali_usize old_size, ali_usize new_size, ali_usize alignment) {
    ALI_UNUSED(data);
    ALI_UNUSED(old_size);
    ALI_UNUSED(alignment);
    return realloc(ptr, new_size);
}

void ali_libc_free(void* data, void* ptr) {
    ALI_UNUSED(data);
    return free(ptr);
}

AliAllocator ali_libc_allocator = {
    .alloc = ali_libc_alloc,
    .realloc = ali_libc_realloc,
    .free = ali_libc_free,
    .data = NULL,
};

// @module ali_allocator end

// @module ali_arena

AliRegion* ali_region_new(ali_usize capacity) {
	AliRegion* new = malloc(sizeof(*new) + capacity);
	ali_assert(new != NULL);
	new->count = 0;
	new->capacity = capacity;
	new->next = NULL;
	return new;
}

void* ali_region_alloc(AliRegion* self, ali_usize size, ali_usize alignment) {
    ali_assert(size < ALI_REGION_DEFAULT_CAP);

	if (self->count + size >= self->capacity) return NULL;
	self->count += self->count % alignment;
	void* ptr = self->data + self->count;
	self->count += size;
	return ptr;
}

void* ali_arena_alloc(void* data, ali_usize size, ali_usize alignment) {
    AliArena* self = (AliArena*)data;
    ali_assert(self != NULL);

    if (self->region_capacity == 0) self->region_capacity = ALI_REGION_DEFAULT_CAP;
    ali_assert(self->region_capacity >= size);

	if (self->start == NULL) {
        self->start = ali_region_new(self->region_capacity);
        self->end = self->start;
	}

	AliRegion* region = self->end;
	void* ptr;
	do {
        ptr = ali_region_alloc(region, size, alignment);
        if (ptr == NULL) {
        	if (region->next == NULL) {
        		region->next = ali_region_new(ALI_REGION_DEFAULT_CAP);
        	}
        	region = region->next;
        }
	} while (ptr == NULL);
	self->end = region;

	return ptr;
}

void* ali_arena_realloc(void* data, void* ptr, ali_usize oldsize, ali_usize newsize, ali_usize alignment) {
    AliArena* arena = (AliArena*)data;
    ali_assert(arena != NULL);

    void* copy = ali_arena_alloc(arena, newsize, alignment);
    ali_memcpy(copy, ptr, oldsize);
    return copy;
}

void ali_arena_free_free(void* data, void* ptr) {
    ALI_UNUSED(data);
    ALI_UNUSED(ptr);
}

void ali_arena_reset(AliArena* self) {
	for (AliRegion* r = self->start; r != NULL; r = r->next) {
        r->count = 0;
	}
	self->end = self->start;
}

void ali_arena_free(AliArena* self) {
	while (self->start != NULL) {
        AliRegion* next = self->start->next;
        free(self->start);
        self->start = next;
	}
	self->end = NULL;
}

AliAllocator ali_arena_allocator(AliArena* arena) {
    return (AliAllocator) {
        .alloc = ali_arena_alloc,
        .realloc = ali_arena_realloc,
        .free = ali_arena_free_free,
        .data = arena,
    };
}

AliArenaMark ali_arena_mark(AliArena* self) {
	return (AliArenaMark) { self->end, self->end->count };
}

void ali_arena_rollback(AliArena* self, AliArenaMark mark) {
	if (mark.r == NULL) {
        ali_arena_reset(self);
        return;
	}

	mark.r->count = mark.count;
	for (AliRegion* r = mark.r; r != NULL; r = r->next) {
        r->count = 0;
	}

	self->end = mark.r;
}

// @module ali_arena end

// @module ali_bump

AliBump ali_bump_from_buffer(void* buffer, ali_usize buffer_size) {
    return (AliBump) {
        .buffer = buffer,
        .size = 0,
        .capacity = buffer_size,
    };
}

void* ali_bump_alloc(void* data, ali_usize size, ali_usize alignment) {
    AliBump* bump = (AliBump*)data;

    ali_assert(bump->size + size <= bump->capacity);
    bump->size += bump->size % alignment;
    void* ptr = bump->buffer + bump->size;
    bump->size += size;
    return ptr;
}

void* ali_bump_realloc(void* data, void* ptr, ali_usize old_size, ali_usize new_size, ali_usize alignment) {
    AliBump* bump = (AliBump*)data;

    ali_assert(bump->size + new_size <= bump->capacity);
    bump->size += bump->size % alignment;
    void* realloced = bump->buffer + bump->size;
    bump->size += new_size;
    ali_memcpy(realloced, ptr, old_size);
    return ptr;
}

void ali_bump_free(void* data, void* ptr) {
    ALI_UNUSED(data);
    ALI_UNUSED(ptr);
}

AliAllocator ali_bump_allocator(AliBump* bump) {
    return (AliAllocator) {
        .alloc = ali_bump_alloc,
        .realloc = ali_bump_realloc,
        .free = ali_bump_free,
        .data = bump,
    };
}

// @module ali_bump end

// @module ali_testing

void ali_testing_run(AliTesting* t, ali_test_t test) {
    test(t);
}

void ali_testing_print(AliTesting* t) {
    if (t->error_count != 0) {
        ali_logn_error("%lu test(s) failed", t->error_count);
    }
}

void ali_testing__fail(AliTesting* t, const char* file, int line, const char* func) {
    ali_logn_error("%s:%d: %s: FAIL", file, line, func);
    t->error_count++;
}

ali_bool ali_testing__expect(AliTesting* t, ali_bool ok, const char* file, int line, const char* func, const char* fmt, ...) {
    if (!ok) {
        va_list args;

        va_start(args, fmt);
        ali_log_log(LOG_ERROR, "%s:%d: %s: ", file, line, func);
        va_end(args);

        va_start(args, fmt);
        ali_log_logn_va(LOG_ERROR, fmt, args);
        va_end(args);

        t->error_count++;
    }
    return ok;
}

// @module ali_testing end

// @module ali_da
void* ali_da_create(AliAllocator allocator, ali_usize init_capacity, ali_usize item_size) {
    AliDaHeader* header = ali_alloc(allocator, sizeof(*header) + item_size * init_capacity);
    header->allocator = allocator;
    header->count = 0;
    header->capacity = init_capacity;
    return header->data;
}

AliDaHeader* ali_da_get_header(void* da) {
    ali_assert(da != NULL);
	return (AliDaHeader*)da - 1;
}

void* ali_da_maybe_resize_with_size(void* da, ali_usize to_add, ali_usize item_size) {
    ali_assert(da != NULL);
    AliDaHeader* h = ali_da_get_header(da);

    if (h->count + to_add >= h->capacity) {
        ali_usize old_capacity = h->capacity;
        while (h->count + to_add >= h->capacity) {
            if (h->capacity == 0) h->capacity = 8;
            else h->capacity *= 8;
        }
        h = ali_realloc(h->allocator, h, sizeof(*h) + old_capacity * item_size, sizeof(*h) + h->capacity * item_size);
    }
    return h->data;
}

void* ali_da_free(void* da) {
    if (da != NULL) {
        AliDaHeader* h = ali_da_get_header(da);
        ali_free(h->allocator, h);
    }
	return (da = NULL);
}

// @module ali_da end

// @module ali_slice

AliSlice ali_slice_from_parts_with_size(void* data, ali_usize count, ali_usize item_size) {
    AliSlice slice = {
        .item_size = item_size,
        .count = count,
        .data = data,
    };
    return slice;
}

AliSlice ali_slice_to_da_with_size(void* da, ali_usize item_size) {
    AliSlice slice = {
        .count = ali_da_getlen(da),
        .item_size = item_size,
        .data = da,
    };
    return slice;
}

AliSlice ali_da_slice_with_size(void* da, ali_usize start, ali_usize end_exclusive, ali_usize item_size) {
    ali_assert(start < ali_da_getlen(da));
    ali_assert(end_exclusive <= ali_da_getlen(da));

    AliSlice slice = {
        .count = end_exclusive - start,
        .item_size = item_size,
        .data = (ali_u8*)(da) + start * item_size
    };
    return slice;
}

AliSlice ali_slice_cstr(char* str, ali_usize start, ali_usize end_exclusive) {
    ali_assert(start < strlen(str));
    ali_assert(end_exclusive <= strlen(str));

    AliSlice slice = {
        .item_size = 1,
        .count = end_exclusive - start,
        .data = (ali_u8*)str + start,
    };
    return slice;
}

AliSlice ali_slice_sv(AliSv sv, ali_usize start, ali_usize end_exclusive) {
    ali_assert(start < sv.len);
    ali_assert(end_exclusive <= sv.len + 1);

    AliSlice slice = {
        .data = (ali_u8*)(sv.start + start),
        .count = end_exclusive - start,
        .item_size = 1,
    };
    return slice;
}

AliSlice ali_slice_slice(AliSlice slice, ali_usize start, ali_usize end_exclusive) {
    ali_assert(start < slice.count);
    ali_assert(end_exclusive <= slice.count);

    AliSlice slice_ = {
        .count = end_exclusive - start,
        .item_size = slice.item_size,
        .data = slice.data + start * slice.item_size,
    };
    return slice_;
}

void* ali_slice_get(AliSlice slice, ali_usize i) {
    return slice.data + i * slice.item_size; 
}

// @module ali_slice end

// @module ali_temp_alloc

static ali_u8 ali_temp_buffer[ALI_TEMP_BUF_SIZE] = {0};
static ali_usize ali_temp_buffer_size = 0;

void* ali_temp_alloc(ali_usize size) {
	ali_assert(ali_temp_buffer_size + size < ALI_TEMP_BUF_SIZE);
	void* ptr = ali_temp_buffer + ali_temp_buffer_size;
	ali_temp_buffer_size += size;
	return ptr;
}

void* ali_temp_memdup(void* data, ali_usize data_size) {
	void* mem = ali_temp_alloc(data_size);
	ali_memcpy(mem, data, data_size);
	return mem;
}

char* ali_temp_strdup(const char* str) {
	ali_usize size = strlen(str) + 1;
	char* copy = ali_temp_alloc(size);
	return ali_memcpy(copy, str, size);
}

char* ali_temp_strndup(const char* start, ali_usize count) {
    char* copy = ali_temp_alloc(count + 1);
    ali_memcpy(copy, start, count);
    copy[count] = 0;
    return copy;
}

char* ali_temp_sprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	char* out = ali_temp_alloc(n + 1);
	va_start(args, fmt);
	vsnprintf(out, n + 1, fmt, args);
	va_end(args);

	return out;
}

void* ali_temp_get_cur() {
    return &ali_temp_buffer[ali_temp_buffer_size];
}

void ali_temp_push(char c) {
    ali_assert(ali_temp_buffer_size < ALI_TEMP_BUF_SIZE);
    ali_temp_buffer[ali_temp_buffer_size++] = c;
}

void ali_temp_push_str(const char* str) {
    ali_usize len = strlen(str);
    ali_assert(ali_temp_buffer_size + len < ALI_TEMP_BUF_SIZE);
    ali_memcpy(ali_temp_buffer + ali_temp_buffer_size, str, len);
    ali_temp_buffer_size += len;
}

ali_usize ali_temp_stamp(void) {
	return ali_temp_buffer_size;
}

void ali_temp_rewind(ali_usize stamp) {
	ali_temp_buffer_size = stamp;
}

void ali_temp_reset(void) {
	ali_temp_buffer_size = 0;
}

// @module ali_temp_alloc end

// @module ali_utf8
ali_usize ali_utf8len(const ali_utf8* utf8) {
	ali_usize len = 0;
	while (*utf8 != 0) {
        ali_usize rune_size = 0;
        ali_utf8c_to_rune(utf8, &rune_size);
        len++;
        utf8 += rune_size;
	}
	return len;
}

ali_bool ali_rune_is_valid(ali_rune rune) {
	if (rune > 0x10FFFF) return false;
	return true;
}

ali_rune ali_utf8c_to_rune(const ali_utf8* utf8c, ali_usize* rune_size) {
	ali_rune rune = 0;
	ali_usize rune_size_ = 0;

	if ((utf8c[0] & 0x80) == 0x00) {
        rune = utf8c[0];
        rune_size_ = 1;
	} else if ((utf8c[0] & 0xE0) == 0xC0) {
        rune = ((utf8c[0] & 0x1F) << 6*1) | ((utf8c[1] & 0x3F) << 6*0);
        rune_size_ = 2;
	} else if ((utf8c[0] & 0xF0) == 0xE0) {
        rune = ((utf8c[0] & 0x1F) << 6*2) | ((utf8c[1] & 0x3F) << 6*1) | ((utf8c[2] & 0x3F) << 6*0);
        rune_size_ = 3;
	} else if ((utf8c[0] & 0xF8) == 0xF0) {
        rune = ((utf8c[0] & 0x1F) << 6*3) | ((utf8c[1] & 0x3F) << 6*2) | ((utf8c[2] & 0x3F) << 6*1) | ((utf8c[3] & 0x3F) << 6*0);
        rune_size_ = 4;
	} else {
        // invalid
        return -1;
	}

	if (rune_size) *rune_size = rune_size_;
	return rune;
}

ali_rune* ali_utf8_to_runes(AliAllocator allocator, const ali_utf8* utf8, ali_usize* count) {
	ali_usize len = ali_utf8len(utf8);
	*count = len;

	ali_rune* runes = ali_alloc(allocator, len * sizeof(*runes));
	len = 0;
	while (*utf8 != 0) {
        ali_usize rune_size = 0;
        runes[len++] = ali_utf8c_to_rune(utf8, &rune_size);
        utf8 += rune_size;
	}

	return runes;
}

ali_usize ali_rune_size(ali_rune rune) {
	if (rune > 0x10FFFF) return 0;
	else if (rune > 0xFFFF) return 4;
	else if (rune > 0x07FF) return 3;
	else if (rune > 0x007F) return 2;
	else return 1;
}

const ali_utf8* ali_rune_to_utf8(ali_rune rune) {
static ali_utf8 utf8[5] = {0};
	
	ali_usize rune_size = ali_rune_size(rune);
	if (rune_size == 4) {
        utf8[0] = 0xF0 | ((rune >> 6*3) & 0x07);
        utf8[1] = 0x80 | ((rune >> 6*2) & 0x3F);
        utf8[2] = 0x80 | ((rune >> 6*1) & 0x3F);
        utf8[3] = 0x80 | ((rune >> 6*0) & 0x3F);
	} else if (rune_size == 3) {
        utf8[0] = 0xE0 | ((rune >> 6*2) & 0x0F);
        utf8[1] = 0x80 | ((rune >> 6*1) & 0x3F);
        utf8[2] = 0x80 | ((rune >> 6*0) & 0x3F);
	} else if (rune_size == 2) {
        utf8[0] = 0xC0 | ((rune >> 6*1) & 0x1F);
        utf8[1] = 0x80 | ((rune >> 6*0) & 0x3F);
	} else if (rune_size == 1) {
        utf8[0] = rune;
	} else {
        // invalid
        return NULL;
	}
	utf8[rune_size] = 0;

	return utf8;
}

ali_utf8* ali_runes_to_utf8(AliAllocator allocator, ali_rune* runes, ali_usize len) {
	ali_usize real_len = 0;
	for (ali_usize i = 0; i < len; ++i) {
        real_len += ali_rune_size(runes[i]);
	}

	ali_utf8* utf8 = ali_alloc(allocator, real_len + 1);
	ali_utf8* utf8p = utf8;
	for (ali_usize i = 0; i < len; ++i) {
        ali_usize rune_size = ali_rune_size(runes[i]);
        memcpy(utf8p, ali_rune_to_utf8(runes[i]), rune_size);
        utf8p += rune_size;
	}
	utf8[real_len] = 0;

	return utf8;
}

ali_rune* ali_temp_utf8_to_runes(const ali_utf8* utf8, ali_usize* count) {
	ali_rune* runes = ali_utf8_to_runes(ali_libc_allocator, utf8, count);
	ali_rune* out = ali_temp_alloc(sizeof(*out) * (*count));
	ali_memcpy(out, runes, sizeof(*out) * (*count));
	ali_free(ali_libc_allocator, runes);
	return out;
}

ali_utf8* ali_temp_runes_to_utf8(ali_rune* runes, ali_usize len) {
	ali_utf8* utf8 = ali_runes_to_utf8(ali_libc_allocator, runes, len);
	ali_utf8* out = (ali_utf8*)ali_temp_strdup((char*)utf8);
	ali_free(ali_libc_allocator, utf8);
	return out;
}

// @module ali_utf8 end

// @module ali_sv

AliSv ali_sv_from_parts(char* start, ali_usize len) {
    AliSv sv = {
        .start = start,
        .len = len
    };
    return sv;
}

AliSv ali_sv_from_cstr(char* cstr) {
    AliSv sv = {
        .start = cstr,
        .len = strlen(cstr)
    };
    return sv;
}

AliSv ali_sv_from_slice(AliSlice slice) {
    ali_assert(slice.item_size == 1);
    return (AliSv) {
        .start = (char*)slice.data,
        .len = slice.count,
    };
}

void ali_sv_step(AliSv* self) {
	if (self->start == NULL || self->len == 0) return;
	self->start += 1;
	self->len -= 1;
}

AliSv ali_sv_trim_left(AliSv self) {
	while (self.len > 0 && isspace(*self.start)) {
        ali_sv_step(&self);
	}
	return self;
}

AliSv ali_sv_trim_right(AliSv self) {
	while (self.len > 0 && isspace(self.start[self.len - 1])) {
        self.len -= 1;
	}
	return self;
}

AliSv ali_sv_trim(AliSv self) {
	return ali_sv_trim_left(ali_sv_trim_right(self));
}

AliSv ali_sv_chop_by_c(AliSv* self, char c) {
	AliSv chopped = { .start = self->start, .len = 0 };

	while (*self->start != c) {
        ali_sv_step(self);

        if (self->len == 0) break;
	}

	chopped.len = self->start - chopped.start;
	ali_sv_step(self);

	return chopped;
}

ali_bool ali_sv_chop_u64_bin(AliSv* sv, ali_u64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    ali_u64 number = 0;
    while (sv->len > 0 && isdigit(sv->start[0])) {
        ali_u64 digit = sv->start[0] - '0';
        if (digit > 1) return false;
        number <<= 1;
        number += digit;
        ali_sv_step(sv);
    }

    *out = number;
    return true;
}

ali_bool ali_sv_chop_u64_oct(AliSv* sv, ali_u64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    ali_u64 number = 0;
    while (sv->len > 0 && isdigit(sv->start[0])) {
        ali_u64 digit = sv->start[0] - '0';
        if (digit > 7) return false;
        number *= 8;
        number += digit;
        ali_sv_step(sv);
    }

    *out = number;
    return true;
}

ali_bool ali_sv_chop_u64_dec(AliSv* sv, ali_u64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    ali_u64 number = 0;
    while (sv->len > 0 && isdigit(sv->start[0])) {
        number *= 10;
        number += sv->start[0] - '0';
        ali_sv_step(sv);
    }

    *out = number;
    return true;
}

ali_bool ali_sv_chop_u64_hex(AliSv* sv, ali_u64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    ali_u64 number = 0;
    while (sv->len > 0 && isalnum(sv->start[0])) {
        ali_u64 digit = 0;
        if (isdigit(sv->start[0])) {
            digit = sv->start[0] - '0';
        } else {
            digit = sv->start[0] & 0x20 ? sv->start[0] - 'a' : sv->start[0] - 'A';
            digit += 10;
            if (digit > 15) return false;
        }

        number *= 16;
        number += digit;
        ali_sv_step(sv);
    }

    *out = number;
    return true;
}

ali_bool ali_sv_chop_u64(AliSv* sv, ali_u64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    if (ali_sv_chop_prefix(sv, ali_sv_from_cstr("0b"))) {
        return ali_sv_chop_u64_bin(sv, out);
    }

    if (sv->len >= 2 && sv->start[0] == '0' && isdigit(sv->start[1])) {
        sv->start += 1;
        sv->len -= 1;
        return ali_sv_chop_u64_oct(sv, out);
    }

    if (ali_sv_chop_prefix(sv, ali_sv_from_cstr("0x"))) {
        return ali_sv_chop_u64_hex(sv, out);
    }

    return ali_sv_chop_u64_dec(sv, out);
}

ali_bool ali_sv_chop_f64(AliSv* sv, ali_f64* out) {
    if (sv->len == 0 || !isdigit(sv->start[0])) return false;

    ali_f64 number = 0;
    while (sv->len > 0 && isdigit(sv->start[0])) {
        number *= 10;
        number += sv->start[0] - '0';
        ali_sv_step(sv);
    }

    if (sv->len > 0 && sv->start[0] == '.') {
        ali_sv_step(sv);
        ali_f64 after_decimal = 10;
        while (sv->len > 0 && isdigit(sv->start[0])) {
            number += (sv->start[0] - '0') / after_decimal;
            after_decimal *= 10;
            ali_sv_step(sv);
        }
    }

    *out = number;
    return true;
}

AliSv ali_sv_chop_left(AliSv* self, ali_usize n) {
	if (self->len < n) n = self->len;

	AliSv chunk = ali_sv_from_parts(self->start, n);
	self->start += n;
	self->len -= n;

	return chunk;
}

AliSv ali_sv_chop_right(AliSv* self, ali_usize n) {
	if (self->len < n) n = self->len;

	AliSv chunk = ali_sv_from_parts(self->start + self->len - n, n);
	self->len -= n;

	return chunk;
}

ali_bool ali_sv_eq(AliSv left, AliSv right) {
	ali_bool eq = left.len == right.len;
	for (ali_usize i = 0; eq && i < left.len; ++i) {
        eq &= left.start[i] == right.start[i];
	}
	return eq;
}

ali_bool ali_sv_starts_with(AliSv self, AliSv prefix) {
	if (self.len < prefix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start, prefix.len), prefix);
}

ali_bool ali_sv_ends_with(AliSv self, AliSv suffix) {
	if (self.len < suffix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start + self.len - suffix.len, suffix.len), suffix);
}

ali_bool ali_sv_chop_prefix(AliSv* self, AliSv prefix) {
    if (!ali_sv_starts_with(*self, prefix)) return false;
    self->start += prefix.len;
    self->len -= prefix.len;
    return true;
}

ali_bool ali_sv_chop_suffix(AliSv* self, AliSv suffix) {
    if (!ali_sv_ends_with(*self, suffix)) return false;
    self->len -= suffix.len;
    return true;
}

char* ali_temp_sv_to_cstr(AliSv sv) {
	char* out = ali_temp_alloc(sv.len + 1);
	ali_memcpy(out, sv.start, sv.len);
	out[sv.len] = 0;
	return out;
}

// @module ali_sv end

// @module ali_sb
void ali_sb_maybe_resize(AliSb* self, ali_usize to_add) {
	if (self->count + to_add >= self->capacity) {
        while (self->count + to_add >= self->capacity) {
        	if (self->capacity == 0) self->capacity = ALI_DA_DEFAULT_INIT_CAPACITY;
        	else self->capacity *= 2;
        }

        self->data = realloc(self->data, self->capacity);
	}
}

void ali_sb_push_strs_null(AliSb* self, ...) {
	va_list args;
	va_start(args, self);

	const char* str = va_arg(args, const char*);
	while (str != NULL) {
        ali_usize n = strlen(str);
        ali_sb_maybe_resize(self, n);
        ali_memcpy(self->data + self->count, str, n);
        self->count += n;
        str = va_arg(args, const char*);
	}

	va_end(args);
}

void ali_sb_push_nstr(AliSb* self, char* start, ali_usize n) {
    ali_sb_maybe_resize(self, n);
    ali_memcpy(self->data + self->count, start, n);
    self->count += n;
}

void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* str;
	ali_assert(vasprintf(&str, fmt, args) == 0);
	ali_sb_push_strs(self, str);
	free(str);

	va_end(args);
}

void ali_sb_free(AliSb* self) {
	free(self->data);
	self->data = NULL;
	self->count = 0;
	self->capacity = 0;
}

ali_bool ali_sb_read_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "rb");
	if (f == NULL) {
        ali_logn_error("Couldn't read %s: %s", path, strerror(errno));
        return false;
	}

	fseek(f, 0, SEEK_END);
	ali_usize n = ftell(f);
	fseek(f, 0, SEEK_SET);

	ali_sb_maybe_resize(self, n);
	fread(self->data + self->count, n, 1, f);
	self->count += n;

	if (f != NULL) fclose(f);
	return true;
}

ali_bool ali_sb_read_file_by_chunks(AliSb* self, const char* path) {
    ali_bool result = true;

    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        ali_logn_error("Couldn't open %s: %s", path, strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    char buffer[1024];
    while (true) {
        ali_isize n = fread(buffer, 1, sizeof(buffer), f);
        if (n < 0) {
            ali_logn_error("Couldn't read from %s: %s", path, strerror(errno));
            return false;
        } else if (n == 0) {
            break;
        }

        ali_sb_push_nstr(self, buffer, n);
    }

defer:
    if (f != NULL) fclose(f);
    return result;
}

ali_bool ali_sb_write_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "wb");
	if (f == NULL) {
        ali_logn_error("Couldn't write to %s: %s", path, strerror(errno));
        return false;
	}

	fwrite(self->data, 1, self->count, f);

	if (f != NULL) fclose(f);
	return true;
}

// @module ali_sb end

// @module ali_zlib

#ifdef ALI_ZLIB
#include <zlib.h>

ali_bool ali_zlib_compress(AliSb* out, const ali_u8* to_compress, ali_usize to_compress_size) {
    ali_bool result = true;

    z_stream stream;
    stream.opaque = Z_NULL;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;

    int ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        ali_logn_error("Couldn't initialize deflate stream: %s", zError(ret));
        ALI_RETURN_DEFER(false);
    }

    stream.avail_in = to_compress_size;
    stream.next_in = (ali_u8*)to_compress;

    ali_u8 buffer[(4 << 10)];
    do {
        stream.avail_out = (4 << 10);
        stream.next_out = buffer;

        ret = deflate(&stream, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            ali_logn_error("Couldn't deflate: %s", zError(ret));
            ALI_RETURN_DEFER(false);
        }

        ali_usize have = (4 << 10) - stream.avail_out;
        ali_sb_push_nstr(out, (char*)buffer, have);
    } while (stream.avail_out == 0);

defer:
    deflateEnd(&stream);
    return result;
}

ali_bool ali_zlib_decompress(AliSb* out, const ali_u8* to_decompress, ali_usize to_decompress_size) {
    ali_bool result = true;

    z_stream stream;
    stream.opaque = Z_NULL;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;

    int ret = inflateInit(&stream);
    if (ret != Z_OK) {
        ali_logn_error("Couldn't initialize inflate stream: %s", zError(ret));
        ALI_RETURN_DEFER(false);
    }

    stream.avail_in = to_decompress_size;
    stream.next_in = (ali_u8*)to_decompress;

    ali_u8 buffer[(4 << 10)];
    do {
        stream.avail_out = (4 << 10);
        stream.next_out = buffer;

        ret = inflate(&stream, Z_NO_FLUSH);
        switch (ret) {
            case Z_NEED_DICT:
            case Z_STREAM_ERROR:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                ali_logn_error("Couldn't inflate: %s", zError(ret));
                ALI_RETURN_DEFER(false);
        }

        ali_usize have = (4 << 10) - stream.avail_out;
        ali_sb_push_nstr(out, (char*)buffer, have);
    } while (stream.avail_out == 0);

defer:
    inflateEnd(&stream);
    return result;
}

#endif // ALI_ZLIB
// @module ali_zlib end

// @module ali_measure

double ali_get_now() {
	struct timespec ts;
	ali_assert(clock_gettime(CLOCK_REALTIME, &ts) == 0);
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

typedef struct {
	const char* name;

	double start;

	double total;
	ali_usize count;
}Measurement;

static Measurement measurements[ALI_MEASUREMENTS_COUNT] = {0};
static ali_usize measurements_count = 0;

void ali_measurement_push(Measurement measurement) {
	ali_assert(measurements_count < ALI_MEASUREMENTS_COUNT);
	measurements[measurements_count++] = measurement;
}

Measurement* ali_find_measurement(const char* name) {
	Measurement* found = NULL;

	for (ali_usize i = 0; found == NULL && i < measurements_count; ++i) {
        if (strcmp(measurements[i].name, name) == 0) {
        	found = &measurements[i];
        	break;
        }
	}

	return found;
}

void ali_measure_start(const char* name) {
	Measurement* found = ali_find_measurement(name);
	if (found == NULL) {
        Measurement measurement = { .name = name, .start = ali_get_now() };
        ali_measurement_push(measurement);
        return;
	}

	found->start = ali_get_now();
}

void ali_measure_end(const char* name) {
	Measurement* found = ali_find_measurement(name);
	ali_assert(found != NULL);

	found->total += ali_get_now() - found->start;
	found->count += 1;
}

void ali_print_measurements(void) {
	for (ali_usize i = 0; i < measurements_count; ++i) {
        ali_logn_info("[ali_measure] %s: %lfs", measurements[i].name, measurements[i].total / measurements[i].count);
	}
}

// @module ali_measure end

// @module ali_cmd

void ali_cmd_append_args_(char*** cmd, ...) {
    va_list args;
    va_start(args, cmd);

    char* arg = va_arg(args, char*);
    while (arg != NULL) {
        ali_cmd_append_arg(*cmd, arg);
        arg = va_arg(args, char*);
    }

    va_end(args);
}

char* ali_cmd_render(char** cmd) {
    char* render = ali_temp_get_cur();

    for (ali_usize i = 0; i < ali_da_getlen(cmd); ++i) {
        char* arg = cmd[i];
        if (arg == NULL) break;
        if (i > 0) ali_temp_push(' ');
        if (ali_strchr(arg, ' ') != 0) {
            ali_temp_push_str(arg);
        } else {
            ali_temp_push('\'');
            ali_temp_push_str(arg);
            ali_temp_push('\'');
        }
    }
    ali_temp_push(0);

    return render;
}

pid_t ali_cmd_run_async_redirect(char** cmd, AliCmdRedirect* redirect) {
    ali_usize stamp = ali_temp_stamp();
    char* render = ali_cmd_render(cmd);
    ali_logn_info("[CMD] %s", render);
    ali_temp_rewind(stamp);

    ali_bool should_redirect_stdin = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDIN) != 0;
    ali_bool should_redirect_stdout = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDOUT) != 0;
    ali_bool should_redirect_stderr = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDERR) != 0;

    if (should_redirect_stdout || should_redirect_stderr) {
        if (pipe(redirect->out_pipe) < 0) {
            ali_logn_error("Couldn't create pipe: %s", strerror(errno));
            return -1;
        }
    }

    if (should_redirect_stdin) {
        if (pipe(redirect->in_pipe) < 0) {
            ali_logn_error("Couldn't create pipe: %s", strerror(errno));
            return -1;
        }
    }

    ali_cmd_shallow_append_arg(cmd, NULL);
    pid_t pid = fork();
    if (pid < 0) {
        ali_logn_error("Couldn't fork: %s", strerror(errno));
        return -1;
    } else if (pid == 0) {
        if (should_redirect_stdin) {
            if (!ali_dup2_logged(redirect->in_pipe[0], 0)) exit(1);
        }

        if (should_redirect_stdout) {
            if (!ali_dup2_logged(redirect->out_pipe[1], 1)) exit(1);
        }

        if (should_redirect_stderr) {
            if (!ali_dup2_logged(redirect->out_pipe[1], 2)) exit(1);
        }

        execvp(cmd[0], cmd);

        ali_logn_error("Couldn't start process: %s", strerror(errno));
        exit(1);
    }
    
    return pid;
}

ali_bool ali_wait_for_process(pid_t pid) {
    for (;;) {
        int wstatus;
        if (waitpid(pid, &wstatus, 0) < 0) {
            ali_logn_error("Couldn't waitpid for %s", strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int estatus = WEXITSTATUS(wstatus);
            if (estatus != 0) {
                ali_logn_error("Process %d exited with status %d", pid, estatus);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            int sig = WTERMSIG(wstatus);
            ali_logn_error("Process %d exited with signal %d (%s)", pid, sig, strsignal(sig));
            return false;
        }
    }

    return true;
}

ali_bool ali_cmd_run_sync(char** cmd) {
    pid_t pid = ali_cmd_run_async(cmd);
    return ali_wait_for_process(pid);
}

ali_bool ali_cmd_run_sync_and_reset(char** cmd) {
    ali_bool ret = ali_cmd_run_sync(cmd);
    ali_da_get_header(cmd)->count = 0;
    return ret;
}

ali_bool ali_needs_rebuild(const char* output, const char** inputs, ali_usize input_count) {
    struct stat st;

    if (stat(output, &st) < 0) {
        if (errno == ENOENT) return true;
        ali_logn_error("Couldn't stat %s: %s", output, strerror(errno));
        return false;
    }

    struct timespec ts_output = st.st_mtim;

    for (ali_usize i = 0; i < input_count; ++i) {
        if (stat(inputs[i], &st) < 0) {
            ali_logn_error("Couldn't stat %s: %s", inputs[i], strerror(errno));
            return false;
        }

        if (st.st_mtim.tv_sec > ts_output.tv_sec) return 1;
    }

    return false;
}

ali_bool ali_needs_rebuild1(const char* output, const char* input) {
    return ali_needs_rebuild(output, &input, 1);
}

ali_bool ali_rename(char*** cmd, const char* from, const char* to) {
    if (*cmd == NULL) ali_da_maybe_resize(*cmd, 1);
    ali_da_get_header(*cmd)->count = 0;
    ali_cmd_append_args(cmd, "mv", from, to);
    return ali_cmd_run_sync_and_reset(*cmd);
}

ali_bool ali_remove(char*** cmd, const char* path) {
    ali_da_get_header(*cmd)->count = 0;
    ali_cmd_append_args(cmd, "rm", path);
    return ali_cmd_run_sync_and_reset(*cmd);
}

ali_bool ali_dup2_logged(int fd1, int fd2) {
    if (dup2(fd1, fd2) < 0) {
        ali_logn_error("Couldn't dup2: %s", strerror(errno));
        return false;
    }
    return true;
}

ali_bool ali_create_dir_if_not_exists(const char* path) {
    if (mkdir(path, 0766) < 0) {
        if (errno != EEXIST) {
            ali_logn_error("Couldn't create %s: %s", path, strerror(errno));
            return false;
        }
    }

    return true;
}

ali_bool ali_create_dir_all_if_not_exists(char* path) {
    ali_usize stamp = ali_temp_stamp();

    char* slash = ali_strchr(path, '/');
    while (slash != NULL) {
        ali_usize count = slash - path;
        char* dir = ali_temp_strndup(path, count);
        if (strcmp(dir, ".") != 0 && strcmp(dir, "..") != 0) {
            if (!ali_create_dir_if_not_exists(dir)) return false;
        }
        slash = ali_strchr(slash + 1, '/');
    }

    ali_temp_rewind(stamp);
    if (!ali_create_dir_if_not_exists(path)) return false;

    ali_logn_info("Created dir %s or already exists", path);

    return true;
}

// @module ali_cmd end

#endif // ALI_IMPLEMENTATION

#ifdef ALI_REMOVE_PREFIX
#ifndef ALI_REMOVE_PREFIX_GUARD_
#define ALI_REMOVE_PREFIX_GUARD_

// @module ali_util
typedef ali_bool bool;

#define STRINGIFY ALI_STRINGIFY
#define STRINGIFY_2 ALI_STRINGIFY_2

#define ARRAY_LEN ALI_ARRAY_LEN
#define INLINE_ARRAY ALI_INLINE_ARRAY

#define UNUSED ALI_UNUSED
#define UNREACHABLE ALI_UNREACHABLE
#define TODO ALI_TODO
#define PANIC ALI_PANIC

#define RETURN_DEFER ALI_RETURN_DEFER

#define FORMAT_ATTRIBUTE ALI_FORMAT_ATTRIBUTE

#define SWAP ALI_SWAP

#define add_u64_checked ali_add_u64_checked
#define sub_u64_checked ali_sub_u64_checked

#define shift ali_shift

#define path_name ali_path_name
#define shift_args ali_shift_args
// @module ali_util end

// @module ali_types
#ifdef ALI_TYPES_
#ifndef ALI_TYPES_ALIASES_
#define ALI_TYPES_ALIASES_
typedef ali_u8 u8;
typedef ali_u16 u16;
typedef ali_u32 u32;
typedef ali_u64 u64;

typedef ali_i8 i8;
typedef ali_i16 i16;
typedef ali_i32 i32;
typedef ali_i64 i64;

typedef ali_f32 f32;

#ifdef __x86_64__
typedef ali_f64 f64;
#endif // __x86_64__

typedef ali_usize usize;
typedef ali_isize isize;
#endif // ALI_TYPES_ALIASES_
#endif // ALI_TYPES_
// @module ali_types end

// @module ali_log

#define global_logfile ali_global_logfile
#define global_loglevel ali_global_loglevel

#define log_logn ali_log_logn
#define log_logn_va ali_log_logn_va
#define log_log ali_log_log
#define log_log_va ali_log_log_va

#define logn_info ali_logn_info
#define logn_warn ali_logn_warn
#define logn_error ali_logn_error

// @module ali_log end

// @module ali_flag

#define flag_string_ex ali_flag_string_ex
#define flag_u64_ex ali_flag_u64_ex
#define flag_f64_ex ali_flag_f64_ex
#define flag_option_ex ali_flag_option_ex

#define flag_string ali_flag_string
#define flag_u64 ali_flag_u64
#define flag_f64 ali_flag_f64
#define flag_option ali_flag_option

#define flag_print_help ali_flag_print_help
#define flag_parse ali_flag_parse
#define flag_parse_with_program ali_flag_parse_with_program

// @module ali_flag end

// @module ali_da
#define da_new_header ali_da_new_header
#define da_get_header ali_da_get_header
#define da_maybe_resize ali_da_maybe_resize
#define da_getlen ali_da_getlen

#define da_append ali_da_append
#define da_shallow_append ali_da_shallow_append
#define da_append_many ali_da_append_many
#define da_free ali_da_free

#define da_reset ali_da_reset

#define da_remove_unordered ali_da_remove_unordered
#define da_remove_ordered ali_da_remove_ordered

#define da_for ali_da_for
#define da_foreach ali_da_foreach
// @module ali_da end

// @module ali_slice

#define slice_from_parts ali_slice_from_parts
#define da_to_slice ali_da_to_slice
#define da_slice ali_da_slice
#define slice_cstr ali_slice_cstr
#define slice_slice ali_slice_slice
#define slice_get ali_slice_get

#define slice_for ali_slice_for
#define slice_foreach ali_slice_foreach

// @module ali_slice end

// @module ali_temp_alloc

#define temp_alloc ali_temp_alloc
#define temp_strdup ali_temp_strdup
#define temp_strndup ali_temp_strndup
#define temp_memdup ali_temp_memdup
#define temp_sprintf ali_temp_sprintf
#define temp_stamp ali_temp_stamp
#define temp_rewind ali_temp_rewind
#define temp_reset ali_temp_reset

// @module ali_temp_alloc end

// @module ali_bump

#define bump_alloc_ex ali_bump_alloc_ex
#define bump_alloc ali_bump_alloc

// @module ali_bump end

// @module ali_arena
#define region_new ali_region_new
#define region_alloc ali_region_alloc

#define arena_alloc_ex ali_arena_alloc_ex
#define arena_alloc ali_arena_alloc
#define arena_memdup ali_arena_memdup
#define arena_strdup ali_arena_strdup
#define arena_sprintf ali_arena_sprintf

#define arena_mark ali_arena_mark
#define arena_rollback ali_arena_rollback
#define arena_reset ali_arena_reset
#define arena_free ali_arena_free
// @module ali_arena end

// @module ali_testing

#define testing_run ali_testing_run
#define testing_print ali_testing_print
#define testing_fail ali_testing_fail
#define testing_expect ali_testing_expect

// @module ali_testing end

// @module ali_utf8
typedef ali_utf8 utf8;
typedef ali_rune rune;

#define UTF8 ALI_UTF8

#define utf8len ali_utf8len
#define utf8c_to_rune ali_utf8c_to_rune
#define utf8_to_runes ali_utf8_to_runes

#define is_rune_valid ali_is_rune_valid
#define rune_size ali_rune_size
#define rune_to_utf8 ali_rune_to_utf8
#define runes_to_utf8 ali_runes_to_utf8

#define temp_utf8_to_runes ali_temp_utf8_to_runes
#define temp_runes_to_utf8 ali_temp_runes_to_utf8
// @module ali_utf8 end

// @module ali_sv
#define SV_FMT ALI_SV_FMT
#define SV_F ALI_SV_F

#define sv_from_cstr ali_sv_from_cstr
#define sv_from_parts ali_sv_from_parts

#define sv_step ali_sv_step
#define sv_trim_left ali_sv_trim_left
#define sv_trim_right ali_sv_trim_right
#define sv_trim ali_sv_trim

#define sv_chop_left ali_sv_chop_left
#define sv_chop_right ali_sv_chop_right
#define sv_chop_by_c ali_sv_chop_by_c

#define sv_chop_u64_bin ali_sv_chop_u64_bin
#define sv_chop_u64_oct ali_sv_chop_u64_oct
#define sv_chop_u64_dec ali_sv_chop_u64_dec
#define sv_chop_u64_hex ali_sv_chop_u64_hex
#define sv_chop_u64 ali_sv_chop_u64

#define sv_chop_f64 ali_sv_chop_f64

#define sv_chop_long ali_sv_chop_long
#define sv_chop_float ali_sv_chop_float
#define sv_chop_double ali_sv_chop_double

#define sv_eq ali_sv_eq
#define sv_starts_with ali_sv_starts_with
#define sv_ends_with ali_sv_ends_with

#define sv_chop_prefix ali_sv_chop_prefix
#define sv_chop_suffix ali_sv_chop_suffix

#define temp_sv_to_cstr ali_temp_sv_to_cstr
// @module ali_sv end

// @module ali_sb
#define sb_maybe_resize ali_sb_maybe_resize
#define sb_push_strs_null ali_sb_push_strs_null
#define sb_push_sprintf ali_sb_push_sprintf
#define sb_free ali_sb_free

#define sb_push_nstr ali_sb_push_nstr
#define sb_push_strs ali_sb_push_strs
#define sb_to_sv ali_sb_to_sv

#define sb_read_file ali_sb_read_file
#define sb_read_file_by_chunks ali_sb_read_file_by_chunks

#define sb_write_file ali_sb_write_file
// @module ali_sb end

// @module ali_measure
#define measure_start ali_measure_start
#define measure_end ali_measure_end

#define print_measurements ali_print_measurements
// @module ali_measure end

// @module ali_cmd

#define cmd_append_arg ali_cmd_append_arg
#define cmd_append_args ali_cmd_append_args
#define wait_for_process ali_wait_for_process
#define cmd_render ali_cmd_render
#define cmd_run_async_redirect ali_cmd_run_async_redirect
#define cmd_run_async ali_cmd_run_async
#define cmd_run_sync ali_cmd_run_sync
#define cmd_run_sync_and_reset ali_cmd_run_sync_and_reset

#define needs_rebuild ali_needs_rebuild
#define needs_rebuild1 ali_needs_rebuild1
#define create_dir_if_not_exists ali_create_dir_if_not_exists
#define create_dir_all_if_not_exists ali_create_dir_all_if_not_exists
#define rebuild_yourself ali_rebuild_yourself

// @module ali_cmd end

#endif // ALI_REMOVE_PREFIX_GUARD_
#endif // ALI_REMOVE_PREFIX

#ifdef ALI_INTERNAL_TESTING
static void test_add_and_sub_u64_ckecked(AliTesting* testing) {
    u64 res;

    u64 a1 = 1;
    u64 b1 = 1;
    ali_testing_expect(testing, ali_add_u64_checked(a1, b1, &res), "adding 1 + 1 should succeed");

    u64 a2 = UINT64_MAX;
    u64 b2 = 1;
    ali_testing_expect(testing, !ali_add_u64_checked(a2, b2, &res), "adding UINT64_MAX + 1 should fail");

    u64 a3 = 1;
    u64 b3 = 1;
    ali_testing_expect(testing, ali_sub_u64_checked(a3, b3, &res), "subtracting 1 - 1 should succeed");

    u64 a4 = 1;
    u64 b4 = 2;
    ali_testing_expect(testing, !ali_sub_u64_checked(a4, b4, &res), "subtracting 1 - 2 should fail");
}

static ali_test_t internal_tests[] = {
    test_add_and_sub_u64_ckecked,
};

void ali_run_internal_tests() {
    AliTesting testing = {0};
    for (ali_usize i = 0; i < ALI_ARRAY_LEN(internal_tests); ++i) {
        ali_testing_run(&testing, internal_tests[i]);
    }
    ali_testing_print(&testing);
}
#endif // ALI_INTERNAL_TESTING
