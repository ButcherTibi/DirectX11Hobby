
// Header
#include "BasicMath.h"

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>


float nui::toRadians(float degrees)
{
	return degrees * ((float)M_PI / 180.f);
}
