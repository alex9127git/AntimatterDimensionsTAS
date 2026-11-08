#pragma once
#include <chrono>


class Timer {
    private:
        std::chrono::time_point<std::chrono::steady_clock> start;

    public:
        Timer() : start(std::chrono::steady_clock::now()) {}

        double reset() {
            double result = peek();
            start = std::chrono::steady_clock::now();
            return result;
        }

        double peek() const {
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            return elapsed.count();
        }
};
