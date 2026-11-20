#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>

namespace orbit
{
constexpr int ticksPerQuarter = 960;
constexpr int divisions[] = { 2, 4, 8, 16 };

struct Layer
{
    int units = 8;       // Number of denominator notes in the cycle.
    int division = 8;    // Note denominator (8 means eighth notes).
    int hits = 4;        // Uniformly spaced clicks within the cycle.
};

inline int cycleTicks (Layer l) noexcept
{
    return std::clamp (l.units, 1, 32) * (4 * ticksPerQuarter / l.division);
}

inline double cycleQuarters (Layer l) noexcept
{
    return static_cast<double> (cycleTicks (l)) / ticksPerQuarter;
}

inline double hitInterval (Layer l) noexcept
{
    return cycleQuarters (l) / std::clamp (l.hits, 1, 16);
}

inline int64_t realignmentTicks (Layer a, Layer b) noexcept
{
    return std::lcm<int64_t> (cycleTicks (a), cycleTicks (b));
}

inline double positiveModulo (double x, double period) noexcept
{
    const auto r = std::fmod (x, period);
    return r < 0.0 ? r + period : r;
}

inline double phase (double quarterPosition, Layer l) noexcept
{
    return positiveModulo (quarterPosition, cycleQuarters (l)) / cycleQuarters (l);
}

// A hit belongs to the first sample whose beat interval contains the grid point.
// This also handles transport starting or seeking directly onto a grid point.
inline bool isHitAtSample (double quarterPosition, double quartersPerSample,
                           Layer l) noexcept
{
    const double interval = hitInterval (l);
    const double index = std::ceil ((quarterPosition - 1.0e-9) / interval);
    const double hit = index * interval;
    return hit >= quarterPosition - 1.0e-9
        && hit < quarterPosition + quartersPerSample - 1.0e-9;
}

inline double wrapLoop (double quarterPosition, double start, double end) noexcept
{
    if (end <= start || quarterPosition < end)
        return quarterPosition;
    return start + positiveModulo (quarterPosition - start, end - start);
}
}
