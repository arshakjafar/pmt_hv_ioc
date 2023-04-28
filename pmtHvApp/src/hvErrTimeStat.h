#ifndef __HV_ERR_TIME_STAT_H_
#define __HV_ERR_TIME_STAT_H_

#include "epicsTypes.h"
#include <chrono>

class HvErrTimeStat
{
public:
    HvErrTimeStat();

public:
    void updateCtrlStatValue(uint16_t ctrlStat, bool valid);
    epicsFloat64 getErrStateTimeLenSec();

private:
    bool isErrState(uint16_t ctrlStat);

public:
    //static uint16_t ErrCheckMask;

private:
    std::chrono::time_point<std::chrono::steady_clock> m_errStartTime;
    uint64_t m_errStateTimeLenSec;
    bool m_isInErrState;
};

#endif