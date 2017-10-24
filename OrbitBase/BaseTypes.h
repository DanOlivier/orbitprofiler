//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#if _WIN32||_WIN64
#define WIN32_LEAN_AND_MEAN
#include <BaseTsd.h>
#include <wtypes.h>
#else
#include <winpr/wtypes.h>
#endif
