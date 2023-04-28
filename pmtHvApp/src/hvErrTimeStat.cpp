#include "hvErrTimeStat.h"
#include <epicsExport.h>

//uint16_t HvErrTimeStat::ErrCheckMask = 0x05;     // 5: ACerr & On
int g_hvErrCheckMask = 0x05; // 5: ACerr & On
epicsExportAddress(int, g_hvErrCheckMask);

HvErrTimeStat::HvErrTimeStat()
{
    m_isInErrState = false;
    m_errStateTimeLenSec = 0;
    m_errStartTime = std::chrono::steady_clock::now();
}

void HvErrTimeStat::updateCtrlStatValue(uint16_t ctrlStat, bool valid)
{
    if(valid)
    {
        if(isErrState(ctrlStat))
        {
            if(m_isInErrState)
            {
                // err state continous.
                std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
                m_errStateTimeLenSec = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_errStartTime).count();
            }
            else
            {
                // err state started.
                m_isInErrState = true;
                m_errStateTimeLenSec = 0;
                m_errStartTime = std::chrono::steady_clock::now();
            }
        }
        else
        {
            m_isInErrState = false;
            m_errStateTimeLenSec = 0;
        }
    }
    else
    {
        // reset err state when data is invalid.
        m_isInErrState = false;
        m_errStateTimeLenSec = 0;
    }
}

epicsFloat64 HvErrTimeStat::getErrStateTimeLenSec()
{
    return (epicsFloat64)m_errStateTimeLenSec;
}

bool HvErrTimeStat::isErrState(uint16_t ctrlStat)
{
    return (ctrlStat & (uint16_t)g_hvErrCheckMask) == (uint16_t)g_hvErrCheckMask;
}
