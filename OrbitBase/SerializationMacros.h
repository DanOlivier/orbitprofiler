//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#define __STDC_FORMAT_MACROS // breakpad
#include <stdint.h>

#define ORBIT_SERIALIZABLE \
    template <class Archive> \
    void serialize(Archive& a_Archive, uint32_t const a_Version)