#pragma once

#include <chrono>

class Profiler {
public:
    Profiler(std::chrono::duration<double>& duration) 
        : duration_(duration), start_(std::chrono::high_resolution_clock::now()) {}

    ~Profiler() {
        duration_ += std::chrono::high_resolution_clock::now() - start_;
    }

private:
    std::chrono::duration<double>& duration_;
    std::chrono::high_resolution_clock::time_point start_;
};
