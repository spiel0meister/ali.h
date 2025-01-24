#ifndef ALI_MATH_H_
#define ALI_MATH_H_
#include <immintrin.h>

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

#ifndef ALI_SQRTF
#include <math.h>
#define ALI_SQRTF sqrtf
#endif // ALI_SQRTF

#ifndef ALI_SINF
#include <math.h>
#define ALI_SINF sinf
#endif // ALI_SINF

#ifndef ALI_COSF
#include <math.h>
#define ALI_COSF cosf
#endif // ALI_COSF

#ifndef ALI_ATAN2F
#include <math.h>
#define ALI_ATAN2F atan2f
#endif // ALI_ATAN2F

#ifndef PI
#define PI 3.141592653f
#endif // PI

typedef struct {
    ali_f32 x, y;
}AliVector2;
#define ALI_VECTOR2(x, y) ((AliVector2) { x, y })
#define ALI_VECTOR2_FMT "(%f, %f)"
#define ALI_VECTOR2_F(v) v.x, v.y

AliVector2 ali_vec2_zero(void);
AliVector2 ali_vec2_from_angle(ali_f32 rads);
AliVector2 ali_vec2_from_scalar(ali_f32 scalar);
AliVector2 ali_vec2_normalize(AliVector2 self);

AliVector2 ali_vec2_add(AliVector2 self, AliVector2 that);
AliVector2 ali_vec2_sub(AliVector2 self, AliVector2 that);
AliVector2 ali_vec2_mul(AliVector2 self, AliVector2 that);
AliVector2 ali_vec2_div(AliVector2 self, AliVector2 that);
AliVector2 ali_vec2_scale(AliVector2 self, ali_f32 scalar);

ali_f32 ali_vec2_dot(AliVector2 self, AliVector2 that);
ali_f32 ali_vec2_cross(AliVector2 self, AliVector2 that);
ali_f32 ali_vec2_angle(AliVector2 self, AliVector2 that);

ali_f32 ali_vec2_dist_sqr(AliVector2 self, AliVector2 that);
ali_f32 ali_vec2_dist(AliVector2 self, AliVector2 that);

ali_f32 ali_vec2_length_sqr(AliVector2 self);
ali_f32 ali_vec2_length(AliVector2 self);

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t);
ali_f32 ali_normalizef(ali_f32 start, ali_f32 end, ali_f32 value);
ali_f32 ali_remapf(ali_f32 value, ali_f32 in_start, ali_f32 in_end, ali_f32 out_start, ali_f32 out_end);
ali_u64 ali_rotl64(ali_u64 x, ali_u8 k);

ali_f32 ali_quadbezierf(ali_f32 start, ali_f32 end, ali_f32 control, ali_f32 t);
ali_f32 ali_cubebezierf(ali_f32 start, ali_f32 end, ali_f32 control1, ali_f32 control2, ali_f32 t);

// rand

typedef struct {
	ali_u64 state[4];
}AliXoshiro256ppState;

ali_u64 ali_xoshiro256pp_next(AliXoshiro256ppState *state);
void ali_xoshiro256pp_seed(AliXoshiro256ppState *state, ali_u64 seed[4]);

void ali_srand(ali_u64 seed);

ali_u64 ali_rand();
ali_u64* ali_temp_rand_sequence(ali_usize count);
ali_f64 ali_rand_float();
ali_u64 ali_rand_range(ali_u64 min, ali_u64 max);

#endif // ALI_MATH_H_

#ifdef ALI_MATH_IMPLEMENTATION
#undef ALI_MATH_IMPLEMENTATION

// @module ali_math
#include <string.h>

AliVector2 ali_vec2_zero(void) {
    return (AliVector2) { 0, 0 };
}

AliVector2 ali_vec2_from_angle(ali_f32 rads) {
    return (AliVector2) { ALI_COSF(rads), ALI_SINF(rads) };
}

AliVector2 ali_vec2_from_scalar(ali_f32 scalar) {
    return (AliVector2) { scalar, scalar };
}

AliVector2 ali_vec2_normalize(AliVector2 self) {
    ali_f32 len = ali_vec2_length(self);

    if (len > 0) {
        self = ali_vec2_div(self, ali_vec2_from_scalar(len));
    }

    return self;
}

AliVector2 ali_vec2_add(AliVector2 self, AliVector2 that) {
#ifdef __SSE__
    __m128 a = { self.x, self.y };
    __m128 b = { that.x, that.y };
    __m128 c = _mm_add_ps(a, b);
    self.x = c[0];
    self.y = c[1];
#else
    self.x += that.x;
    self.y += that.y;
#endif
    return self;
}

AliVector2 ali_vec2_sub(AliVector2 self, AliVector2 that) {
#ifdef __SSE__
    __m128 a = { self.x, self.y };
    __m128 b = { that.x, that.y };
    __m128 c = _mm_sub_ps(a, b);
    self.x = c[0];
    self.y = c[1];
#else
    self.x -= that.x;
    self.y -= that.y;
#endif
    return self;
}

AliVector2 ali_vec2_mul(AliVector2 self, AliVector2 that) {
#ifdef __SSE__
    __m128 a = { self.x, self.y };
    __m128 b = { that.x, that.y };
    __m128 c = _mm_mul_ps(a, b);
    self.x = c[0];
    self.y = c[1];
#else
    self.x *= that.x;
    self.y *= that.y;
#endif
    return self;
}

AliVector2 ali_vec2_div(AliVector2 self, AliVector2 that) {
#ifdef __SSE__
    __m128 a = { self.x, self.y };
    __m128 b = { that.x, that.y };
    __m128 c = _mm_div_ps(a, b);
    self.x = c[0];
    self.y = c[1];
#else
    self.x /= that.x;
    self.y /= that.y;
#endif
    return self;
}

AliVector2 ali_vec2_scale(AliVector2 self, ali_f32 scalar) {
    return ali_vec2_mul(self, ali_vec2_from_scalar(scalar));
}

ali_f32 ali_vec2_length_sqr(AliVector2 self) {
    return self.x * self.x + self.y * self.y;
}

ali_f32 ali_vec2_dot(AliVector2 self, AliVector2 that) {
    return (self.x * that.x + self.y * that.y);
}

ali_f32 ali_vec2_cross(AliVector2 self, AliVector2 that) {
    return (self.x * that.y - self.y * that.x);
}

ali_f32 ali_vec2_angle(AliVector2 self, AliVector2 that) {
    return ALI_ATAN2F(ali_vec2_cross(self, that), ali_vec2_dot(self, that));
}

ali_f32 ali_vec2_dist_sqr(AliVector2 self, AliVector2 that) {
    ali_f32 dx = self.x - that.x;
    ali_f32 dy = self.y - that.y;
    return dx * dx + dy * dy;
}

ali_f32 ali_vec2_dist(AliVector2 self, AliVector2 that) {
    return ALI_SQRTF(ali_vec2_dist_sqr(self, that));
}


ali_f32 ali_vec2_length(AliVector2 self) {
    return ALI_SQRTF(ali_vec2_length_sqr(self));
}

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t) {
	return a + (b - a) * t;
}

ali_f32 ali_remapf(ali_f32 value, ali_f32 in_start, ali_f32 in_end, ali_f32 out_start, ali_f32 out_end) {
    return (value - in_start)/(in_end - in_start) * (out_end - out_start) + out_start;
}

ali_f32 ali_normalizef(ali_f32 start, ali_f32 end, ali_f32 value) {
	return (value - start) / (end - start);
}

ali_u64 ali_rotl64(ali_u64 x, ali_u8 k) {
	return (x << k) | (x >> (64 - k));

}

ali_f32 ali_quadbezierf(ali_f32 start, ali_f32 end, ali_f32 control, ali_f32 t) {
	ali_f32 a = ali_lerpf(start, control, t);
	ali_f32 b = ali_lerpf(control, end, t);
	return ali_lerpf(a, b, t);
}

ali_f32 ali_cubebezierf(ali_f32 start, ali_f32 end, ali_f32 control1, ali_f32 control2, ali_f32 t) {
	ali_f32 a = ali_quadbezierf(start, control1, control2, t);
	ali_f32 b = ali_quadbezierf(control1, control2, end, t);
	return ali_lerpf(a, b, t);
}

// @module ali_math end


// @module ali_rand

AliXoshiro256ppState xoshiro_global_state = { { 0x96EA83C1, 0x218B21E5, 0xAA91FEBD, 0x976414D4 } };
ali_u64 ali_rand_seed = 0xAABBCCDD;

ali_u64 ali_rand_splitmix64() {
    ali_u64 z = (ali_rand_seed += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30))*0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27))*0x94d049bb133111eb;
    return z ^ (z >> 31);
}

void ali_srand(ali_u64 seed) {
    ali_rand_seed = seed;

    xoshiro_global_state.state[0] = (ali_u64)(ali_rand_splitmix64() & 0xffffffff);
    xoshiro_global_state.state[1] = (ali_u64)((ali_rand_splitmix64() & 0xffffffff00000000) >> 32);
    xoshiro_global_state.state[2] = (ali_u64)(ali_rand_splitmix64() & 0xffffffff);
    xoshiro_global_state.state[3] = (ali_u64)((ali_rand_splitmix64() & 0xffffffff00000000) >> 32);
}

ali_u64 ali_xoshiro256pp_next(AliXoshiro256ppState *state) {
	ali_u64 *s = state->state;
	ali_u64 const result = ali_rotl64(s[1] * 5, 7) * 9;
	ali_u64 const t = s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = ali_rotl64(s[3], 11);

	return result;
}

void ali_xoshiro256pp_seed(AliXoshiro256ppState *state, ali_u64 seed[4]) {
	memcpy(state->state, seed, sizeof(state->state));
}

ali_u64* ali_temp_rand_sequence(ali_usize count) {
	ali_u64* out = ali_temp_alloc(sizeof(*out) * count);
	for (ali_usize i = 0; i < count; ++i) out[i] = ali_rand();
	return out;
}

ali_f64 ali_rand_float() {
    ali_u64 value = ali_rand();
    return (ali_f64)value / (ali_f64)UINT64_MAX;
}

ali_u64 ali_rand_range(ali_u64 min, ali_u64 max) {
    if (min > max) ALI_SWAP(ali_u64, &min, &max);
    return ali_rand() % (max - min) + min;
}

ali_u64 ali_rand() {
	return ali_xoshiro256pp_next(&xoshiro_global_state);
}

// @module ali_rand end

#endif // ALI_MATH_IMPLEMENTATION

#ifdef ALI_MATH_REMOVE_PREFIX
#ifndef ALI_MATH_REMOVE_PREFIX_GUARD_
#define ALI_MATH_REMOVE_PREFIX_GUARD_

#ifndef ALI_TYPES_
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

#define VECTOR2_FMT ALI_VECTOR2_FMT
#define VECTOR2_F ALI_VECTOR2_F

#define vec2_zero ali_vec2_zero
#define vec2_from_angle ali_vec2_from_angle
#define vec2_from_scalar ali_vec2_from_scalar
#define vec2_normalize ali_vec2_normalize

#define vec2_add ali_vec2_add
#define vec2_sub ali_vec2_sub
#define vec2_scale ali_vec2_scale

#define vec2_dot ali_vec2_dot
#define vec2_cross ali_vec2_cross
#define vec2_angle ali_vec2_angle

#define vec2_dist_sqr ali_vec2_dist_sqr
#define vec2_dist ali_vec2_dist

#define vec2_length ali_vec2_length
#define vec2_length_sqr ali_vec2_length_sqr

// TODO: should we do this?
// #define lerpf ali_lerpf
// #define normalizef ali_normalizef
// #define rotl64 ali_rotl64
// #define quadbezierf ali_quadbezierf
// #define cubebezierf ali_cubebezierf
#endif // ALI_MATH_REMOVE_PREFIX_GUARD_
#endif // ALI_MATH_REMOVE_PREFIX
