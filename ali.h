/**
	ali.h - A single header file consisting of things the C std lib is missing.
	For now, it contains:
		- some util stuff (ali_util)
		- some simple types and aliases (ali_types)
		- logging (ali_log)
        - cli flags parsing (ali_flag)
		- arena (ali_arena)
		- dynamic array (ali_da)
		- slices (ali_slice)
		- temp allocator (ali_temp_alloc)
		- utf8 (ali_utf8)
		- string view (ali_sv)
		- string builder (ali_sb)
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
#include <stdbool.h>
#include <stdio.h>

#define ALI_VERSION "0.1.0"

// Customizable functions
#ifndef ALI_MALLOC
#include <stdlib.h>
#define ALI_MALLOC malloc
#endif // ALI_MALLOC

#ifndef ALI_REALLOC
#include <stdlib.h>
#define ALI_REALLOC realloc
#endif // ALI_REALLOC

#ifndef ALI_FREE
#include <stdlib.h>
#define ALI_FREE free
#endif // ALI_FREE

#ifndef ALI_ASSERT
#include <assert.h>
#define ALI_ASSERT assert
#endif // ALI_ASSERT

#ifndef ALI_MEMCPY
#include <string.h>
#define ALI_MEMCPY memcpy
#endif // ALI_MEMCPY

#ifndef ALI_ABORT
#include <stdlib.h>
#define ALI_ABORT abort
#endif // ALI_ABORT

// @module ali_util
#define ALI_ARRAY_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))
#define ALI_INLINE_ARRAY(Type, ...) ( (Type[]) { __VA_ARGS__ } )

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

// 'path/to/file.c' -> 'file.c', '/path/to/dir' -> 'dir'
const char* ali_path_name(const char* path);
char* ali_shift_args(int* argc, char*** argv);

// @module ali_util end

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

void ali_init_global_log();

ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_log_log(AliLogLevel level, const char* fmt, ...);

#define ali_log_info(...) ali_log_log(LOG_INFO, __VA_ARGS__)
#define ali_log_warn(...) ali_log_log(LOG_WARN, __VA_ARGS__)
#define ali_log_error(...) ali_log_log(LOG_ERROR, __VA_ARGS__)

#else // ALI_LOG_END
#define ali_log_info(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define ali_log_warn(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#define ali_log_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
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
    bool option;
}FlagAs;

typedef struct {
    const char* name;
    const char* description;

    FlagType type;
    FlagAs as;
}AliFlag;

const char** ali_flag_string(const char* name, const char* desc, const char* default_);
ali_u64* ali_flag_u64(const char* name, const char* desc, ali_u64 default_);
ali_f64* ali_flag_f64(const char* name, const char* desc, ali_f64 default_);
bool* ali_flag_option(const char* name, const char* desc, bool default_);

void ali_flag_print_help(FILE* sink);
ali_isize ali_flag_parse_with_program(int argc, char** argv);
ali_isize ali_flag_parse(int argc, char** argv);

// @module ali_flag end

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
	AliRegion *start, *end;
}AliArena;

typedef struct {
	AliRegion* r;
	ali_usize count;
}AliArenaMark;

AliRegion* ali_region_new(ali_usize capacity);
void* ali_region_alloc(AliRegion* self, ali_usize size, ali_usize alignment);

void* ali_arena_alloc_ex(AliArena* self, ali_usize size, ali_usize alignment);
#define ali_arena_alloc(self, size) ali_arena_alloc_ex(self, size, 8)
void* ali_arena_realloc(AliArena* arena, void* data, ali_usize oldsize, ali_usize newsize);
void* ali_arena_memdup(AliArena* self, const void* mem, ali_usize size_bytes);
char* ali_arena_strdup(AliArena* self, const char* cstr);

ALI_FORMAT_ATTRIBUTE(2, 3)
char* ali_arena_sprintf(AliArena* self, const char* fmt, ...);

AliArenaMark ali_arena_mark(AliArena* self);
void ali_arena_rollback(AliArena* self, AliArenaMark mark);
void ali_arena_reset(AliArena* self);
void ali_arena_free(AliArena* self);
// @module ali_arena end

// @module ali_da
#ifndef ALI_DA_DEFAULT_INIT_CAPACITY
#define ALI_DA_DEFAULT_INIT_CAPACITY 8
#endif // ALI_DA_DEFAULT_INIT_CAPACITY

typedef struct {
	ali_usize count;
	ali_usize capacity;
	ali_u8 data[];
}AliDaHeader;

AliDaHeader* ali_da_new_header_with_size(AliArena* arena, ali_usize init_capacity, ali_usize item_size);
AliDaHeader* ali_da_get_header_with_size(AliArena* arena, void* da, ali_usize item_size);
void* ali_da_free_with_size(void* da, ali_usize item_size);
void* ali_arena_da_maybe_resize_with_size(AliArena* arena, void* da, ali_usize to_add, ali_usize item_size);

#define ali_da_new_header(arena, init_capacity) ali_da_new_header_with_size(arena, init_capacity, sizeof(*(da)))
#define ali_da_get_header(arena, da) ali_da_get_header_with_size(arena, da, sizeof(*(da)))
#define ali_da_getlen(da) ((da) == NULL ? 0 : ali_da_get_header(NULL, da)->count)

#define ali_arena_da_maybe_resize(arena, da, to_add) ((da) = ali_arena_da_maybe_resize_with_size(arena, da, to_add, sizeof(*(da))))

#define ali_da_reset(arena, da) if ((da) != NULL) { ali_da_get_header(arena, da)->count = 0; }

#define ali_arena_da_append(arena, da, item) (ali_arena_da_maybe_resize(arena, da, 1), (da)[ali_da_get_header(arena, da)->count++] = item)
#define ali_arena_da_shallow_append(arena, da, item) (ali_arena_da_maybe_resize(arena, da, 1), (da)[ali_da_get_header(arena, da)->count] = item)
#define ali_arena_da_append_many(arena, da, items, item_count) do { \
    da = ali_arena_da_maybe_resize_with_size(arena, da, 1, sizeof(*(da))); \
    memcpy(da + da_getlen(da), items, (item_count) * sizeof(*(da))); \
    ali_da_get_header(da)->count += item_count; \
} while (0)

#define ali_da_append(da, item) ali_arena_da_append(NULL, da, item)
#define ali_da_shallow_append(da, item) ali_arena_da_shallow_append(NULL, da, item)
#define ali_da_append_many(da, items, item_count) ali_arena_da_append_many(NULL, da, items, item_count)

#define ali_da_free(da) ((da) = ali_da_free_with_size(da, sizeof(*(da))))
#define ali_da_remove_unordered(arena, da, i) (ALI_ASSERT(i >= 0), (da)[i] = (da)[--ali_da_get_header(arena, da)->count])
#define ali_da_remove_ordered(arena, da, i) (ALI_ASSERT(i >= 0), memmove(da + i, da + i + 1, (ali_da_get_header(arena, da)->count - i - 1) * sizeof(*(da))), ali_da_get_header(da)->count--)

#define ali_da_for(da, iter_name) for (ali_usize iter_name = 0; iter_name < ali_da_getlen(da); ++iter_name)
#define ali_da_foreach(da, Type, iter_name) for (Type* iter_name = da; iter_name < da + ali_da_getlen(da); ++iter_name)

// @module ali_da end

// @module ali_slice

typedef struct {
    ali_usize count;

    ali_usize item_size;
    ali_u8* data;
}AliSlice;

AliSlice ali_slice_from_parts(void* data, ali_usize count, ali_usize item_size);
AliSlice ali_da_to_slice_with_size(void* da, ali_usize item_size);
#define ali_da_to_slice(da) ali_da_to_slice_with_size(da, sizeof(*(da)))
AliSlice ali_da_slice_with_size(void* da, ali_usize start, ali_usize end_exclusive, ali_usize item_size);
#define ali_da_slice(da, start, end_exclusive) ali_da_slice_with_size(da, start, end_exclusive, sizeof(*(da)))
AliSlice ali_slice_cstr(char* str, ali_usize start, ali_usize end_exclusive);
AliSlice ali_slice_slice(AliSlice slice, ali_usize start, ali_usize end_exclusive);
void* ali_slice_get(AliSlice slice, ali_usize i);

#define ali_slice_for(slice, i) for (ali_usize i = 0; i < slice.count; ++i)
#define ali_slice_foreach(slice, Type, ptr) for (Type* ptr = slice.data; ptr < slice.data + (slice.count * slice.item_size); ++ptr)

// @module ali_slice end

// @module ali_temp_alloc
#ifndef ALI_TEMP_BUF_SIZE
#define ALI_TEMP_BUF_SIZE (8 << 20)
#endif // ALI_TEMP_BUF_SIZE

void* ali_temp_alloc(ali_usize size);
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
typedef ali_u32 ali_utf8codepoint;

#define ALI_UTF8(cstr) (ali_utf8*)(cstr)

ali_usize ali_utf8len(const ali_utf8* utf8);
ali_utf8codepoint ali_utf8c_to_codepoint(const ali_utf8* utf8c, ali_usize* codepoint_size);
ali_utf8codepoint* ali_utf8_to_codepoints(AliArena* arena, const ali_utf8* utf8, ali_usize* count);

bool ali_is_codepoint_valid(ali_utf8codepoint codepoint);
ali_usize ali_codepoint_size(ali_utf8codepoint codepoint);
const ali_utf8* ali_codepoint_to_utf8(ali_utf8codepoint codepoint);
ali_utf8* ali_codepoints_to_utf8(AliArena* arena, ali_utf8codepoint* codepoints, ali_usize len);

ali_utf8codepoint* ali_temp_utf8_to_codepoints(const ali_utf8* utf8, ali_usize* count);
ali_utf8* ali_temp_codepoints_to_utf8(ali_utf8codepoint* codepoints, ali_usize len);

#define ali_free_utf8 ALI_FREE
#define ali_free_codepoints ALI_FREE

// @module ali_utf8 end

// @module ali_sv
typedef struct {
	char* start;
	ali_usize len;
}AliSv;

#define ALI_SV_FMT "%.*s"
#define ALI_SV_F(sv) (int)(sv).len, (sv).start

#define ali_sv_from_parts(start_, len_) ((AliSv) { .start = start_, .len = len_ })
#define ali_sv_from_parts(start_, len_) ((AliSv) { .start = start_, .len = len_ })
#define ali_sv_from_cstr(cstr) ((AliSv) { .start = cstr, .len = strlen(cstr) })
#define ali_sv_from_cstr_static(cstr) ((AliSv) { .start = cstr, .len = sizeof(cstr) - 1 })

void ali_sv_step(AliSv* self);
AliSv ali_sv_trim_left(AliSv self);
AliSv ali_sv_trim_right(AliSv self);
AliSv ali_sv_trim(AliSv self);

AliSv ali_sv_chop_left(AliSv* self, ali_usize n);
AliSv ali_sv_chop_right(AliSv* self, ali_usize n);
AliSv ali_sv_chop_by_c(AliSv* self, char c);

bool ali_sv_chop_long(AliSv* self, int base, long* out);
bool ali_sv_chop_float(AliSv* self, float* out);
bool ali_sv_chop_double(AliSv* self, double* out);

bool ali_sv_eq(AliSv left, AliSv right);
bool ali_sv_starts_with(AliSv self, AliSv prefix);
bool ali_sv_ends_with(AliSv self, AliSv suffix);

char* ali_temp_sv_to_cstr(AliSv sv);

// @module ali_sv end

// @module ali_sb
typedef struct {
	char* data;
	ali_usize count;
	ali_usize capacity;
}AliSb;

void ali_sb_maybe_resize(AliSb* self, ali_usize to_add);
void ali_sb_push_strs_null(AliSb* self, ...);
ALI_FORMAT_ATTRIBUTE(2, 3)
void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...);
void ali_sb_free(AliSb* self);

bool ali_sb_read_file(AliSb* self, const char* path);
bool ali_sb_write_file(AliSb* self, const char* path);

#define ali_sb_push_strs(...) ali_sb_push_strs_null(__VA_ARGS__, NULL)

#define ali_sb_to_sv(sb) ali_sv_from_parts((sb).items, (sb).count)

// @module ali_sb end

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

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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
bool ali_wait_for_process(pid_t pid);
bool ali_cmd_run_sync(char** cmd);
bool ali_cmd_run_sync_and_reset(char** cmd);

bool ali_needs_rebuild(const char* output, const char** inputs, ali_usize input_count);
bool ali_needs_rebuild1(const char* output, const char* input);

bool ali_dup2_logged(int fd1, int fd2);
bool ali_rename(char*** cmd, const char* from, const char* to);
bool ali_remove(char*** cmd, const char* path);

bool ali_create_dir_if_not_exists(const char* path);

#define ali_rebuild_yourself(cmd, argc, argv) do { \
    const char* src = __FILE__; \
    const char* dst = argv[0]; \
    if (ali_needs_rebuild1(dst, src)) { \
        const char* old_dst = temp_sprintf("%s.prev", dst); \
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

typedef enum {
    C_EXE,
    C_OBJ,
    C_DYNLIB,
    // TODO: implement
    // C_STATICLIB,
}AliCBuilderType;

typedef struct AliCBuilder {
    AliCBuilderType type;
    AliArena* arena;
    struct AliCBuilder* subbuilders;

    char* cc;
    char* target;
    char** srcs;
    char** cflags;
    char** libs;
}AliCBuilder;

void ali_c_builder_add_srcs_(AliCBuilder* builder, ...);
#define ali_c_builder_add_srcs(...) ali_c_builder_add_srcs_(__VA_ARGS__, NULL)
void ali_c_builder_add_libs_(AliCBuilder* builder, ...);
#define ali_c_builder_add_libs(...) ali_c_builder_add_libs_(__VA_ARGS__, NULL)
void ali_c_builder_add_flags_(AliCBuilder* builder, ...);
#define ali_c_builder_add_flags(...) ali_c_builder_add_flags_(__VA_ARGS__, NULL)
void ali_c_builder_reset(AliCBuilder* builder, AliCBuilderType type, AliArena* arena, char* target, char* src);
bool ali_c_builder_execute(AliCBuilder* builder, char*** cmd, bool force);
AliCBuilder* ali_c_builder_next_subbuilder(AliCBuilder* builder);

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

// @module ali_log
#ifndef ALI_NO_LOG

static_assert(LOG_COUNT_ == 3, "Log level was added");
const char* loglevel_to_str[LOG_COUNT_] = {
	[LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",
	[LOG_ERROR] = "ERROR",
};

FILE* ali_global_logfile = NULL;
AliLogLevel ali_global_loglevel = LOG_INFO;

void ali_log_log(AliLogLevel level, const char* fmt, ...) {
	if (ali_global_logfile == NULL) ali_global_logfile = stdout;

	va_list args;
	va_start(args, fmt);

	if (ali_global_loglevel <= level) {
		fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
		vfprintf(ali_global_logfile, fmt, args);
		fprintf(ali_global_logfile, "\n");
	}

	va_end(args);
}
#endif // ALI_LOG_END
// @module ali_log end

// @module ali_flag
#define ALI_FLAG_LIST_MAX_SIZE 128
AliFlag ali_flag_list[ALI_FLAG_LIST_MAX_SIZE] = {0};
ali_usize ali_flag_list_size = 0;

AliFlag* ali_flag_push(AliFlag flag) {
     AliFlag* flag_ = &ali_flag_list[ali_flag_list_size++];
     *flag_ = flag;
     return flag_;
}

const char** ali_flag_string(const char* name, const char* desc, const char* default_) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .type = FLAG_STRING,
        .as.string = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.string;
}

ali_u64* ali_flag_u64(const char* name, const char* desc, ali_u64 default_) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .type = FLAG_U64,
        .as.num_u64 = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.num_u64;
}

ali_f64* ali_flag_f64(const char* name, const char* desc, ali_f64 default_) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .type = FLAG_F64,
        .as.num_f64 = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.num_f64;
}

bool* ali_flag_option(const char* name, const char* desc, bool default_) {
    AliFlag flag = {
        .name = name,
        .description = desc,
        .type = FLAG_OPTION,
        .as.option = default_,
    };

    AliFlag* pushed_flag = ali_flag_push(flag);
    return &pushed_flag->as.option;
}

void ali_flag_print_help(FILE* sink) {
    for (ali_usize i = 0; i < ali_flag_list_size; ++i) {
        AliFlag* flag = &ali_flag_list[i];
        fprintf(sink, "%s:\n", flag->name);
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

ali_isize ali_flag_parse_with_program(int argc, char** argv) {
    char* program = ali_shift_args(&argc, &argv);
    ALI_UNUSED(program);
    return ali_flag_parse(argc, argv);
}

ali_isize ali_flag_parse(int argc, char** argv) {
while_loop: while (argc > 0) {
        char* arg = ali_shift_args(&argc, &argv);

        if (*arg == '-') {
            for (ali_usize j = 0; j < ali_flag_list_size; ++j) {
                if (strcmp(arg, ali_flag_list[j].name) == 0) {
                    switch (ali_flag_list[j].type) {
                        case FLAG_STRING:
                            if (argc == 0) {
                                ali_log_error("%s requires an arguement", ali_flag_list[j].name);
                                return -1;
                            }
                            ali_flag_list[j].as.string = ali_shift_args(&argc, &argv);
                            break;
                        case FLAG_U64:
                            if (argc == 0) {
                                ali_log_error("%s requires an arguement", ali_flag_list[j].name);
                                return -1;
                            }
                            ali_flag_list[j].as.num_u64 = atol(ali_shift_args(&argc, &argv));
                            break;
                        case FLAG_F64:
                            if (argc == 0) {
                                ali_log_error("%s requires an arguement", ali_flag_list[j].name);
                                return -1;
                            }
                            char* endptr;
                            ali_flag_list[j].as.num_f64 = strtod(ali_shift_args(&argc, &argv), &endptr);
                            break;
                        case FLAG_OPTION:
                            ali_flag_list[j].as.option = true;
                            break;
                    }
                    goto while_loop;
                }
            }

            if (strcmp(arg, "-h") == 0) {
                ali_flag_print_help(stdout);
                exit(0);
            }
        } else {
            return (ali_isize)argc;
        }
    }

    return (ali_isize)argc;
}

// @module ali_flag end

// @module ali_arena

AliRegion* ali_region_new(ali_usize capacity) {
	AliRegion* new = ALI_MALLOC(sizeof(*new) + capacity);
	ALI_ASSERT(new != NULL);
	new->count = 0;
	new->capacity = capacity;
	new->next = NULL;
	return new;
}

void* ali_region_alloc(AliRegion* self, ali_usize size, ali_usize alignment) {
	if (self->count + size >= self->capacity) return NULL;
	self->count += self->count % alignment;
	void* ptr = self->data + self->count;
	self->count += size;
	return ptr;
}

void* ali_arena_alloc_ex(AliArena* self, ali_usize size, ali_usize alignment) {
    if (self == NULL) return ALI_MALLOC(size);

	if (self->start == NULL) {
		self->start = ali_region_new(ALI_REGION_DEFAULT_CAP);
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

void* ali_arena_realloc(AliArena* arena, void* data, ali_usize oldsize, ali_usize newsize) {
    void* copy = ali_arena_alloc(arena, newsize);
    ALI_MEMCPY(copy, data, oldsize);
    if (arena == NULL) ALI_FREE(data);
    return copy;
}

void* ali_arena_memdup(AliArena* self, const void* mem, ali_usize size_bytes) {
	void* data = ali_arena_alloc(self, size_bytes);
	ALI_MEMCPY(data, mem, size_bytes);
	return data;
}

char* ali_arena_strdup(AliArena* self, const char* cstr) {
	return ali_arena_memdup(self, cstr, strlen(cstr) + 1);
}

char* ali_arena_sprintf(AliArena* self, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* out;
	ALI_ASSERT(vasprintf(&out, fmt, args) >= 0);
	char* real_out = ali_arena_strdup(self, out);
	ALI_FREE(out);

	va_end(args);
	return real_out;
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
		ALI_FREE(self->start);
		self->start = next;
	}
	self->end = NULL;
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

// @module ali_da
AliDaHeader* ali_da_new_header_with_size(AliArena* arena, ali_usize init_capacity, ali_usize item_size) {
	AliDaHeader* h = ali_arena_alloc(arena, sizeof(*h) + item_size * init_capacity);
	h->count = 0;
	h->capacity = init_capacity;
	return h;
}

AliDaHeader* ali_da_get_header_with_size(AliArena* arena, void* da, ali_usize item_size) {
	if (da == NULL) return ali_da_new_header_with_size(arena, ALI_DA_DEFAULT_INIT_CAPACITY, item_size);
	
	return (AliDaHeader*)da - 1;
}

void* ali_arena_da_maybe_resize_with_size(AliArena* arena, void* da, ali_usize to_add, ali_usize item_size) {
    AliDaHeader* h = ali_da_get_header_with_size(arena, da, item_size);

    if (h->count + to_add >= h->capacity) {
        ali_usize old_capacity = h->capacity;
        while (h->count + to_add >= h->capacity) {
            if (h->capacity == 0) h->capacity = 8;
            else h->capacity *= 8;
        }
        h = ali_arena_realloc(arena, h, sizeof(*h) + old_capacity * item_size, sizeof(*h) + h->capacity * item_size);
    }
    return h->data;
}

void* ali_da_free_with_size(void* da, ali_usize item_size) {
	AliDaHeader* h = ali_da_get_header_with_size(NULL, da, item_size);
	ALI_FREE(h);
	return (da = NULL);
}

// @module ali_da end

// @module ali_slice

AliSlice ali_slice_from_parts(void* data, ali_usize count, ali_usize item_size) {
    AliSlice slice = {
        .item_size = item_size,
        .count = count,
        .data = data,
    };
    return slice;
}

AliSlice ali_da_to_slice_with_size(void* da, ali_usize item_size) {
    AliSlice slice = {
        .count = ali_da_getlen(da),
        .item_size = item_size,
        .data = da,
    };
    return slice;
}

AliSlice ali_da_slice_with_size(void* da, ali_usize start, ali_usize end_exclusive, ali_usize item_size) {
    ALI_ASSERT(start < ali_da_getlen(da));
    ALI_ASSERT(end_exclusive <= ali_da_getlen(da));

    AliSlice slice = {
        .count = end_exclusive - start,
        .item_size = item_size,
        .data = (ali_u8*)(da) + start * item_size
    };
    return slice;
}

AliSlice ali_slice_cstr(char* str, ali_usize start, ali_usize end_exclusive) {
    ALI_ASSERT(start < strlen(str));
    ALI_ASSERT(end_exclusive <= strlen(str));

    AliSlice slice = {
        .item_size = 1,
        .count = end_exclusive - start,
        .data = (ali_u8*)str + start,
    };
    return slice;
}

AliSlice ali_slice_slice(AliSlice slice, ali_usize start, ali_usize end_exclusive) {
    ALI_ASSERT(start < slice.count);
    ALI_ASSERT(end_exclusive <= slice.count);

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
	ALI_ASSERT(ali_temp_buffer_size + size < ALI_TEMP_BUF_SIZE);
	void* ptr = ali_temp_buffer + ali_temp_buffer_size;
	ali_temp_buffer_size += size;
	return ptr;
}

void* ali_temp_memdup(void* data, ali_usize data_size) {
	void* mem = ali_temp_alloc(data_size);
	ALI_MEMCPY(mem, data, data_size);
	return mem;
}

char* ali_temp_strdup(const char* str) {
	ali_usize size = strlen(str) + 1;
	char* copy = ali_temp_alloc(size);
	return ALI_MEMCPY(copy, str, size);
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
    ALI_ASSERT(ali_temp_buffer_size < ALI_TEMP_BUF_SIZE);
    ali_temp_buffer[ali_temp_buffer_size++] = c;
}

void ali_temp_push_str(const char* str) {
    ali_usize len = strlen(str);
    ALI_ASSERT(ali_temp_buffer_size + len < ALI_TEMP_BUF_SIZE);
    ALI_MEMCPY(ali_temp_buffer + ali_temp_buffer_size, str, len);
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
		ali_usize codepoint_size = 0;
		ali_utf8c_to_codepoint(utf8, &codepoint_size);
		len++;
		utf8 += codepoint_size;
	}
	return len;
}

bool ali_codepoint_is_valid(ali_utf8codepoint codepoint) {
	if (codepoint > 0x10FFFF) return false;
	return true;
}

ali_utf8codepoint ali_utf8c_to_codepoint(const ali_utf8* utf8c, ali_usize* codepoint_size) {
	ali_utf8codepoint codepoint = 0;
	ali_usize codepoint_size_ = 0;

	if ((utf8c[0] & 0x80) == 0x00) {
		codepoint = utf8c[0];
		codepoint_size_ = 1;
	} else if ((utf8c[0] & 0xE0) == 0xC0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*1) | ((utf8c[1] & 0x3F) << 6*0);
		codepoint_size_ = 2;
	} else if ((utf8c[0] & 0xF0) == 0xE0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*2) | ((utf8c[1] & 0x3F) << 6*1) | ((utf8c[2] & 0x3F) << 6*0);
		codepoint_size_ = 3;
	} else if ((utf8c[0] & 0xF8) == 0xF0) {
		codepoint = ((utf8c[0] & 0x1F) << 6*3) | ((utf8c[1] & 0x3F) << 6*2) | ((utf8c[2] & 0x3F) << 6*1) | ((utf8c[3] & 0x3F) << 6*0);
		codepoint_size_ = 4;
	} else {
		// invalid
		return -1;
	}

	if (codepoint_size) *codepoint_size = codepoint_size_;
	return codepoint;
}

ali_utf8codepoint* ali_utf8_to_codepoints(AliArena* arena, const ali_utf8* utf8, ali_usize* count) {
	ali_usize len = ali_utf8len(utf8);
	*count = len;

	ali_utf8codepoint* codepoints = ali_arena_alloc(arena, len * sizeof(*codepoints));
	len = 0;
	while (*utf8 != 0) {
		ali_usize codepoint_size = 0;
		codepoints[len++] = ali_utf8c_to_codepoint(utf8, &codepoint_size);
		utf8 += codepoint_size;
	}

	return codepoints;
}

ali_usize ali_codepoint_size(ali_utf8codepoint codepoint) {
	if (codepoint > 0x10FFFF) return 0;
	else if (codepoint > 0xFFFF) return 4;
	else if (codepoint > 0x07FF) return 3;
	else if (codepoint > 0x007F) return 2;
	else return 1;
}

const ali_utf8* ali_codepoint_to_utf8(ali_utf8codepoint codepoint) {
static ali_utf8 utf8[5] = {0};
	
	ali_usize codepoint_size = ali_codepoint_size(codepoint);
	if (codepoint_size == 4) {
		utf8[0] = 0xF0 | ((codepoint >> 6*3) & 0x07);
		utf8[1] = 0x80 | ((codepoint >> 6*2) & 0x3F);
		utf8[2] = 0x80 | ((codepoint >> 6*1) & 0x3F);
		utf8[3] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 3) {
		utf8[0] = 0xE0 | ((codepoint >> 6*2) & 0x0F);
		utf8[1] = 0x80 | ((codepoint >> 6*1) & 0x3F);
		utf8[2] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 2) {
		utf8[0] = 0xC0 | ((codepoint >> 6*1) & 0x1F);
		utf8[1] = 0x80 | ((codepoint >> 6*0) & 0x3F);
	} else if (codepoint_size == 1) {
		utf8[0] = codepoint;
	} else {
		// invalid
		return NULL;
	}
	utf8[codepoint_size] = 0;

	return utf8;
}

ali_utf8* ali_codepoints_to_utf8(AliArena* arena, ali_utf8codepoint* codepoints, ali_usize len) {
	ali_usize real_len = 0;
	for (ali_usize i = 0; i < len; ++i) {
		real_len += ali_codepoint_size(codepoints[i]);
	}

	ali_utf8* utf8 = ali_arena_alloc(arena, real_len + 1);
	ali_utf8* utf8p = utf8;
	for (ali_usize i = 0; i < len; ++i) {
		ali_usize codepoint_size = ali_codepoint_size(codepoints[i]);
		memcpy(utf8p, ali_codepoint_to_utf8(codepoints[i]), codepoint_size);
		utf8p += codepoint_size;
	}
	utf8[real_len] = 0;

	return utf8;
}

ali_utf8codepoint* ali_temp_utf8_to_codepoints(const ali_utf8* utf8, ali_usize* count) {
	ali_utf8codepoint* codepoints = ali_utf8_to_codepoints(NULL, utf8, count);
	ali_utf8codepoint* out = ali_temp_alloc(sizeof(*out) * (*count));
	ALI_MEMCPY(out, codepoints, sizeof(*out) * (*count));
	ali_free_codepoints(codepoints);
	return out;
}

ali_utf8* ali_temp_codepoints_to_utf8(ali_utf8codepoint* codepoints, ali_usize len) {
	ali_utf8* utf8 = ali_codepoints_to_utf8(NULL, codepoints, len);
	ali_utf8* out = (ali_utf8*)ali_temp_strdup((char*)utf8);
	ali_free_utf8(utf8);
	return out;
}

// @module ali_utf8 end

// @module ali_sv

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
	ali_sv_step(self);

	chopped.len = self->start - chopped.start;
	return chopped;
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

bool ali_sv_chop_long(AliSv* self, int base, long* out) {
	char* endptr;
	long num = strtol(self->start, &endptr, base);
	if (endptr == self->start) return false;
	*out = num;

	ali_usize n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_float(AliSv* self, float* out) {
	char* endptr;
	float num = strtof(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	ali_usize n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_double(AliSv* self, double* out) {
	char* endptr;
	double num = strtod(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	ali_usize n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_eq(AliSv left, AliSv right) {
	bool eq = left.len == right.len;
	for (ali_usize i = 0; eq && i < left.len; ++i) {
		eq &= left.start[i] == right.start[i];
	}
	return eq;
}

bool ali_sv_starts_with(AliSv self, AliSv prefix) {
	if (self.len < prefix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start, prefix.len), prefix);
}

bool ali_sv_ends_with(AliSv self, AliSv suffix) {
	if (self.len < suffix.len) return false;
	return ali_sv_eq(ali_sv_from_parts(self.start + self.len - suffix.len, suffix.len), suffix);
}

char* ali_temp_sv_to_cstr(AliSv sv) {
	char* out = ali_temp_alloc(sv.len + 1);
	ALI_MEMCPY(out, sv.start, sv.len);
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

		self->data = ALI_REALLOC(self->data, self->capacity);
	}
}

void ali_sb_push_strs_null(AliSb* self, ...) {
	va_list args;
	va_start(args, self);

	const char* str = va_arg(args, const char*);
	while (str != NULL) {
		ali_usize n = strlen(str);
		ali_sb_maybe_resize(self, n);
		ALI_MEMCPY(self->data + self->count, str, n);
		self->count += n;
		str = va_arg(args, const char*);
	}

	va_end(args);
}

void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char* str;
	ALI_ASSERT(vasprintf(&str, fmt, args) == 0);
	ali_sb_push_strs(self, str);
	ALI_FREE(str);

	va_end(args);
}

void ali_sb_free(AliSb* self) {
	ALI_FREE(self->data);
	self->data = NULL;
	self->count = 0;
	self->capacity = 0;
}

bool ali_sb_read_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "rb");
	if (f == NULL) {
		ali_log_error("Couldn't read %s: %s\n", path, strerror(errno));
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

bool ali_sb_write_file(AliSb* self, const char* path) {
	FILE* f = fopen(path, "wb");
	if (f == NULL) {
		ali_log_error("Couldn't write to %s: %s\n", path, strerror(errno));
		return false;
	}

	fwrite(self->data, 1, self->count, f);

	if (f != NULL) fclose(f);
	return true;
}

// @module ali_sb end

// @module ali_measure

double ali_get_now() {
	struct timespec ts;
	ALI_ASSERT(clock_gettime(CLOCK_REALTIME, &ts) == 0);
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
	ALI_ASSERT(measurements_count < ALI_MEASUREMENTS_COUNT);
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
	ALI_ASSERT(found != NULL);

	found->total += ali_get_now() - found->start;
	found->count += 1;
}

void ali_print_measurements(void) {
	for (ali_usize i = 0; i < measurements_count; ++i) {
		ali_log_info("[ali_measure] %s: %lfs", measurements[i].name, measurements[i].total / measurements[i].count);
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
        if (!strchr(arg, ' ')) {
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
    ali_log_info("[CMD] %s", render);
    ali_temp_rewind(stamp);

    bool should_redirect_stdin = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDIN) != 0;
    bool should_redirect_stdout = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDOUT) != 0;
    bool should_redirect_stderr = redirect == NULL ? false : (redirect->redirect_bitmask & ALI_REDIRECT_STDERR) != 0;

    if (should_redirect_stdout || should_redirect_stderr) {
        if (pipe(redirect->out_pipe) < 0) {
            ali_log_error("Couldn't create pipe: %s\n", strerror(errno));
            return -1;
        }
    }

    if (should_redirect_stdin) {
        if (pipe(redirect->in_pipe) < 0) {
            ali_log_error("Couldn't create pipe: %s\n", strerror(errno));
            return -1;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        ali_log_error("Couldn't fork: %s\n", strerror(errno));
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

        ali_log_error("Couldn't start process: %s\n", strerror(errno));
        exit(1);
    }
    
    return pid;
}

bool ali_wait_for_process(pid_t pid) {
    for (;;) {
        int wstatus;
        if (waitpid(pid, &wstatus, 0) < 0) {
            ali_log_error("Couldn't waitpid for %s", strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int estatus = WEXITSTATUS(wstatus);
            if (estatus != 0) {
                ali_log_error("Process %d exited with status %d", pid, estatus);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            int sig = WTERMSIG(wstatus);
            ali_log_error("Process %d exited with signal %d (%s)", pid, sig, strsignal(sig));
            return false;
        }
    }

    return true;
}

bool ali_cmd_run_sync(char** cmd) {
    pid_t pid = ali_cmd_run_async(cmd);
    return ali_wait_for_process(pid);
}

bool ali_cmd_run_sync_and_reset(char** cmd) {
    bool ret = ali_cmd_run_sync(cmd);
    ali_da_get_header(NULL, cmd)->count = 0;
    return ret;
}

bool ali_needs_rebuild(const char* output, const char** inputs, ali_usize input_count) {
    struct stat st;

    if (stat(output, &st) < 0) {
        if (errno == ENOENT) return true;
        ali_log_error("Couldn't stat %s: %s", output, strerror(errno));
        return false;
    }

    struct timespec ts_output = st.st_mtim;

    for (ali_usize i = 0; i < input_count; ++i) {
        if (stat(inputs[i], &st) < 0) {
            ali_log_error("Couldn't stat %s: %s", inputs[i], strerror(errno));
            return false;
        }

        if (st.st_mtim.tv_sec > ts_output.tv_sec) return 1;
    }

    return false;
}

bool ali_needs_rebuild1(const char* output, const char* input) {
    return ali_needs_rebuild(output, &input, 1);
}

void ali_c_builder_add_srcs_(AliCBuilder* builder, ...) {
    va_list args;
    va_start(args, builder);

    char* src;
    do {
        src = va_arg(args, char*);
        if (src != NULL) {
            ali_arena_da_append(builder->arena, builder->srcs, src);
        }
    } while (src != NULL);

    va_end(args);
}

void ali_c_builder_add_libs_(AliCBuilder* builder, ...) {
    va_list args;
    va_start(args, builder);

    char* lib;
    do {
        lib = va_arg(args, char*);
        if (lib != NULL) {
            ali_arena_da_append(builder->arena, builder->libs, lib);
        }
    } while (lib != NULL);

    va_end(args);
}

void ali_c_builder_add_flags_(AliCBuilder* builder, ...) {
    va_list args;
    va_start(args, builder);

    char* flag;
    do {
        flag = va_arg(args, char*);
        if (flag != NULL) {
            ali_arena_da_append(builder->arena, builder->cflags, flag);
        }
    } while (flag != NULL);

    va_end(args);
}

void ali_c_builder_reset(AliCBuilder* builder, AliCBuilderType type, AliArena* arena, char* target, char* src) {
    ali_da_reset(builder->arena, builder->srcs);
    ali_da_reset(builder->arena, builder->cflags);
    ali_da_reset(builder->arena, builder->libs);

#if __GNUC__
    builder->cc = "gcc";
#elif defined(__clang__)
    builder->cc = "clang";
#else
#error "Unsupported compiler"
#endif

    builder->arena = arena;
    builder->type = type;
    builder->target = target;
    ali_arena_da_append(builder->arena, builder->srcs, src);
}

bool ali_c_builder_execute(AliCBuilder* builder, char*** cmd, bool force) {
    ali_da_foreach(builder->subbuilders, AliCBuilder, subbuilder) {
        if (!ali_c_builder_execute(subbuilder, cmd, force)) return false;
        switch (subbuilder->type) {
            case C_EXE:
                break;
            case C_OBJ:
                ali_c_builder_add_srcs(builder, subbuilder->target);
                break;
            case C_DYNLIB:
                ali_c_builder_add_libs(builder, ali_temp_sprintf("-l:%s", subbuilder->target));
                break;
            default:
                ALI_UNREACHABLE();
        }
    }

    if (force || ali_needs_rebuild(builder->target, (const char**)builder->srcs, ali_da_getlen(builder->srcs))) {
        ali_log_info("Building %s", builder->target);

        ali_cmd_append_args(cmd, builder->cc);
        if (ali_da_getlen(builder->srcs) == 0) {
            ali_log_error("No source files");
            return false;
        }
        ali_da_foreach(builder->cflags, char*, flag) {
            ali_cmd_append_arg(*cmd, *flag);
        }
        switch (builder->type) {
            case C_EXE:
                break;
            case C_OBJ:
                ali_cmd_append_args(cmd, "-c");
                break;
            case C_DYNLIB:
                ali_cmd_append_args(cmd, "-fPIC", "-shared");
                break;
            default:
                ALI_UNREACHABLE();
        }
        ali_cmd_append_args(cmd, "-o", builder->target);
        ali_da_foreach(builder->srcs, char*, src) {
            ali_cmd_append_arg(*cmd, *src);
        }
        ali_da_foreach(builder->libs, char*, lib) {
            ali_cmd_append_arg(*cmd, *lib);
        }
        bool ret = ali_cmd_run_sync_and_reset(*cmd);
        if (ret) {
            ali_log_info("Built %s", builder->target);
        } else {
            ali_log_error("Failed to built %s", builder->target);
        }
        return ret;
    } else {
        ali_log_warn("No need to build %s", builder->target);
        return true;
    }
}

AliCBuilder* ali_c_builder_next_subbuilder(AliCBuilder* builder) {
    ali_usize count = ali_da_get_header(builder->arena, builder->subbuilders)->count;
    ali_arena_da_maybe_resize(builder->arena, builder->subbuilders, 1);
    ali_da_get_header(builder->arena, builder->subbuilders)->count++;
    return &builder->subbuilders[count];
}

bool ali_rename(char*** cmd, const char* from, const char* to) {
    ali_da_get_header(NULL, *cmd)->count = 0;
    ali_cmd_append_args(cmd, "mv", from, to);
    return ali_cmd_run_sync_and_reset(*cmd);
}

bool ali_remove(char*** cmd, const char* path) {
    ali_da_get_header(NULL, *cmd)->count = 0;
    ali_cmd_append_args(cmd, "rm", path);
    return ali_cmd_run_sync_and_reset(*cmd);
}

bool ali_dup2_logged(int fd1, int fd2) {
    if (dup2(fd1, fd2) < 0) {
        ali_log_error("Couldn't dup2: %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool ali_create_dir_if_not_exists(const char* path) {
    if (mkdir(path, 0766) < 0) {
        if (errno != EEXIST) {
            ali_log_error("Couldn't create %s: %s\n", path, strerror(errno));
            return false;
        }
    }

    return true;
}

// @module ali_cmd end

#endif // ALI_IMPLEMENTATION

#ifdef ALI_REMOVE_PREFIX
#undef ALI_REMOVE_PREFIX

// @module ali_util
#define ARRAY_LEN ALI_ARRAY_LEN
#define INLINE_ARRAY ALI_INLINE_ARRAY

#define UNUSED ALI_UNUSED
#define UNREACHABLE ALI_UNREACHABLE
#define TODO ALI_TODO
#define PANIC ALI_PANIC

#define RETURN_DEFER ALI_RETURN_DEFER

#define FORMAT_ATTRIBUTE ALI_FORMAT_ATTRIBUTE

#define SWAP ALI_SWAP

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

#define log_log ali_log_log

#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error

// @module ali_log end

// @module ali_flag

#define flag_string ali_flag_string
#define flag_u64 ali_flag_u64
#define flag_f64 ali_flag_f64
#define flag_option ali_flag_option

#define flag_print_help ali_flag_print_help
#define flag_parse ali_flag_parse
#define flag_parse_with_program ali_flag_parse

// @module ali_flag end

// @module ali_da
#define da_new_header_with_size ali_da_new_header_with_size
#define da_get_header_with_size ali_da_get_header_with_size
#define da_maybe_resize_with_size ali_da_maybe_resize_with_size
#define da_free_with_size ali_da_free_with_size

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
#define temp_memdup ali_temp_memdup
#define temp_sprintf ali_temp_sprintf
#define temp_strdup ali_temp_strdup
#define temp_stamp ali_temp_stamp
#define temp_rewind ali_temp_rewind
#define temp_reset ali_temp_reset

// @module ali_temp_alloc end

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

// @module ali_utf8
// NOTE: we mustn't do this
// #define utf8 ali_utf8
#define utf8codepoint ali_utf8codepoint

#define UTF8 ALI_UTF8

#define utf8len ali_utf8len
#define utf8c_to_codepoint ali_utf8c_to_codepoint
#define utf8_to_codepoints ali_utf8_to_codepoints

#define is_codepoint_valid ali_is_codepoint_valid
#define codepoint_size ali_codepoint_size
#define codepoint_to_utf8 ali_codepoint_to_utf8
#define codepoints_to_utf8 ali_codepoints_to_utf8

#define temp_utf8_to_codepoints ali_temp_utf8_to_codepoints
#define temp_codepoints_to_utf8 ali_temp_codepoints_to_utf8
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

#define sv_chop_long ali_sv_chop_long
#define sv_chop_float ali_sv_chop_float
#define sv_chop_double ali_sv_chop_double

#define sv_eq ali_sv_eq
#define sv_starts_with ali_sv_starts_with
#define sv_ends_with ali_sv_ends_with

#define temp_sv_to_cstr ali_temp_sv_to_cstr
// @module ali_sv end

// @module ali_sb
#define sb_maybe_resize ali_sb_maybe_resize
#define sb_push_strs_null ali_sb_push_strs_null
#define sb_push_sprintf ali_sb_push_sprintf
#define sb_free ali_sb_free

#define sb_push_strs ali_sb_push_strs
#define sb_to_sv ali_sb_to_sv

#define sb_read_file ali_sb_read_file
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
#define rebuild_yourself ali_rebuild_yourself

#define c_builder_add_srcs ali_c_builder_add_srcs
#define c_builder_add_libs ali_c_builder_add_libs
#define c_builder_add_flags ali_c_builder_add_flags
#define c_builder_reset ali_c_builder_reset
#define c_builder_execute ali_c_builder_execute
#define c_builder_next_subbuilder ali_c_builder_next_subbuilder

// @module ali_cmd end

#endif // ALI_REMOVE_PREFIX

