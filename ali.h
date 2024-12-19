/**
	ali.h - A single header file consisting of things the C std lib is missing.
	For now, it contains:
		- some util macros (ali_util)
		- some simple types (ali_types)
		- logging (ali_log)
		- dynamic array (ali_da)
		- arena (ali_arena)
		- string view (ali_sv)
		- string builder (ali_sb)

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
#include <stdint.h>
#include <stdbool.h>

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

// ali_util
#define ALI_ARRAY_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))

#define ALI_UNUSED(thing) (void)(thing)
#define ALI_UNREACABLE() do { fprintf(stderr, "%s:%d: UNREACABLE\n", __FILE__, __LINE__); ALI_ABORT(); } while (0)
#define ALI_TODO() do { fprintf(stderr, "%s:%d: TODO: %s not implemented\n", __FILE__, __LINE__, __func__); ALI_ABORT(); } while (0)
#define ALI_PANIC(...) do { fprintf(stderr, __VA_ARGS__); ALI_ABORT(); } while (0)

#define ALI_RETURN_DEFER(value) do { result = value; goto defer } while (0)
// ali_util end

// ali_types
typedef uint8_t  ali_u8;
typedef uint16_t ali_u16;
typedef uint32_t ali_u32;
typedef uint64_t ali_u64;

typedef int8_t  ali_i8;
typedef int16_t ali_i16;
typedef int32_t ali_i32;
typedef int64_t ali_i64;
// ali_types end

// ali_log
typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,

	LOG_COUNT_
}AliLogLevel;

extern FILE* ali_global_logfile;
extern AliLogLevel ali_global_loglevel;

void ali_init_global_log();
void ali_log_log(AliLogLevel level, const char* fmt, ...);

#define ali_log_info(...) ali_log_log(LOG_INFO, __VA_ARGS__)
#define ali_log_warn(...) ali_log_log(LOG_WARN, __VA_ARGS__)
#define ali_log_error(...) ali_log_log(LOG_ERROR, __VA_ARGS__)

// ali_log end

// ali_da
#ifndef ALI_DA_DEFAULT_INIT_CAPACITY
#define ALI_DA_DEFAULT_INIT_CAPACITY 8
#endif // ALI_DA_DEFAULT_INIT_CAPACITY

typedef struct {
	size_t count;
	size_t capacity;
	ali_u8 data[];
}AliDaHeader;

AliDaHeader* ali_da_new_header_with_size(size_t init_capacity, size_t item_size);
AliDaHeader* ali_da_get_header_with_size(void* da, size_t item_size);
void* ali_da_maybe_resize_with_size(void* da, size_t to_add, size_t item_size);
void* ali_da_free_with_size(void* da, size_t item_size);

#define ali_da_new_header(init_capacity) ali_da_new_header_with_size(da, init_capacity, sizeof(*(da)))
#define ali_da_get_header(da) ali_da_get_header_with_size(da, sizeof(*(da)))
#define ali_da_maybe_resize(da, to_add) ali_da_maybe_resize_with_size(da, to_add, sizeof(*(da)))
#define ali_da_getlen(da) (ALI_ASSERT((da) != NULL), ali_da_get_header(da)->count)

#define ali_da_append(da, item) ((da) = ali_da_maybe_resize(da, 1), (da)[ali_da_get_header_with_size(da, sizeof(*(da)))->count++] = item)

#define ali_da_free(da) ((da) = ali_da_free_with_size(da, sizeof(*(da)))
#define ali_da_remove_unordered(da, i) (ALI_ASSERT(i >= 0), (da)[i] = (da)[--ali_da_get_header(da)->count])
#define ali_da_remove_ordered(da, i) (ALI_ASSERT(i >= 0), memmove(da + i, da + i + 1, (ali_da_get_header(da)->count - i - 1) * sizeof(*(da))), ali_da_get_header(da)->count--)

#define ali_da_for(da, iter_name) for (size_t iter_name = 0; iter_name < ali_da_getlen(da); ++iter_name)
#define ali_da_foreach(da, Type, iter_name) for (Type* iter_name = da; iter_name < da + ali_da_getlen(da); ++iter_name)

// ali_da end

// ali_arena 
#ifndef ALI_REGION_DEFAULT_CAP
#define ALI_REGION_DEFAULT_CAP (4 << 10)
#endif // ALI_REGION_DEFAULT_CAP

typedef struct AliRegion {
	size_t count;
	size_t capacity;
	struct AliRegion* next;

	ali_u8 data[];
}AliRegion;

typedef struct {
	AliRegion *start, *end;
}AliArena;

typedef struct {
	AliRegion* r;
	size_t count;
}AliArenaMark;

AliRegion* ali_region_new(size_t capacity);
void* ali_region_alloc(AliRegion* self, size_t size);

void* ali_arena_alloc(AliArena* self, size_t size);
void* ali_arena_memdup(AliArena* self, const void* mem, size_t size_bytes);
char* ali_arena_strdup(AliArena* self, const char* cstr);
char* ali_arena_sprintf(AliArena* self, const char* fmt, ...);

AliArenaMark ali_arena_mark(AliArena* self);
void ali_arena_rollback(AliArena* self, AliArenaMark mark);
void ali_arena_reset(AliArena* self);
void ali_arena_free(AliArena* self);
// ali_arena end

// ali_sv
typedef struct {
	char* start;
	size_t len;
}AliSv;

#define ALI_SV_FMT "%.*s"
#define ALI_SV_F(sv) (int)(sv).len, (sv).start

#define ali_sv_from_cstr(cstr) ((AliSv) { .start = cstr, .len = strlen(cstr) })
#define ali_sv_from_parts(start_, len_) ((AliSv) { .start = start_, .len = len_ })

void ali_sv_step(AliSv* self);
AliSv ali_sv_trim_left(AliSv self);
AliSv ali_sv_trim_right(AliSv self);
AliSv ali_sv_trim(AliSv self);

AliSv ali_sv_chop_left(AliSv* self, size_t n);
AliSv ali_sv_chop_right(AliSv* self, size_t n);
AliSv ali_sv_chop_by_c(AliSv* self, char c);

bool ali_sv_chop_long(AliSv* self, int base, long* out);
bool ali_sv_chop_float(AliSv* self, float* out);
bool ali_sv_chop_double(AliSv* self, double* out);

bool ali_sv_eq(AliSv left, AliSv right);
bool ali_sv_starts_with(AliSv self, AliSv prefix);
bool ali_sv_ends_with(AliSv self, AliSv suffix);

// ali_sv end

// ali_sb
typedef struct {
	char* data;
	size_t count;
	size_t capacity;
}AliSb;

void ali_sb_maybe_resize(AliSb* self, size_t to_add);
void ali_sb_push_strs_null(AliSb* self, ...);
void ali_sb_push_sprintf(AliSb* self, const char* fmt, ...);
void ali_sb_free(AliSb* self);

bool ali_sb_read_file(AliSb* self, const char* path);
bool ali_sb_write_file(AliSb* self, const char* path);

#define ali_sb_push_strs(...) ali_sb_push_strs_null(__VA_ARGS__, NULL)

// ali_sb end

#endif // ALI_H_

#ifdef ALI_IMPLEMENTATION
#undef ALI_IMPLEMENTATION
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

// ali_log

static_assert(LOG_COUNT_ == 3, "Log level was added");
const char* loglevel_to_str[LOG_COUNT_] = {
	[LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",
	[LOG_ERROR] = "ERROR",
};

FILE* ali_global_logfile = NULL;
AliLogLevel ali_global_loglevel = LOG_INFO;

void ali_init_global_log() {
	ali_global_logfile = stdout;
	ali_global_loglevel = LOG_INFO;
}

void ali_log_log(AliLogLevel level, const char* fmt, ...) {
	ALI_ASSERT(ali_global_logfile != NULL);

	va_list args;
	va_start(args, fmt);

	if (ali_global_loglevel >= level) {
		fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
		vfprintf(ali_global_logfile, fmt, args);
		fprintf(ali_global_logfile, "\n");
	}

	va_end(args);
}

// ali_log end

// ali_da
AliDaHeader* ali_da_new_header_with_size(size_t init_capacity, size_t item_size) {
	AliDaHeader* h = ALI_MALLOC(sizeof(*h) + item_size * init_capacity);
	h->count = 0;
	h->capacity = init_capacity;
	return h;
}

AliDaHeader* ali_da_get_header_with_size(void* da, size_t item_size) {
	if (da == NULL) return ali_da_new_header_with_size(ALI_DA_DEFAULT_INIT_CAPACITY, item_size);
	
	return (AliDaHeader*)da - 1;
}

void* ali_da_maybe_resize_with_size(void* da, size_t to_add, size_t item_size) {
	AliDaHeader* h = ali_da_get_header_with_size(da, item_size);
	
	if (h->count + to_add >= h->capacity) {
		while (h->count + to_add >= h->capacity) {
			if (h->capacity == 0) h->capacity = ALI_DA_DEFAULT_INIT_CAPACITY;
			else h->capacity *= 2;
		}

		h = ALI_REALLOC(h, sizeof(*h) + h->capacity * item_size);
	}

	return h->data;
}

void* ali_da_free_with_size(void* da, size_t item_size) {
	AliDaHeader* h = ali_da_get_header_with_size(da, item_size);
	ALI_FREE(h);
	return (da = NULL);
}

// ali_da end

// ali_arena

AliRegion* ali_region_new(size_t capacity) {
	AliRegion* new = ALI_MALLOC(sizeof(*new) + capacity);
	ALI_ASSERT(new != NULL);
	new->count = 0;
	new->capacity = capacity;
	new->next = NULL;
	return new;
}

void* ali_region_alloc(AliRegion* self, size_t size) {
	if (self->count + size >= self->capacity) return NULL;
	void* ptr = self->data + self->count;
	self->count += size;
	return ptr;
}

void* ali_arena_alloc(AliArena* self, size_t size) {
	if (self->start == NULL) {
		self->start = ali_region_new(ALI_REGION_DEFAULT_CAP);
		self->end = self->start;
	}

	AliRegion* region = self->end;
	void* ptr;
	do {
		ptr = ali_region_alloc(region, size);
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

void* ali_arena_memdup(AliArena* self, const void* mem, size_t size_bytes) {
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

// ali_arena end

// ali_sv

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

AliSv ali_sv_chop_left(AliSv* self, size_t n) {
	if (self->len < n) n = self->len;

	AliSv chunk = ali_sv_from_parts(self->start, n);
	self->start += n;
	self->len -= n;

	return chunk;
}

AliSv ali_sv_chop_right(AliSv* self, size_t n) {
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

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_float(AliSv* self, float* out) {
	char* endptr;
	float num = strtof(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_chop_double(AliSv* self, double* out) {
	char* endptr;
	double num = strtod(self->start, &endptr);
	if (endptr == self->start) return false;
	*out = num;

	size_t n = endptr - self->start;
	self->start += n;
	self->len -= n;

	return true;
}

bool ali_sv_eq(AliSv left, AliSv right) {
	bool eq = left.len == right.len;
	for (size_t i = 0; eq && i < left.len; ++i) {
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

// ali_sv end

// ali_sb
void ali_sb_maybe_resize(AliSb* self, size_t to_add) {
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
		size_t n = strlen(str);
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
		fprintf(stderr, "Couldn't read %s: %s\n", path, strerror(errno));
		return false;
	}

	fseek(f, 0, SEEK_END);
	size_t n = ftell(f);
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
		fprintf(stderr, "Couldn't write to %s: %s\n", path, strerror(errno));
		return false;
	}

	fwrite(self->data, 1, self->count, f);

	if (f != NULL) fclose(f);
	return true;
}

// ali_sb end

#endif // ALI_IMPLEMENTATION

#ifdef ALI_REMOVE_PREFIX
#undef ALI_REMOVE_PREFIX

// ali_util
#define ARRAY_LEN ALI_ARRAY_LEN
#define UNUSED ALI_UNUSED
#define UNREACABLE ALI_UNREACHABLE
#define TODO ALI_TODO
#define PANIC ALI_PANIC
#define RETURN_DEFER ALI_RETURN_DEFER
// ali_util end

// ali_types
#define u8 ali_u8
#define u16 ali_u16
#define u32 ali_u32
#define u64 ali_u64

#define i8 ali_i8
#define i16 ali_i16
#define i32 ali_i32
#define i64 ali_i64
// ali_types end

// ali_log

#define global_logfile ali_global_logfile
#define global_loglevel ali_global_loglevel

#define init_global_log ali_init_global_log
#define log_log ali_log_log

#define log_info ali_log_info
#define log_warn ali_log_warn
#define log_error ali_log_error

// ali_log end

// ali_da
#define da_new_header_with_size ali_da_new_header_with_size
#define da_get_header_with_size ali_da_get_header_with_size
#define da_maybe_resize_with_size ali_da_maybe_resize_with_size
#define da_free_with_size ali_da_free_with_size

#define da_new_header ali_da_new_header
#define da_get_header ali_da_get_header
#define da_maybe_resize ali_da_maybe_resize
#define da_getlen ali_da_getlen

#define da_append
#define da_free

#define da_remove_unordered ali_da_remove_unordered
#define da_remove_ordered ali_da_remove_ordered

#define da_for ali_da_for
#define da_foreach ali_da_foreach
// ali_da end

// ali_arena
#define region_new ali_region_new
#define region_alloc ali_region_alloc

#define arena_alloc ali_arena_alloc
#define arena_memdup ali_arena_memdup
#define arena_strdup ali_arena_strdup
#define arena_sprintf ali_arena_sprintf

#define arena_mark ali_arena_mark
#define arena_rollback ali_arena_rollback
#define arena_reset ali_arena_reset
#define arena_free ali_arena_free
// ali_arena end

// ali_sv
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
// ali_sv end

// ali_sb
#define sb_maybe_resize ali_sb_maybe_resize
#define sb_push_strs_null ali_sb_push_strs_null
#define sb_push_sprintf ali_sb_push_sprintf
#define sb_free ali_sb_free

#define sb_push_strs ali_sb_push_strs

#define sb_read_file ali_sb_read_file
#define sb_write_file ali_sb_write_file
// ali_sb end

#endif // ALI_REMOVE_PREFIX

