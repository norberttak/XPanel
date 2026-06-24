/*
 * Copyright 2026 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/*
 * Stand-in for the MS CppUnitTest "CppUnitTest.h" header, used only by the
 * CMake/ctest build on macOS/Linux. This directory must come before any path
 * containing the real CppUnitTest.h on the include search path.
 *
 * Kept out of test/ itself so quote-includes ("CppUnitTest.h") in test_*.cpp
 * don't accidentally pick this up on Windows, where the real CppUnitTest.h
 * (from $(VCInstallDir)UnitTest\include) must keep being used.
 */
#include "../test_macros.h"
