#pragma once
#include <stdexcept>

class StateLimitExceeded : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class TickLimitExceeded : public std::runtime_error {
    using std::runtime_error::runtime_error;
};