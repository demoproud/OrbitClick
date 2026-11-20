#include "Timing.h"
#include <cmath>
#include <iostream>

#define CHECK(condition) do { if (! (condition)) { \
    std::cerr << "failed: " #condition " at line " << __LINE__ << '\n'; return 1; \
} } while (false)

int main()
{
    const orbit::Layer three { 8, 8, 3 };
    const orbit::Layer two { 8, 8, 2 };
    CHECK (orbit::realignmentTicks (three, two) == 4 * orbit::ticksPerQuarter);
    CHECK (std::abs (orbit::hitInterval (three) - 4.0 / 3.0) < 1.0e-12);
    CHECK (std::abs (orbit::hitInterval (two) - 2.0) < 1.0e-12);

    // A three-eighth-note cycle against a full 4/4 bar realigns at 12 beats.
    const orbit::Layer shortCycle { 3, 8, 1 };
    const orbit::Layer fullBar { 8, 8, 1 };
    CHECK (orbit::realignmentTicks (shortCycle, fullBar) == 12 * orbit::ticksPerQuarter);
    CHECK (std::abs (orbit::phase (10.5, shortCycle)) < 1.0e-12);

    const double sampleStep = 90.0 / (60.0 * 48000.0);
    CHECK (orbit::isHitAtSample (0.0, sampleStep, two));
    CHECK (orbit::isHitAtSample (6.0, sampleStep, two)); // start mid-song
    CHECK (! orbit::isHitAtSample (6.25, sampleStep, two)); // seek off-grid
    CHECK (orbit::isHitAtSample (2.0 - sampleStep / 2.0, sampleStep, two));
    const double fasterStep = 120.0 / (60.0 * 48000.0);
    CHECK (orbit::isHitAtSample (4.0 - fasterStep / 2.0, fasterStep, two)); // tempo changed
    CHECK (std::abs (orbit::wrapLoop (8.1, 4.0, 8.0) - 4.1) < 1.0e-12);
    CHECK (std::abs (orbit::wrapLoop (8.0, 4.0, 8.0) - 4.0) < 1.0e-12);
    std::cout << "timing checks passed\n";
}
