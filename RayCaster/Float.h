#pragma once

typedef float Real;

static inline Real IntToReal(int i) {
	return (float)i;
}

static inline int RealToInt(Real r) {
	return (int)r;
}

static inline Real FloatToReal(float f) {
	return f;
}

static inline float RealToFloat(Real r) {
	return r;
}

// Perform a fixed point multiplication without a 64-bit intermediate result.
// This is fast but beware of overflow!
static inline Real mulRealF(Real a, Real b)
{
	return a * b;
}

// Perform a fixed point multiplication using a 64-bit intermediate result to
// prevent overflow problems.
static inline Real mulReal(Real a, Real b)
{
	return mulRealF(a, b);
}

static inline Real divReal(Real a, Real b)
{
	return a / b;
}

static inline Real invReal(Real a)
{
	return divReal(1.f, a);
}

static inline Real absReal(Real a)
{
	return fabsf(a);
}