#ifndef ALI_MATH_H_
#define ALI_MATH_H_

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

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t);
ali_f32 ali_normalizef(ali_f32 start, ali_f32 end, ali_f32 value);
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

ali_f32 ali_lerpf(ali_f32 a, ali_f32 b, ali_f32 t) {
	return a + (b - a) * t;
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
	ALI_MEMCPY(state->state, seed, sizeof(state->state));
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
