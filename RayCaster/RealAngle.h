#pragma once
#include <stdlib.h>
#include <math.h>
#include "Real.h"
#include "CosSinTable.h"
#include "commonDef.h"


#ifdef USE_TRIG_LUT
extern Real fast_cossin_table[MAX_CIRCLE_ANGLE];           // Declare table of fast cosinus and sinus
#endif

// use real values to express angles in radiants
typedef Real Angle;
typedef struct AngleData AngleData;

struct AngleData {
	Real *colCos;
	Real colInc; // ray angle increment between adjacent columns
	Angle fov; // radians
};

#ifdef USE_TRIG_LUT
Real fastcos(AngleData *pa, Angle n);
Real fastsin(AngleData *pa, Angle n);
#endif

Angle convertFromDegrees(const AngleData *pa, Real d);
Real convertFromAngles(const AngleData *pa, Angle a);
Angle getAngle2PI(const AngleData *pa);

//AngleData *createAngleData(int w, Real fov);
void initAngleData(AngleData *pa, int w, int fov);
void resetAngleData(AngleData *pa);
void updateAngleData(AngleData *a, int w, Real fov);

Angle getFirstRayAngle(const AngleData *a, Angle playerAngle);
Angle incAngle(const AngleData *a, Angle angle, int c);
Real getColCos(const AngleData *a, int col);
Real getCos(const AngleData *, Angle);
Real getSin(const AngleData *, Angle);
Real getInvCos(const AngleData *pa, Angle);
Real getInvSin(const AngleData *pa, Angle);

// degrees -> angles
static inline Angle convertFromDegrees(const AngleData *pa, Real d) {
	return mulReal(divReal(d, IntToReal(360)), FloatToReal(_2PI));
	//return (d * _2PI / 360);
}

// angles -> radians
static inline Real convertFromAngles(const AngleData *pa, Angle a) {
	return a;
	//return mulReal(divReal(a, FloatToReal(_2PI)), IntToReal(360)); // to degrees
}

static inline Angle getAngle2PI(const AngleData *pa) {
	return FloatToReal(_2PI);
}

// fov degrees
//AngleData *createAngleData(int w, Real fov) {
//	AngleData *angleData = (AngleData *)malloc(sizeof(AngleData));
//	angleData->colCos = NULL;
//	updateAngleData(angleData, w, fov);
//	return angleData;
//}

// fov degrees
static inline void initAngleData(AngleData *pa, int w, int fov) {
	pa->colCos = NULL;
	updateAngleData(pa, w, IntToReal(fov));
}


static inline void resetAngleData(AngleData *pa) {
	free(pa->colCos);
	pa->colCos = NULL;
}

// fov degrees
static void updateAngleData(AngleData *pa, int w, Real fov) {
	pa->fov = convertFromDegrees(NULL, fov); // fov, degrees to radians
	pa->colInc = divReal(pa->fov, IntToReal(w));//fov / w;
	//pa->pi = FloatToReal(PI);//convertFromDegrees(NULL, FloatToReal(180.f));
	//pa->_2pi = FloatToReal(_2PI);//convertFromDegrees(NULL, FloatToReal(360.f));
	pa->colCos = (Real*)malloc(sizeof(Real) * w);
	Angle rayAngle = getFirstRayAngle(pa, 0);
	for (int i = 0; i != w; ++i) {
		pa->colCos[i] = getCos(pa, rayAngle);
		rayAngle = incAngle(pa, rayAngle, 1);
	}
}

static inline Angle getFirstRayAngle(const AngleData *pa, Angle playerAngle) {
	Angle angle = playerAngle - pa->fov / 2;
	if (angle < 0.f) {
		angle += FloatToReal(_2PI);
	}
	return angle;
}

// angle>=0, c>=0
static inline Angle incAngle(const AngleData *pa, Angle angle, int c) {
	Angle nextAngle = angle + mulReal(pa->colInc, IntToReal(c)); // angle + c * a->colInc;
	if (nextAngle >= FloatToReal(_2PI)) {
		nextAngle -= FloatToReal(_2PI);
	}
	return nextAngle;
}

static inline Real getColCos(const AngleData *pa, int col) {
	return pa->colCos[col];
}

//Real getColSin(AngleData *a, int col) {
//	return a->colCos[col]; // errato
//}

// a >= 0
static inline Real getCos(const AngleData *pa, Angle a) {
#ifndef USE_TRIG_LUT
	return FloatToReal(cosf(RealToFloat(a)));
#else
	return fastcos(pa, a);
#endif
}

// a >= 0
static inline Real getSin(const AngleData *pa, Angle a) {
#ifndef USE_TRIG_LUT
	return FloatToReal(sinf(RealToFloat(a)));
#else
	return fastsin(pa, a);
#endif
}

// a >= 0
static inline Real getInvCos(const AngleData *pa, Angle a)
{
	Real cos = getCos(pa, a);
	return divReal(IntToReal(1), cos);
}

// a >= 0
static inline Real getInvSin(const AngleData *pa, Angle a)
{
	Real sin = getSin(pa, a);
	return divReal(IntToReal(1), sin);
}

// a >= 0
static inline Real getTan(const AngleData *pa, Angle a)
{
	Real sin = getSin(pa, a);
	Real cos = getCos(pa, a);
	return divReal(sin, cos);
}

// a >= 0
static inline Real getInvTan(const AngleData *pa, Angle a)
{
	Real sin = getSin(pa, a);
	Real cos = getCos(pa, a);
	return divReal(cos, sin);
}

#ifdef USE_TRIG_LUT

// n>= 0
static inline Real fastcos(AngleData *pa, Angle n)
{
	//float f = n * HALF_MAX_CIRCLE_ANGLE / PI;
	//int i = (int)f;
	Real f = mulReal(divReal(n, FloatToReal(PI)), IntToReal(HALF_MAX_CIRCLE_ANGLE)); // 
	int i = RealToInt(f);

	return fast_cossin_table[(i + QUARTER_MAX_CIRCLE_ANGLE) & MASK_MAX_CIRCLE_ANGLE];
}

// n >= 0
static inline Real fastsin(AngleData *pa, Angle n)
{
	Real f = mulReal(divReal(n, FloatToReal(PI)), IntToReal(HALF_MAX_CIRCLE_ANGLE)); // 
	int i = RealToInt(f);

	return fast_cossin_table[i & MASK_MAX_CIRCLE_ANGLE];
}

#endif