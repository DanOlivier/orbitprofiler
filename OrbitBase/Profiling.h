//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#if _WIN32||_WIN64
#include <winstl/performance/performance_counter.hpp>

typedef winstl::performance_counter                 PerfCounter;
typedef winstl::performance_counter::interval_type  IntervalType;
typedef winstl::performance_counter::epoch_type     EpochType;

#else
#include <unixstl/performance/performance_counter.hpp>
#include <winpr/wtypes.h>

typedef unixstl::performance_counter                 PerfCounter;
typedef unixstl::performance_counter::interval_type  IntervalType;
typedef unixstl::performance_counter::epoch_type     EpochType;
#endif

