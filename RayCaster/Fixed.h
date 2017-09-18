#pragma once
#include <stdint.h>

#define FIX_SHIFT 16
#define FLOAT_SCALER ((float)(1<<FIX_SHIFT))
#define R_FLOAT_SCALER (1.f / FLOAT_SCALER)

typedef int32_t Real;

static inline Real IntToReal(int32_t i) {
	return ((int32_t)i) << FIX_SHIFT;
}

static inline int32_t RealToInt(Real r) {
	return r >> FIX_SHIFT;
}

static inline Real FloatToReal(float f) {
	return (Real)(f * FLOAT_SCALER + (f >= 0.f ? 0.5f : -0.5f));
}

static inline float RealToFloat(Real r) {
	return ((float)r) * R_FLOAT_SCALER;
}

// Perform a fixed point multiplication without a 64-bit intermediate result.
// This is fast but beware of overflow!
static inline Real mulRealF(Real a, Real b)
{
	return (a * b) >> FIX_SHIFT;
}

// Perform a fixed point multiplication using a 64-bit intermediate result to
// prevent overflow problems.
static inline Real mulReal(Real a, Real b)
{
	return (Real)(((int64_t)a * b) >> FIX_SHIFT);
}

static inline Real divReal(Real a, Real b)
{
	return (Real)((((int64_t)a) << FIX_SHIFT) / b);
}

static inline Real invReal(Real a)
{
	return divReal(RealToInt(1), a);
}

static inline Real absReal(Real a)
{
	return (a ^ (a >> 31)) - (a >> 31);
}