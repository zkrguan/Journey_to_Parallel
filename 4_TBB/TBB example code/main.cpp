/*
    Uncomment the logic you would like to test and keep the others commented out. 
*/

#define TBB_SETUP_TESTING
//#define SERIAL
//#define FUNCTOR
//#define TBB

//#define Q1_S
//#define Q1_T

//#define Q2_S
//#define Q2_T

#ifdef TBB_SETUP_TESTING
#include <tbb/tbb.h>
#include <iostream>
#include <vector>

// Functor for parallel accumulation
class AccumulateFunctor {
public:
    AccumulateFunctor(const std::vector<int>& data, int& result)
        : data(data), result(result) {}

    void operator()(const tbb::blocked_range<size_t>& range) const {
        for (size_t i = range.begin(); i < range.end(); ++i) {
            result += data[i];
        }
    }

private:
    const std::vector<int>& data;
    int& result;
};

int main() {
    const int size = 100;
    std::vector<int> data(size);

    // Initialize the data
    for (int i = 0; i < size; ++i) {
        data[i] = i;
    }

    int result = 0;

    // Using TBB parallel_for with AccumulateFunctor
    AccumulateFunctor accumulateFunctor(data, result);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, data.size()), accumulateFunctor);

    // Print the result
    std::cout << "Parallel Accumulation Result: " << result << std::endl;

    return 0;
}
#endif // TBB_SETUP_TESTING

#ifdef SERIAL
// Workshop 6 - Word Count Algorithm
// w6.serial.cpp
// Chris Szalwinski
// 2018/10/28

#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
using namespace std::chrono;

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {
    
    // I have to use microseconds otherwise the performance can not be compared!!!
    auto ms = duration_cast<microseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " microseconds" << std::endl;
}

// delimiter returns true if s is a delimiter
//
bool delimiter(char s) {
    return s == ' ' || s == '.' || s == ',' || s == '\n';
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0] << " filename\n";
        return 1;
    }

    // load text from file to string
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "*** Incorrect file name ***\n";
        std::cerr << "Filename " << argv[1] << " does not exist\n";
        return 2;
    }
    std::string str;
    file.seekg(0, std::ios::end);
    // request a change in capacity
    str.reserve((unsigned)file.tellg());
    file.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // allocate memory for analysis
    steady_clock::time_point ts, te;
    int len = (int)str.size();
    int* size = new int[len];
    int* numb = new int[len];

    ts = steady_clock::now();
    for (int i = 0; i < len; i++) {
        if (!delimiter(str[i])) {
            int s = 0;
            while (i + s < len && !delimiter(str[i + s])) s++;
            size[i] = s;
            int n = 0;
            for (int j = i + s + 1; j + s < len; j++) {
                bool bad = false;
                for (int k = 0;
                    k < s && k + i < len && k + j < len; k++) {
                    if (str[i + k] != str[j + k]) {
                        bad = true;
                        break;
                    }
                }
                if (!bad && delimiter(str[j + s])) n++;
            }
            numb[i] = n;
        }
        else {
            size[i] = 0;
            numb[i] = 0;
        }
        i += size[i];
    }
    te = steady_clock::now();

    // remove duplicate words
    for (int i = 0; i < len; i++) {
        if (size[i]) {
            for (int j = i + 1; j < len; j++) {
                if (size[j] == size[i]) {
                    bool duplicate = true;
                    for (int k = 0; k < size[i]; k++) {
                        if (str[i + k] != str[j + k]) {
                            duplicate = false;
                            break;
                        }
                    }
                    if (duplicate) {
                        numb[j] = 0;
                    }
                }
            }
        }
    }

    // determine longest word
    int maxsize = 0;
    int maxpos = 0;
    for (int i = 0; i < len; ++i) {
        if (size[i]) {
            if (numb[i] != 0) {
                std::cout << numb[i] << " * ";
                for (int j = 0; j < size[i]; ++j)
                    std::cout << str[i + j];
                std::cout << std::endl;
                if (size[i] > maxsize) {
                    maxsize = size[i];
                    maxpos = i;
                }
            }
            i += size[i];
        }
    }

    // report the longest word
    if (maxsize) {
        std::cout << "\nLongest Word\n";
        std::cout << numb[maxpos] << " * ";
        for (int k = 0; k < maxsize; k++)
            std::cout << str[maxpos + k];
        std::cout << std::endl;
    }
    else {
        std::cout << "no word repetitions found" << std::endl;
    }
    reportTime(argv[1], te - ts);

    // deallocate
    delete[] size;
    delete[] numb;
    std::cout << "Serial Solution" << std::endl;
}



#endif // MAIN

#ifdef FUNCTOR
// Workshop 6 - Word Count Algorithm
// w6.functor.cpp
// Chris Szalwinski
// 2018/10/28

#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include "WordCount.h"
using namespace std::chrono;

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {

    // I have to use microseconds otherwise the performance can not be compared!!!
    auto ms = duration_cast<microseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " microseconds" << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0] << " filename\n";
        return 1;
    }

    // load text from file to string
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "*** Incorrect file name ***\n";
        std::cerr << "Filename " << argv[1] << " does not exist\n";
        return 2;
    }
    std::string str;
    file.seekg(0, std::ios::end);
    // request a change in capacity
    str.reserve((unsigned)file.tellg());
    file.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // allocate memory for analysis
    steady_clock::time_point ts, te;
    int len = (int)str.size();
    int* size = new int[len];
    int* numb = new int[len];

    // define delimiters
    auto delimiter = [](char s) {
        return s == ' ' || s == '.' || s == ',' || s == '\n'; };

    ts = steady_clock::now();
    WordCount wordcount(str.c_str(), size, numb, len, delimiter);
    wordcount(0, len);
    te = steady_clock::now();

    // remove duplicate words
    for (int i = 0; i < len; i++) {
        if (size[i]) {
            for (int j = i + 1; j < len; j++) {
                if (size[j] == size[i]) {
                    bool duplicate = true;
                    for (int k = 0; k < size[i]; k++) {
                        if (str[i + k] != str[j + k]) {
                            duplicate = false;
                            break;
                        }
                    }
                    if (duplicate) {
                        numb[j] = 0;
                    }
                }
            }
        }
    }

    // determine longest word
    int maxsize = 0;
    int maxpos = 0;
    for (int i = 0; i < len; ++i) {
        if (size[i]) {
            if (numb[i] != 0) {
                std::cout << numb[i] << " * ";
                for (int j = 0; j < size[i]; ++j)
                    std::cout << str[i + j];
                std::cout << std::endl;
                if (size[i] > maxsize) {
                    maxsize = size[i];
                    maxpos = i;
                }
            }
            i += size[i];
        }
    }

    // report the longest word
    if (maxsize) {
        std::cout << "\nLongest Word\n";
        std::cout << numb[maxpos] << " * ";
        for (int k = 0; k < maxsize; k++)
            std::cout << str[maxpos + k];
        std::cout << std::endl;
    }
    else {
        std::cout << "no word repetitions found" << std::endl;
    }
    reportTime(argv[1], te - ts);

    // deallocate
    delete[] size;
    delete[] numb;
    std::cout << "Functor Solution" << std::endl;
}
#endif // FUNCTOR

#ifdef TBB
// Workshop 6 - Word Count Algorithm
 // w6.tbb.cpp
 // Chris Szalwinski
 // 2018/10/28

#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include "WordCount.h"
using namespace std::chrono;

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

int main(int argc, char** argv) {
    if (argc == 1 || argc > 3) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0] << " filename [grainsize]\n";
        return 1;
    }
    unsigned grainsize = 1;
    if (argc == 3) grainsize = (unsigned)atoi(argv[2]);

    // load text from file to string
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "*** Incorrect file name ***\n";
        std::cerr << "Filename " << argv[1] << " does not exist\n";
        return 2;
    }
    std::string str;
    file.seekg(0, std::ios::end);
    // request a change in capacity
    str.reserve((unsigned)file.tellg());
    file.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    // allocate memory for analysis
    steady_clock::time_point ts, te;
    int len = (int)str.size();
    int* size = new int[len];
    int* numb = new int[len];

    // define delimiters
    auto delimiter = [](char s) {
        return s == ' ' || s == '.' || s == ',' || s == '\n'; };

    ts = steady_clock::now();
    WordCount wordcount(str.c_str(), size, numb, len, delimiter);
    tbb::parallel_for(tbb::blocked_range<int>(0, len,grainsize),
        [&](const tbb::blocked_range<int>& r) {
            wordcount(r);
        });
    te = steady_clock::now();

    // remove duplicate words
    for (int i = 0; i < len; i++) {
        if (size[i]) {
            for (int j = i + 1; j < len; j++) {
                if (size[j] == size[i]) {
                    bool duplicate = true;
                    for (int k = 0; k < size[i]; k++) {
                        if (str[i + k] != str[j + k]) {
                            duplicate = false;
                            break;
                        }
                    }
                    if (duplicate) {
                        numb[j] = 0;
                    }
                }
            }
        }
    }

    // determine longest word
    int maxsize = 0;
    int maxpos = 0;
    for (int i = 0; i < len; ++i) {
        if (size[i]) {
            if (numb[i] != 0) {
                std::cout << numb[i] << " * ";
                for (int j = 0; j < size[i]; ++j)
                    std::cout << str[i + j];
                std::cout << std::endl;
                if (size[i] > maxsize) {
                    maxsize = size[i];
                    maxpos = i;
                }
            }
            i += size[i];
        }
    }

    // report the longest word
    if (maxsize) {
        std::cout << "\nLongest Word\n";
        std::cout << numb[maxpos] << " * ";
        for (int k = 0; k < maxsize; k++)
            std::cout << str[maxpos + k];
        std::cout << std::endl;
    }
    else {
        std::cout << "no word repetitions found" << std::endl;
    }
    reportTime(argv[1], te - ts);

    // deallocate
    delete[] size;
    delete[] numb;
    std::cout << "TBB Solution" << std::endl;

    std::cout<<"The grainsize is at "<< grainsize<< std::endl;
}

#endif // TBB

#ifdef Q1_S

#include <iostream>

#include <cctype>

int main() {

    char str[]{ "The Standard Template Library" };

    for (int i = 0; str[i] != '\0'; ++i)

        if (str[i] < 'R' && str[i] > 'C')

            str[i] = std::tolower(str[i]);

    std::cout << str << std::endl;
    return 0;
}
#endif // Q1

#ifdef Q1_T
#include <iostream>
#include <cctype>
#include <cstring>
#include <tbb/tbb.h>

int main() {
    char str[]{ "The Standard Template Library" };

    // I used blocked_range, lambda to implement the TBB parallel_for()
    // Missed that strlen so much from 1st year LOL
    tbb::blocked_range<size_t> range(0, std::strlen(str));

    // Declare the lambda function outside parallel_for for cleaner code
    auto convertToLower = [&str](const tbb::blocked_range<size_t>& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            if (str[i] > 'C' && str[i] < 'R') {
                str[i] = std::tolower(str[i]);
            }
        }
        };

    // Perform parallel conversion using the lambda function
    tbb::parallel_for(range, convertToLower);

    std::cout << str << std::endl;

    return 0;
}

#endif // Q1_T


#ifdef Q2_S

#include <iostream>

#include <cctype>

int main() {

    char str[]{ "The Standard Template Library" };

    for (int i = 0; str[i] != '\0'; ++i)

        if (str[i] < 'p' && str[i] > 'a')

            str[i] = std::toupper(str[i]);

    std::cout << str << std::endl;

}

#endif


#ifdef Q2_T

#include <iostream>
#include <cctype>
#include <cstring>
#include <tbb/tbb.h>

int main() {
    char str[]{ "The Standard Template Library" };
    tbb::blocked_range<size_t> range(0, std::strlen(str));
    auto convertToUpper = [&str](const tbb::blocked_range<size_t>& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            if (str[i] > 'a' && str[i] < 'p') {
                str[i] = std::toupper(str[i]);
            }
        }
    };
    tbb::parallel_for(range, convertToUpper);
    std::cout << str << std::endl;
    return 0;
}

#endif // Q2_T

