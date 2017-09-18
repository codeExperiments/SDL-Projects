#include <math.h>
#include "Real.h"
#include "CosSinTable.h"
#include "commonDef.h"

#ifdef USE_TRIG_LUT
Real fast_cossin_table[MAX_CIRCLE_ANGLE];           // Declare table of fast cosinus and sinus

void buildCosSinTable()
{
	for (int i = 0; i < MAX_CIRCLE_ANGLE; i++)
	{
		float sinValue = sinf((3.272e-4f) + i * PI / HALF_MAX_CIRCLE_ANGLE);
		fast_cossin_table[i] = FloatToReal(sinValue);
	}
}

#endif