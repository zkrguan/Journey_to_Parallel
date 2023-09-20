// GPU621_W1_F2023_By_Zhaokai_Guan.cpp : This file contains the 'main' function. Program execution begins and ends there.
 // Workshop 1 - Platforms and Optimizations
 // w1.cpp

/*
* 1. Intel Compiler was working without any issues while doing this test. 
* 
* 2. !!!MS Visual C++ !!!! Caused something 
* Severity	Code	Description	Project	File	Line	Suppression State
* Error	D8016	'/Ox' and '/RTC1' command-line options are incompatible
* 
* Solution:
* Temporarily disbale the run time type checking. 
* right click project -> properties -> C/C++ -> Code generation -> Basic Runtime Checks -> Default
* 
3. G++, I used WSL and tried the following commands. Pretty basic commands, just in case forget it in the future. 
* g++ -OO -o test source.cpp
* ./test 5000000

*/


#include <iostream>
#include <cmath>
#include <chrono>
using namespace std::chrono;

class Version {
public:
    void operator()() const {
        std::cout << "Hello from the ";
#if defined(__GNUC__)           // for GCC 9.1.0
        std::cout << "G++ compiler: ";   // Insert compiler name
        std::cout << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl; // Insert version.revision.patch 

#elif defined(__INTEL_LLVM_COMPILER)         // for Intel Parallel Studio
        std::cout << "Intel DPC++ compiler: ";   // Insert compiler name
        std::cout << __INTEL_LLVM_COMPILER<<std::endl; // Insert version.revision.patch 

#elif defined(_MSC_VER)         // for Visual Studio
        std::cout << "MS Visual C++ compiler: ";   // Insert compiler name
        std::cout << _MSC_FULL_VER <<std::endl; // Insert version.revision.patch 

#else
        std::cout << "What compiler are you using there? Not those 3 above though.: ";   // None of the above
#endif
    }
};

// report system time
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

double magnitude(const double* x, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += x[i] * x[i];
    return sqrt(sum);
}

int main(int argc, char* argv[]) {
    Version version;
    version();
    if (argc != 2) {
        std::cerr << argv[0] << ": invalid number of arguments\n";
        std::cerr << "Usage: " << argv[0] << "  no_of_elements\n";
        return 1;
    }
    int n = std::atoi(argv[1]); // number of elements in a 
    steady_clock::time_point ts, te;

    // allocate memory
    ts = steady_clock::now();
    double* a = new double[n];

    // populate vector a
    for (int i = 0; i < n; i++)
        a[i] = 1.0;
    te = steady_clock::now();
    reportTime(" - allocation and initialization", te - ts);

    // determine magnitude
    ts = steady_clock::now();
    double length = magnitude(a, n);
    te = steady_clock::now();
    reportTime(" - magnitude calculation", te - ts);

    // display result
    std::cout << "Magnitude of a[" << n << "] = " << length << std::endl;

    // deallocate host memory
    delete[] a;
}