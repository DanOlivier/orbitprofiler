//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "LogInterface.h"
#include "Log.h"

using namespace std;

vector<string> LogInterface::GetOutput()
{
    return GLogger.ConsumeEntries( OrbitLog::Viz );
}