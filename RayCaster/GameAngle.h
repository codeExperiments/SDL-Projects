#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "Real.h"

#define PI 3.14159265358979323846f

typedef int32_t Angle;
typedef struct TrigValues TrigValues;
typedef struct AngleData AngleData;

struct TrigValues {
	Real sin, isin;
	Real cos, icos;
	//Real tan, itan;
};

struct AngleData {
	int32_t numAngles;
	Real *colCos;
	TrigValues *trigTable;
	Angle fov;
};

//AngleData *createAngleData(int w, Real fov);
void initAngleData(AngleData *pa, int w, int fov);
void resetAngleData(AngleData *pa);
Angle convertFromDegrees(const AngleData *pa, Real d);
Real convertFromAngles(const AngleData *pa, Angle a);
Angle getAngle2PI(const AngleData *pa);
void updateAngleData(AngleData *pa, int w, Real fov);
Angle getFirstRayAngle(const AngleData *pa, Angle playerAngle);
Angle incAngle(const AngleData *pa, Angle angle, int c);
Real getColCos(const AngleData *pa, int col);
Real getCos(const AngleData *pa, Angle);
Real getSin(const AngleData *pa, Angle);
Real getInvCos(const AngleData *pa, Angle);
Real getInvSin(const AngleData *pa, Angle);
Real getTan(const AngleData *pa, Angle);
Real getInvTan(const AngleData *pa, Angle);

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
	pa->trigTable = NULL;
	updateAngleData(pa, w, IntToReal(fov));
}

static inline void resetAngleData(AngleData *pa) {
	free(pa->trigTable);
	free(pa->colCos);
	pa->trigTable = NULL;
	pa->colCos = NULL;
}

static inline Angle convertFromDegrees(const AngleData *pa, Real d) {
	return RealToInt(mulReal(divReal(d, IntToReal(360)), IntToReal(pa->numAngles)));
}

// angles to radians
static inline Real convertFromAngles(const AngleData *pa, Angle a) {
	return FloatToReal((float)a / pa->numAngles * 2 * PI);
	//return FloatToReal((float)a / pa->numAngles * 360); // to degrees
}

static inline Angle getAngle2PI(const AngleData *pa)
{
	return pa->numAngles;
}

// fov degrees
static inline void updateAngleData(AngleData *pa, int w, Real fov) {
	pa->numAngles = RealToInt(mulReal(divReal(IntToReal(360), fov), IntToReal(w)));
	pa->fov = w;
	Angle a360 = pa->numAngles;
	Angle a90 = a360 / 4;
	Angle a180 = a360 / 2;
	// init trig table
	pa->trigTable = (TrigValues*)malloc(sizeof(TrigValues) * a360);
	for (Angle i = 0; i != a360; ++i) {
		float aRad = (3.272e-4f) + i * PI / a180;
		float sinVal = sinf(aRad);
		pa->trigTable[i].sin = FloatToReal(sinVal);
		pa->trigTable[i].isin = FloatToReal(1.f / sinVal);
		float cosVal = cosf(aRad);
		pa->trigTable[i].cos = FloatToReal(cosVal);
		pa->trigTable[i].icos = FloatToReal(1.f / cosVal);
		float tanVal = tanf(aRad);
		//pa->trigTable[i].tan = FloatToReal(tanVal);
		//pa->trigTable[i].itan = FloatToReal(1.f / tanVal);
	}
	// init columns cos table
	pa->colCos = (Real*)malloc(sizeof(Real) * w);
	Angle rayAngle = getFirstRayAngle(pa, 0);
	for (int i = 0; i != w; ++i) {
		pa->colCos[i] = getCos(pa, rayAngle);
		rayAngle = incAngle(pa, rayAngle, 1);
	}
}

static inline Angle getFirstRayAngle(const AngleData *pa, Angle playerAngle)
{
	Angle angle = playerAngle - pa->fov/2;
	if (angle < 0) {
		angle += pa->numAngles;
	}
	return angle;
}

// angle>=0, c>=0
static inline Angle incAngle(const AngleData *pa, Angle angle, int c)
{
	Angle nextAngle = angle + c;
	if (nextAngle >= pa->numAngles) {
		nextAngle -= pa->numAngles;
	}
	return nextAngle;
}

static inline Real getColCos(const AngleData *pa, int col) {
	return pa->colCos[col];
}

// a >= 0, a < pa->numAngles
static inline Real getCos(const AngleData *pa, Angle a)
{
	return pa->trigTable[a].cos;
}

// a >= 0, a < pa->numAngles
static inline Real getSin(const AngleData *pa, Angle a)
{
	return pa->trigTable[a].sin;
}

// a >= 0, a < pa->numAngles
static inline Real getInvCos(const AngleData *pa, Angle a)
{
	return pa->trigTable[a].icos;
}

// a >= 0, a < pa->numAngles
static inline Real getInvSin(const AngleData *pa, Angle a)
{
	return pa->trigTable[a].isin;
}

// a >= 0, a < pa->numAngles
static inline Real getTan(const AngleData *pa, Angle a)
{
	//return pa->trigTable[a].tan;
	Real sin = getSin(pa, a);
	Real icos = getInvCos(pa, a);
	return mulReal(sin, icos);
}

// a >= 0, a < pa->numAngles
static inline Real getInvTan(const AngleData *pa, Angle a)
{
	Real isin = getInvSin(pa, a);
	Real cos = getCos(pa, a);
	return mulReal(cos, isin);
}