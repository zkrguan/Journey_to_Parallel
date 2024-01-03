#include <functional>
#include<tbb/tbb.h>
#pragma once


#ifndef TBB

class WordCount {
    const char* sourceString;
    int* sizeArray;
    int* numbArray;
    int characterCount;
    std::function<bool(char)> delimiterFunc;

public:
    WordCount(
        const char* const sourceString,
        int* sizeArray,
        int* numbArray,
        const int characterCount,
        std::function<bool(char)> delimiterFunc
    )
    {
        this->characterCount = characterCount;
        this->delimiterFunc = delimiterFunc;
        this->numbArray = numbArray;
        this->sizeArray = sizeArray;
        this->sourceString = sourceString;
    }

    void operator()(const int startingIndex, const int numberOfElement) const {
        for (int i = startingIndex; i < startingIndex + numberOfElement;) {
            if (!delimiterFunc(sourceString[i])) {
                int s = 0;
                while (i + s < characterCount && !delimiterFunc(sourceString[i + s])) {
                    ++s;
                }
                sizeArray[i] = s;
                int n = 0;
                for (int j = i + s + 1; j + s < characterCount; ++j) {
                    bool bad = false;
                    for (int k = 0; k < s && k + i < characterCount && k + j < characterCount; ++k) {
                        if (sourceString[i + k] != sourceString[j + k]) {
                            bad = true;
                            break;
                        }
                    }
                    if (!bad && delimiterFunc(sourceString[j + s])) {
                        ++n;
                    }
                }
                numbArray[i] = n;
                i += sizeArray[i];
            }
            else {
                sizeArray[i] = 0;
                numbArray[i] = 0;
                ++i;
            }
        }
    }

};
#endif // solution without TBB

#ifdef TBB
class WordCount {
    const char* sourceString;
    int* sizeArray;
    int* numbArray;
    int characterCount;
    std::function<bool(char)> delimiterFunc;

public:
    WordCount(
        const char* const sourceString,
        int* sizeArray,
        int* numbArray,
        const int characterCount,
        std::function<bool(char)> delimiterFunc
    )
    {
        this->characterCount = characterCount;
        this->delimiterFunc = delimiterFunc;
        this->numbArray = numbArray;
        this->sizeArray = sizeArray;
        this->sourceString = sourceString;
    }

    void operator()(const tbb::blocked_range<int>& r) const {
        for (int i = r.begin(); i < r.end();) {
            if (!delimiterFunc(sourceString[i])) {
                int s = 0;
                while (i + s < characterCount && !delimiterFunc(sourceString[i + s])) {
                    ++s;
                }
                sizeArray[i] = s;
                int n = 0;
                for (int j = i + s + 1; j + s < characterCount; ++j) {
                    bool bad = false;
                    for (int k = 0; k < s && k + i < characterCount && k + j < characterCount; ++k) {
                        if (sourceString[i + k] != sourceString[j + k]) {
                            bad = true;
                            break;
                        }
                    }
                    if (!bad && delimiterFunc(sourceString[j + s])) {
                        ++n;
                    }
                }
                numbArray[i] = n;
                i += sizeArray[i];
            }
            else {
                sizeArray[i] = 0;
                numbArray[i] = 0;
                ++i;
            }
        }
    }

};

#endif // TBB


