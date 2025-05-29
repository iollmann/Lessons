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

//
// Assignment 2:  See Math.hpp
//

#if DEBUG
#else
#   define NDEBUG   1
#endif
#include <assert.h>

typedef float (*UnaryFunction)(float);
typedef double (*ReferenceFunction)(double);

/*! @abstract test for float equivalence. Suitable for functions for which one and only one result is allowed*/
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

int TestFunction( UnaryFunction testF, UnaryFunction referenceF)
{
    __block int result = 0;
    __block volatile float failCase = NAN;
    
    // Iterate over all possible single precision floating-point numbers
    // The test is a bit slow so multithread in chunks of kIterationStride
    constexpr unsigned long kIterationStride = 1UL << 16;
    dispatch_apply( (1ULL << 32) / kIterationStride,
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
                   ^(size_t iteration)
    {
        if( result )
            return;
        
        // Find the beginning and ending values to test
        uint64_t start = iteration * kIterationStride;
        uint64_t stop = start + kIterationStride;
        
        for( uint64_t val = start; val < stop; val++)
        {
            // reinterpret the bits as float
            union{ uint32_t u;  float f;}u = {uint32_t(val)};
            
            // Calculate our reference and test values
            float test = testF(u.f);
            float reference = referenceF(u.f);
            
            // Handle any errors
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


static inline double FloatUlps( float test, double correct )
{
    // Remove NaNs from consideration
    if( isnan(correct) && isnan(test))
        return 0;

    // Infinities cause problems calculating delta next, so remove those, and any exact matches
    if( (double) test == correct )
        return 0;
    
    // Figure out the error
    double delta = test - correct;
    
    // Scale the error against the correct float-point exponent
    // Separate values into signed fraction and exponent parts
    int referenceExp = 0;   double referenceFract = frexp(correct, &referenceExp);
    int deltaExp = 0;       double fraction = frexp(delta, &deltaExp );
    
    // Calculate the correct exponent for the ulp
    int ulpExp = FLT_MANT_DIG - (referenceExp - deltaExp);
    //Modified Goldberg Ulp  https://inria.hal.science/inria-00070503v1/document
    if(fabs(referenceFract) == 0.5)
        fraction *= 2;
    
    // Adjust the error to the right exponent
    return ldexp(fraction, ulpExp);
}

/*! @abstract This is for any function for which the results are allowed to be incorrectly rounded */
int TestTranscendental( UnaryFunction testF, ReferenceFunction referenceF)
{
    constexpr float tolerance = 0.625f;
    
    __block int result = 0;
    __block volatile float worstCase = NAN;
    __block volatile double worstError = 0;
    
    // Multithread over chunks kIterationStride for improved performance
    constexpr unsigned long kIterationStride = 1UL << 16;
    dispatch_apply( 100000000ULL / kIterationStride,
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
                   ^(size_t iteration)
    {
        // Calculate the range of values to examine
        uint64_t start = iteration * kIterationStride;
        uint64_t stop = start + kIterationStride;

        // Early exit if there is some serious error
        if( result )
            return;
        
        // Loop over test values
        for( uint64_t val = start; val < stop; val++)
        {
            // reinterpret integer bit pattern as a floating-point value
            union{ uint32_t u;  float f;}u = {uint32_t(val)};
            
            // Calculate test and reference results
            float test = testF(u.f);
            double reference = referenceF(u.f);
            
            // Measure the error
            double error = FloatUlps( test, reference );
            if( fabs(error) > fabs(worstError))
            { // Handle serious error cases
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
    
    printf( "(Worst case: %10.14f ulps @ %a: *%a vs %a) ", worstError, worstCase, referenceF(worstCase), testF(worstCase));
    return result;
}


static void DetectLeaks()
{
    printf( "Checking for leaks....\n");
    
    char cmd[256];  snprintf(cmd, sizeof(cmd),  "/usr/bin/leaks %d\n", getpid());
    int hasLeaks = system(cmd);
    
    assert( 0 == hasLeaks);     // If we stopped here, the code is leaking memory. See debug console for leaks report.
    // Try turning on Malloc Stack logging in the Xcode Scheme for better backtrace info in the leaks report
    
    if( ! hasLeaks )
        printf( "\nNO LEAKS\n");
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

    printf( "Testing log2...");
    if( (error = TestTranscendental( Log2, log2)))
        return error;
    printf( "passed\n\n\n");

    return error;
}


// Assignment 3:  Benchmarking
// ===========================
//
//  Add a benchmarking loop to TestTranscendental and TestFunction above to determine which is faster,
//  the single-precision function you wrote vs. the double precision function in the operating system.
//
//  For this, you will need:
//      1) a fast and accurate timer.  Some CPU timers are not on the CPU and take 100,000 instructions to use.
//          You want the one which is more like 10 instructions or 1 instruction.
//
//              Some candidates:
//                   https://developer.apple.com/documentation/kernel/1462446-mach_absolute_time
//                   https://lemire.me/blog/2021/03/24/counting-cycles-and-instructions-on-the-apple-m1-processor/
//                   Performance monitor counters usually include a cycle time. If you can figure how how to read
//                   PMCs, then you will usually get the capability to read cycles.  https://gist.github.com/ibireme/173517c208c7dc333ba962c1f0d67d12
//
//      2) Some understanding of preemptive multitasking.
//
//          On a unix system, such as macOS (which runs a mach unix kernel under a BSD unix layer), under typical use the kernel
//          time slices up work among the many tasks clamoring for time on the CPU.  For a list, you can use top in the terminal or
//          ActivityMonitor.app. While an in depth description of how this works would take quite a bit of time, for the purpose
//          of this lesson, it is sufficient to say that from time to time your threads will be put to sleep by the CPU in order to
//          do other work in other processes, or perhaps other threads in this process. This of course happens quite commonly when
//          you lock a mutex or wait on a semaphore, but it can also happen simply because a timer expired somewhere in the kernel
//          and it decided you are done for a while. In a period of time, maybe 50 microseconds to a few seconds, your thread will
//          be allowed to run again if it has any work to do. As you might imagine, this can cause trouble for benchmarking. If you
//          are timing a set amount of work to figure out how fast it is and the kernel decides to suspend that work and do something
//          else for a while this will contaminate your results! Not good for the budding young student of the scientific method.
//
//              https://developer.apple.com/library/archive/documentation/Carbon/Conceptual/Multitasking_MultiproServ/02concepts/concepts.html
//
//          Note: As a student of computer science, in a top CS department, you might be asked to write a preemptive multitasking scheduler
//          as part of an entire kernel: https://pdos.csail.mit.edu/6.828/2017/labs/lab4/      It looks like you get 1 week!
//
//      3) Test method
//
//          To solve the noisy data problem we will use statistics. Since many statistics like std deviation are not valid without
//          N>=3 (multiple measurements) we will have to set up a test workload and then run it many times and look at the results to
//          figure out how best to report an accurate score. This is slightly complicated by a few factors:
//
//              1) Use the right timing units.
//
//                  The actual unit of time on a CPU or GPU is 1 cycle, which is the amount of time that passes between when one
//                  instruction can finish out of an execution unit (which does the actual computation, e.g. 1+1=2) and when the
//                  next instruction can finish out of the same execution unit. (Note that this is not quite so simple because the time
//                  when instructions finish is not directly observable because there is a downstream completion stage that determines
//                  whether the results are even needed / valid -- a branch might have mis-predicted. It also makes sure the work observably
//                  completes in the order it appears in the instruction stream. (This is call an out of order processor, which generally
//                  seeks to move work that can complete earlier to an earlier point in time so that it is not getting in the way of the
//                  critical part of the workload that is bottlenecking progress.) This completion stage will generally act to aggregate
//                  instructions together so that they are observed to complete in a clumpy fashion.) You'll have to just trust me
//                  that there is a basic quantum of time that regulates instruction flow through a particular execution unit. More than
//                  one instruction can finish each cycle. Each processor core may have many execution units, sometimes as many as 10 or 11,
//                  capable of operating in parallel, though since they do different types of tasks, usually not all of them are typically
//                  busy at the same time, depending on workload. In addition, there are usually 8-16 cores, each of which may run 1 or 2
//                  threads. However many execution units operating in parallel there are, there is a speed of light. You will get no more
//                  than one instruction per execution unit per cycle throughput.
//
//                  The question of what timing unit to use -- wall clock time or CPU cycle -- depends on what you are trying to
//                  measure and how you are trying to understand you results.  The length of time one cycle can take, "the clock frequency",
//                  can vary.
//
//                      A faster clock frequency, meaning that instructions flow through the processor individually faster, will
//                      mean the transistors have to switch faster. Making a transistor switch faster takes a higher voltage.
//                      Since Power = Voltage * Amperage;  Amperage = Voltage / Resistance; Therefore:  Power = Voltage^2 / Resistance
//                      Per physics, all of this power ultimately goes to heat. If we want the clock frequency to go faster, it is
//                      going to heat up the chip quadratically and run down the battery at a similarly enhanced rate. For this reason,
//                      the clock frequency will vary tremendously under load and in response to internal temperature. In extreme cases,
//                      the processor might just shut itself off to avoid melting.
//
//                  Because the clock frequency is constantly going up and down, benchmarking can be quite frustratingly error prone if
//                  you are trying to understand the microscopic flow of instructions through the processor using wall clock time. If
//                  you measure such things in cycles, then they become quite regular and repeatable because the instructions per second
//                  is variable but instructions per cycle is fixed. You will want to measure in cycles if your understanding of performance
//                  is measured in instructions that need to complete before the workload finishes. Let say you have designed a loop in
//                  your function to have 11 instructions in instruction issue port 0, 12 in port 1, 6 in port 5, and 4 each in ports 2,3
//                  and 6 and from this infer that the loop should take the longest of these, 12 cycles. Then you are thinking in instructions
//                  and will want to verify your work by measuring in cycles. If on the other hand, you just want your game to run fast,
//                  then you should use wall clock time. Different times; not linearly related. Choose the right one.
//
//                      It is not okay to measure the wall clock time and just multiply by the reported clock frequency to get cycles.
//                      This is almost always wrong. The clock frequency is often changing. Resist the temptation to find ways to lock
//                      the clock frequency in place. This is engineer thinking and isn't guaranteed solve the whole problem, in the
//                      face of a machine that nobody completely understands, even its designers. Instead, we will use error resistant
//                      statistical methods that will work even if things are not quite as we expect. This is the difference between
//                      scientists and engineers. Engineers think they know everything until they find out they don't -- if they are
//                      lucky. Scientists start with the presumption of ignorance and prove each fact as they go along.  Engineers do
//                      not develop good experimental skills.  They are better at reading specs, though.
//
//                  Once you have decided whether you want cycles or seconds, choose the appropriate low latency timer that gives you
//                  that.
//
//                  Note that cycle timers can involve quite arcane incantations to get to and some digging. You may have to resort
//                  to assembly language or calling obscure kernel APIs. Cycle counts and even instruction sets are often not reported
//                  so as to make it harder for competitors to understand how your product does what it does. While this might benefit
//                  the inexplicably spoiled shareholder, it is a problem for engineers in general. For this reason, even if they did
//                  not intend to become scientists and their training certainly doesn't help them succeed, in their race to cause
//                  problems for themselves in dubious service to the shareholder, engineers often /need/ to be scientists.
//
//              2) Use the right statistical measure
//
//                  Here again the right statistical measure varies according to what you want to know. In the above example of a carefully
//                  architected loop that finishes in 12 cycles, it is probably sufficient to measure the run time (using a cycle counter)
//                  a bunch of times and take the minimum time.  While most engineers will tell you this is unrepresentative because sometimes
//                  it takes longer or a lot longer than that and you will miss that information, the is an unsophisticated view of data
//                  use.  What we would be trying to confirm in this situation is whether our initial calculation that the loop should actually
//                  be 12 cycles in a perfect world is correct. If instead we measure it to be 15 cycles, then we have probably made a mistake
//                  either in measurement or our machine understanding.  It is important to understand and fix this mistake.  Adding in
//                  the contaminating times arising from preemption or perhaps cache misses as the unsophisticated mere engineer would have
//                  us do would only muddy this picture and make it harder to tell when we had succeeded in eliminating the error and may
//                  prevent us from proving our timing hypothesis correct, thereby preventing us from moving forward from a solid point of
//                  understanding. Since the scientists would want to know what the speed of light is for this function, we measure the
//                  minimum time as a direct measure of speed of light performance. (One lesson in science is to always prefer direct measures
//                  to indirect measures.) Generally speaking, if you are using cycle times, then you probably are also looking at minimum
//                  times, since cycle times generally mean we are asking experimentally questions above machine architecture, and the
//                  machine we may surmise should be fairly fixed and reproducible in its behavior. It can't run faster than its architecture, so
//                  a minimum time is good enough. Cycle counters and minimum times to filter out interference from clock slewing and
//                  preemptive multitasking respectively.
//
//                  If you are looking at just making code run fast (without taking time to infer the microarchitecture of the machine)
//                  -- this may even be necessary in the 12 cycle loop example if the code runs much slower than than expected because the
//                  data does not fit in the L1 cache, meaning the CPU core is not the bottleneck, and we have no model for cache operation --
//                  then we are probably looking at using some sort of average wall clock time.  There remains a problem with this though
//                  in that if we run the same experiment a bunch of times, we may find we get 10 times all around 30 microseconds and one
//                  at half a second. While a human will quickly see the discrepancy and ignore the 0.5s time, it will totally perturb the
//                  results from a simple dumb average such that our mean time is off by more than a factor of two! Worse for supporting a
//                  useful hypothesis (e.g. the code change I made make it faster / not faster), if we repeat the whole series of runs again
//                  wont be the same.  It is very hard to optimize code when the benchmark loop jumps up and down by a factor of two randomly!
//                  You will never know if your change actually improved things or not.  So we have to fix this somehow. A repeatable measure
//                  should be repeatable. That is if we run the benchmark app again once a day for a week, we should get substantially similar
//                  results each time. What to do?
//
//                  There are a couple of methods to do this. A statistician might point to something like a P-test to throw away the occasional
//                  squirrelly value. You can use this, but what if there are two of them? Another method might be to simply decide how accurate your
//                  results need to be and then run the experiment until such time that your estimate of the error in the mean is small
//                  enough that you have the accuracy you need.  That is, iterate until the Standard Deviation of the Mean (which is not
//                  the same thing as a the Standard Deviation) is small enough.
//
//                  Standard deviation and standard deviation of the mean have different formulas. If you iterate taking measurements
//                  forever, the standard deviation will not go down, but the standard deviation of the mean will. The standard deviation of
//                  the mean is also known as the standard error of the mean. It is a measure of how close the estimated mean is to the true mean,
//                  -- that is, the accuracy of the mean -- whereas the standard deviation is just how much noise is in the data relative to the mean.
//                  The standard error of the mean is what we want to know -- how accurate is this mean?
//
//                      https://en.wikipedia.org/wiki/Standard_error#Standard_error_of_the_sample_mean
//
//              3)  Measure for the right amount of time
//
//                  The operating system has habits. It will tend to interrupt you ever so often. If you design your single measurement
//                  to run much longer than that, then your likelihood of being preempted is quite high and all of your measurements are
//                  likely to be off. The error in your mean may take a long time to converge towards the desired confidence limit. If you
//                  measure too short a time, then clock latency is a problem. (This is the amount of time it takes to actually read the
//                  time from the clock.)  If you measure a work unit large enough that clock latency is comparatively tiny and it is
//                  still relatively unlikely to be preempted, then you have it about right.
//
//              4)  Putting it all together:
//
//                  If you want a quality benchmark, the kind an experimental scientist would design rather than a engineer would just
//                  throw together, then we will need to run some experiments to run the experiment. Don't worry, this part can still be
//                  automated. The fact that we need to do this should be no surprise to any experimentalist. Any measurement protocol
//                  needs to be debugged for sources of error and redesigned to eliminate them as much as possible. Only then is the
//                  experiment useful at returning accurate numbers instead of random numbers. If you are going to stake your professional
//                  reputation on being able to monitor performance regressions in your code or even optimize your code, some attention
//                  to this matter is due.
//
//                  a) measure system behavior
//
//                      Measure how often the system is likely to preempt your benchmark loop. Is there a fairly safe amount of time
//                      wherein one is likely to be preempted a maximum of once and most of the time, not at all?
//
//                  b) measure the clock latency.
//
//                      Measure how long it takes for the clock to measure time. This is systematic error in our practice.  We will want
//                      to subtract this time from all future measurements.
//
//                  b) Tailor your benchmark load to fit well with the benchmarking time window the system allows before preemption is
//                      likely.  What we are trying to do here is make the timing loop robust against a wide variety of code samples,
//                      from a tiny function like floor (a single instruction on some architectures!) out to a whole 1000x1000 matrix
//                      multiply.  For best accuracy, we want the benchmark load to be much larger than the clock latency, but still smaller
//                      than the preemption window such that it is unusual that preemption happens between when we measure start and
//                      stop times. We will adjust the workload to fit. Example:
//
//                          for( i = 0; i < testCount; i++ )
//                          {
//                              uint64_t startTime = ReadClock();
//                              for( k = 0; k < iterationCount; k++ )
//                                  floor(value);
//                              uint64_t endTime = ReadClock();
//                              uint64_t runTime = endTime - startTime - clockLatency;      // note: conceivably could be negative, so may need to handle that by removing the clock latency once later rather than every time as shown
//                              testTime[i] = (double) runTime / iterationCount;
//                          }
//
//                  c) Process the results
//
//                      This is likely either just the minimum value in testTime[], or you are going to calculate the mean and
//                      standard error of the mean in the testTime array. If the error is still too high -- lets say you need an
//                      error of no more than ±1% to tune your code, then if it is in excess of mean/100 then we have to go measure
//                      more times.  This will save a lot of time, because it allows you to get an accurate mean without resorting
//                      to pessimistic run times. Just run until it is accurate enough. A mere engineer would just run it a bajillion
//                      times just to be safe, when a few seconds would do.  We are not mere engineers. We are scientists! Respect
//                      understand and use the data.  Don't just report it like an engineer would.
//
//                      Measuring more times may seem difficult if testTime[] is a fixed sized array. However, in practice you
//                      don't need an array at all. If you are measuring minimum times, then you just need to keep a running
//                      minimum. If you are doing mean and standard error of the mean, then you just need to keep a running
//                      sum of all times, the running sum of all squared times, and the number of tests you ran. See
//                      https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance.  Note that the arguments here about
//                      numerical instability stem from a naive understanding of floating-point. We can actually do this without
//                      much trouble if we are just a little bit careful to understand when rounding is okay and when it is not
//                      and avoid it when it is not okay.  When we don't want rounding, we will just use large integers instead.
//                      How big do the integers have to be?
//
//                  d)  Draft reusable benchmark code you can use in other contexts.  Can we code this up as a functor:
//
//                          struct SampleFunctor{
//                              inline double operator(){ /* thing to be timed here */ };          // Called as:  SampleFunctor()
//                          };
//
//                          /*! @abstract Benchmark information we collected when BenchmarkFunctor was running */
//                          typedef struct Benchmark
//                          {
//                              double meanTime;                // mean time used for Functor()
//                              double stdDeviation;            // standard deviation
//                              double stdErrorOfTheMean;       // standard error of the mean.  THIS IS NOT THE STANDARD DEVIATION!
//                              double minimumTime;             // The minimum time the functor took to run
//                              double N;                       // The number of test measurements we made.  THis should probably be at least 10.
//                              ...
//                          }Benchmark;
//
//                          /*! @abstract Measure the time taken by Functor.operator().
//                           *  @param  f                   A functor in the style of sampleFunctor that contains the desired workload
//                           *  @param precisionRequired    How accurate the stdErrorOfTheMean needs to be, e.g. 0.01 for ±1% */
//                          template <typename Functor> Benchmark BenchmarkFunctor( const Functor & f,
//                                                                                  double precisionRequired );
//
//                      Please examine the output assembly to ensure that the Functor() wasn't optimized away. Take steps
//                      to ensure it is not.
//
//                  e)  Become Famous!
//
//                      Write this up with supporting data and publish it on github
//                      Make it work on windows too?
//
//              5) The distribution of times are not normally distributed.  Is it log-normal? Is it something else?
//                  How might this methodology be improved?
//
