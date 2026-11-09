#pragma once
#include "constants.h"
#include "decimal.h"
#include <cstdint>
#include <cstddef>


inline constexpr size_t GROUP_THRESHOLD = 4096;
inline constexpr size_t PARALLEL_THRESHOLD = 1000;

inline constexpr double SAC_BONUS_EPSILON = 1e-4;

inline constexpr int64_t MAX_TICKS = 5000000;
inline constexpr size_t MAX_STATES = 800000;

inline const Decimal REFINEMENT_PRECISION = DC::D1 + Decimal(1e-9);
