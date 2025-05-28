//
//  main.cpp
//  FloatingPoint
//
//  Created by Ian Ollmann on 5/27/25.
//

#include <math.h>
#include <float.h>
#include "Math.hpp"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <dispatch/dispatch.h>

#if DEBUG
#else
#   define NDEBUG   1
#endif
#include <assert.h>

typedef float (*UnaryFunction)(float);
typedef double (*ReferenceFunction)(double);

static inline bool IsFloatEqual( float test, float reference)
{
    union{ float f; uint32_t u; }uTest = {test};
    union{ float f; uint32_t u; }uReference = {reference};

    if( uTest.u == uReference.u)
        return true;

    if( isnan(test) && isnan(reference))
        return true;
    
    return false;
}

#include <atomic>
int TestFunction( UnaryFunction testF, UnaryFunction referenceF)
{
    __block int result = 0;
    __block volatile float failCase = NAN;
    
    constexpr unsigned long kIterationStride = 1UL << 16;
    dispatch_apply( (1ULL << 32) / kIterationStride,
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
                   ^(size_t iteration)
    {
        if( result )
            return;
        
        uint64_t start = iteration * kIterationStride;
        uint64_t stop = start + kIterationStride;
        union{ uint32_t u;  float f;}u;
        
        for( uint64_t val = start; val <= stop; val++)
        {
            u.u = uint32_t(val);
            
            float test = testF(u.f);
            float reference = referenceF(u.f);
            
            if( ! IsFloatEqual(test, reference))
            {
                failCase = u.f;
                result = -1;
                return;
            }
        }
    });
    
    if(result)
        printf( "Test(%a) failed: *%a vs %a\n", failCase, referenceF(failCase), testF(failCase));

    return result;
}


static inline float FloatUlps( float test, double correct )
{
    if( isnan(correct) && isnan(test))
        return 0;
    if( (double) test == correct )
        return 0;
    
    // Figure out the error
    double delta = test - correct;
    
    // Scale the error against the correct float-point exponent
    int referenceExp = 0;
    double referenceFract = frexp(correct, &referenceExp);
    int deltaExp = 0;
    double fraction = frexp(delta, &deltaExp );
    int ulpExp = FLT_MANT_DIG - (referenceExp - deltaExp);
    //Modified Goldberg Ulp  https://inria.hal.science/inria-00070503v1/document
    if(fabs(referenceFract) == 0.5)
        fraction *= 2;
    return (float) ldexp(fraction, ulpExp);
}

int TestTranscendental( UnaryFunction testF, ReferenceFunction referenceF)
{
    constexpr float tolerance = 0.625f;
    
    __block int result = 0;
    __block volatile float worstCase = NAN;
    __block volatile float worstError = 0;
    
    constexpr unsigned long kIterationStride = 1UL << 16;
    dispatch_apply( 100000000ULL / kIterationStride,
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
                   ^(size_t iteration)
    {
        uint64_t start = iteration * kIterationStride;
        uint64_t stop = start + kIterationStride;
        union{ uint32_t u;  float f;}u;

        if( result )
            return;
        
        for( uint64_t val = start; val <= stop; val++)
        {
            u.u = uint32_t(val);
            
            float test = testF(u.f);
            double reference = referenceF(u.f);
            
            float error = FloatUlps( test, reference );
            if( fabs(error) > fabs(worstError))
            {
                worstCase = u.f;
                worstError = error;
                
                if( fabs(worstError) > tolerance )
                {
                    result = -1;
                    return;
                }
            }
        }
    });
    
    printf( "(Worst case: %f ulps @ %a) ", worstError, worstCase);
    return result;
}


static void DetectLeaks()
{
    char cmd[256];  snprintf(cmd, sizeof(cmd),  "/usr/bin/leaks %d\n", getpid());
    int hasLeaks = system(cmd);
    
    assert( 0 == hasLeaks);     // If we stopped here, the code is leaking memory. See debug console for leaks report.
    // Try turning on Malloc Stack logging in the Xcode Scheme for better backtrace info in the leaks report
}


int main(int argc, const char * argv[])
{
    atexit( DetectLeaks );
    int error;
    
    printf( "Testing floor...");
    if( (error = TestFunction( Floor, floorf)))
        return error;
    printf( "passed\n");

    printf( "Testing round...");
    if( (error = TestFunction( Round, roundf)))
        return error;
    printf( "passed\n");

    printf( "Testing rint...");
    if( (error = TestFunction( Rint, rintf)))
        return error;
    printf( "passed\n");

    printf( "Testing log...");
    if( (error = TestTranscendental( Log, log)))
        return error;
    printf( "passed\n\n\n");

    return error;
}
