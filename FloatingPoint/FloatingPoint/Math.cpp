//
//  Math.cpp
//  FloatingPoint
//
//  Created by Ian Ollmann on 5/27/25.
//

#include "Math.hpp"
#include <math.h>

float Floor(float x)
{
    return floorf(x);           // TODO: REPLACE ME
}

/*! @abstract return the integral value nearest to x rounding half-way cases away from zero. Does not change sign of x. */
float Round(float x)
{
    return roundf(x);           // TODO: REPLACE ME
}

/*! @abstract return the integral value nearest to x (according to the prevailing rounding mode) in floating-point format. Does not change sign of x. */
float Rint(float x)
{
    return rintf(x);            // TODO: REPLACE ME
}


/*! @abstract computes the value of the natural logarithm of argument x.
 *  @discussion     log(0) returns -Infinity
                    log(1) returns +0
 *                  log(inf)  returns inf
 *                  log(-x) returns NaN */
float Log2(float x)
{
    return log2f(x);             // TODO: REPLACE ME
}
