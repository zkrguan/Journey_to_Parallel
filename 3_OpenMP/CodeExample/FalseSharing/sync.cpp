// using OpenMP sync tatics version:

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

#define NUM_THREADS 8

// report system time
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

int main(int argc, char** argv) {
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
    int actualThreads{};
    std::cout << "I am trying to use " << NUM_THREADS << " threads to run this program parallel" << std::endl;
    // Performance time checker
    steady_clock::time_point ts, te;
    // calculate pi by integrating the area under 1/(1 + x^2) in n steps 
    ts = steady_clock::now();
    #pragma omp parallel 
    {
        // You noticed that this version sum is inside the parallel block.
        // Because each thread will have its own version of the sum. 
        // Then inside the critial, that's where they put all the sum into the pi.
        double sum{};
        // Learned this way from Tim Mattson
        // The num of the threads you will be using inside the omp parallel block might be different than the 
        // Number you got from the computer. 
        actualThreads = omp_get_num_threads();
        int id = omp_get_thread_num();
        double x = 0.0;
        for (int i = id; i < n; i += actualThreads) {
            x = ((double)i + 0.5) * stepSize;
            sum += 4.0 / (1.0 + x * x);
        }
    #pragma omp critical
        pi += sum * stepSize;
    }
    te = steady_clock::now();

    std::cout << "n = " << n <<
        std::fixed << std::setprecision(15) <<
        "\n pi(exact) = " << 3.141592653589793 <<
        "\n pi(calcd) = " << pi << std::endl;
    reportTime("Integration", te - ts);
}