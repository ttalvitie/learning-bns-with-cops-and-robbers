#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <immintrin.h>

using namespace std;

function<void(ostream&)> failureContextPrint = [](ostream&) {};

struct ScopedFailureContextPrint {
    ScopedFailureContextPrint(function<void(ostream&)> func) {
        failureContextPrint = func;
    }
    ~ScopedFailureContextPrint() {
        failureContextPrint = [](ostream&) {};
    }
};

inline void check(
    bool condVal,
    const char* condStr,
    const char* condFile,
    int condLine
) {
    if(!condVal) {
        cerr << "FATAL ERROR " << condFile << ":" << condLine << ": ";
        cerr << "Condition '" << condStr << "' does not hold\n";
        failureContextPrint(cerr);
        abort();
    }
}

#define CHECK(condition) check((condition), #condition, __FILE__, __LINE__)

template <typename T>
inline T parseString(const string& str) {
    T ret;
    stringstream ss(str);
    ss >> ret;
    CHECK(!ss.fail());
    return ret;
}

class Clock {
public:
    Clock() {
        start_ = chrono::steady_clock::now();
    }

    double elapsedTime() const {
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        return 1e-9 * (double)chrono::duration_cast<chrono::nanoseconds>(end - start_).count();
    }

private:
    chrono::steady_clock::time_point start_;
};

template <typename T>
inline void hashCombine(size_t& x, const T& val) {
    x ^= hash<T>()(val) + 0x9e3779b9 + (x << 6) + (x >> 2);
}

namespace std {
    template <typename A, typename B>
    struct hash<pair<A, B>> {
        inline size_t operator()(const pair<A, B>& val) const {
            size_t x = 0;
            hashCombine(x, val.first);
            hashCombine(x, val.second);
            return x;
        }
    };
}
