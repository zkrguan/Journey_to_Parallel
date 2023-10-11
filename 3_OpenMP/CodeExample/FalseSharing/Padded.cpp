// Padded version:
// Convert the array into a two-dimensional one to ensure that each element is stored on separate cache line.
// You need to figure out how much the each of your L1 cache line's size. 
// My computer used I5 12500H 
// L1 Data Cache: 4 x 48 KB (12-way, 64-byte line) + 8 x 32 KB (8-way, 64-byte line)
// L1 Inst.Cache: 4 x 32 KB (8-way, 64-byte line) + 8 x 64 KB (8-way, 64-byte line)



/*
* 1 . Setup the openMP
* project property => C++/C => language => open MP support => yes
*
* 2. When compiler optimization switching to O(2), you need to disable RTC(runtime check)
* project property => C++/C => Code Generation => Basic Runtime Checks => Default
*
*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <omp.h>
using namespace std::chrono;

// I created this for recording each version of code's with different threads results. 


#define NUM_THREADS 6

// report system time
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

int main(int argc, char** argv) {
// Because my L1 line is 64 byte sized.
    const int CACHE_LINE_SIZE = 64;
    if (argc != 2) {
        std::cerr << argv[0] << ": invalid number of arguments\n";
        std::cerr << "Usage: " << argv[0] << "  no_of_slices\n";
        return 1;
    }
    // Setting how many threads will be used in the following code blocks //
    omp_set_num_threads(NUM_THREADS);
    // Moving the variables out of the parallel block. 
    double pi = 0.0;
    int n = std::atoi(argv[1]);
    double stepSize = 1.0 / (double)n;
    double sumArray[NUM_THREADS][CACHE_LINE_SIZE / 8]{ {} };
    int actualThreads{};
    std::cout << "I am trying to use " << NUM_THREADS << " threads to run this program parallel" << std::endl;
    // Performance time checker
    steady_clock::time_point ts, te;
    // calculate pi by integrating the area under 1/(1 + x^2) in n steps 
    ts = steady_clock::now();
#pragma omp parallel 
    {
        // Learned this way from Tim Mattson
        // The num of the threads you will be using inside the omp parallel block might be different than the 
        // Number you got from the computer. 
        actualThreads = omp_get_num_threads();
        int id = omp_get_thread_num();
        double x = 0.0;
        for (int i = id; i < n; i += actualThreads) {
            x = ((double)i + 0.5) * stepSize;
            sumArray[id][0] += 4.0 / (1.0 + x * x);
        }
    }

    for (int j = 0; j < actualThreads; ++j) {
        pi += sumArray[j][0] * stepSize;
    }
    te = steady_clock::now();

    std::cout << "n = " << n <<
        std::fixed << std::setprecision(15) <<
        "\n pi(exact) = " << 3.141592653589793 <<
        "\n pi(calcd) = " << pi << std::endl;
    reportTime("Integration", te - ts);
}