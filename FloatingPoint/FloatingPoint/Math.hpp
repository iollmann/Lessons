//
//  Math.h
//  FloatingPoint
//
//  Created by Ian Ollmann on 5/27/25.
//
//  Assignment:
//  ===========
//  Write some floating-point functions for a math library (below). We will ignore floating-point
//  state and exceptions for now.
//
//  References:
//      What every computer scientists should know about floating-point arithmetic
//          https://www.itu.dk/~sestoft/bachelor/IEEE754_article.pdf
//
//      The IEEE-754 floating-point standard (1985 version for simplicity)
//          https://en.wikipedia.org/wiki/IEEE_754-1985
//
//      The C++ math library   (math.h)
//          https://en.cppreference.com/w/c/numeric/math/
//
//      Some other floating-point math facts (float.h)
//          https://cplusplus.com/reference/cfloat/
//
//      Not a bad summary of stuff to know:  (see also Review questions in same)
//          https://courses.physics.illinois.edu/cs357/sp2020/notes/ref-4-fp.html
//
//  Things to learn about first:
//      The four (5) floating-point rounding modes.
//      Signed zeros
//      Infinity
//      NaN
//      The single precision floating-point representation (what the bits mean)
//          https://www.ibm.com/docs/en/aix/7.2.0?topic=types-single-precision-floating-point
//      How to specify floating-point literals with the right precision:
//          e.g.        1.6f, 1.6, 1.6L
//      C compiler floating-point precision rules:
//          namely, the compiler is free to implicitly convert smaller precision floating-point
//          types to larger floating-point types and do the arithmetic at higher precision for
//          a while before rounding back down to the prescribed precision. Things this affects:
//              Fused multiply add  (FMA)
//              x87  (the AMD-Intel floating-point unit)
//              underflow / overflow
//      catastrophic cancellation
//      double rounding
//      -Wconversion compiler flag
//      default rounding directions for float->int conversions and int->float; When do they round?
//      Floating point p-notation. e.g.  0x1.0p-2    https://en.wikipedia.org/wiki/Hexadecimal#Hexadecimal_exponential_notation
//      ULPs        https://en.wikipedia.org/wiki/Unit_in_the_last_place
//                  https://inria.hal.science/inria-00070503v1/document
//
//     Questions:
//     ==========
//      1) Why is a simple function like hypot() in the C math library?
//      2) why are log1p and expm1 in the math library?
//      3) why are there both sin and sinpi in the math library?  https://en.cppreference.com/w/c/numeric/math/sinpi
//      4) What is the largest single precision floating-point number that can still
//          represent less than unit precision;
//         What is the first floating-point number that can no longer represent the
//          next consecutive integer?
//

/*! @abstract return the largest integral value less than or equal to x. Does not change sign of x. */
float Floor(float x);

/*! @abstract return the integral value nearest to x rounding half-way cases away from zero. Does not change sign of x. */
float Round(float x);

/*! @abstract return the integral value nearest to x (according to the prevailing rounding mode) in floating-point format. Does not change sign of x. */
float Rint(float x);        // we only worry about the default rounding mode for this assignment


/*! @abstract computes the value of the logarithm of argument x to base 2
 *  @discussion     log(0) returns -Infinity
                    log(1) returns +0
 *                  log(inf)  returns inf
 *                  log(-x) returns NaN */
float Log2(float x);

