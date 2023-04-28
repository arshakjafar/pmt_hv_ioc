#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "epicsTypes.h"

#include "pmtHvRecordDriver.h"
#include "pmtHvDevDriver.h"
#include "hvErrTimeStat.h"
#include <iostream>
using namespace std; 
PmtHvDevManager PmtHvRecordDriver::m_devManager;

PmtHvRecordDriver::PmtHvRecordDriver(const char* address, CALLBACK* pcallback)
{
    parseAddress(address);
    m_pCallback = pcallback;
    m_slopeParamInited = false;
    m_eslo = 1;
    m_eoff = 0;

    m_pHvErrTimeStat = NULL;
    if(m_param == "hvErrTime")
        m_pHvErrTimeStat = new HvErrTimeStat();
}

PmtHvRecordDriver::~PmtHvRecordDriver()
{
    if(m_pHvErrTimeStat != NULL)
    {
        delete m_pHvErrTimeStat;
        m_pHvErrTimeStat = NULL;
    }
}

bool PmtHvRecordDriver::isValidRecord()
{
    return !m_configFile.empty() && !m_devName.empty() && !m_param.empty() && m_pCallback != NULL;
}

CALLBACK* PmtHvRecordDriver::getCallback()
{
    return m_pCallback;
}

bool PmtHvRecordDriver::readValue(epicsFloat64* value)
{
    bool result = false;

    PmtHvDevDriver* devDriver = m_devManager.getDevDriver(m_configFile, m_devName, m_uartNode);
    if(devDriver != NULL)
    {
        if(!m_slopeParamInited)
            m_slopeParamInited = readAndCalcSlopeParam(&m_eslo, &m_eoff);

        if(m_slopeParamInited)
        {
            uint16_t val;
            if(m_param == "hvMes")
            {
                result = devDriver->readRegister(1, 3, &val);
            }
            else if(m_param == "hvSet")
            {
                result = devDriver->readRegister(1, 2, &val);
            }
            else if(m_param == "hvOn")
            {
                result = devDriver->readRegister(1, 1, &val);
                if(result)
                    val = (uint16_t)(val & 0x0001);
            }
            else if(m_param == "hvErrTime")
            {
                result = devDriver->readRegister(1, 1, &val);
                m_pHvErrTimeStat->updateCtrlStatValue(val, result);
                if(result)
                    *value = m_pHvErrTimeStat->getErrStateTimeLenSec();
                return result;
            }
            else if(m_param == "hvCtlStat")
            {
                result = devDriver->readRegister(1, 1, &val);
            }
            else if(m_param == "hvTemperature")
            {
                result = devDriver->readRegister(1, 36, &val);
            }
            
            if(result)
                *value = ((epicsFloat64)val) * m_eslo + m_eoff;
        }
    }
    return result;
}

bool PmtHvRecordDriver::readAndCalcSlopeParam(epicsFloat64* eslo, epicsFloat64* eoff)
{
    bool result = false;
    PmtHvDevDriver* devDriver = m_devManager.getDevDriver(m_configFile, m_devName, m_uartNode);
    if(devDriver != NULL)
    {
        if(m_param == "hvMes")
        {
            uint16_t val;
            result = devDriver->readRegister(1, 13, &val);
            if(result)
            {
                epicsFloat64 umesMax = (epicsFloat64)val;
                *eslo = umesMax / 4095;
                *eoff = 0;
            }
        }
        else if(m_param == "hvSet")
        {
            uint16_t min, max;
            result = devDriver->readRegister(1, 10, &min) && devDriver->readRegister(1, 11, &max);
            if(result)
            {
                epicsFloat64 umin = (epicsFloat64)min;
                epicsFloat64 umax = (epicsFloat64)max;
                *eslo = (umax - umin) / 4095;
                *eoff = umin;
            }
        }
        else if(m_param == "hvOn" || m_param == "hvErrTime" || m_param == "hvCtlStat")
        {
            result = true;
            *eslo = 1;
            *eoff = 0;
        }
        else if(m_param == "hvTemperature")
        {
	    *eslo = 0.488;
            *eoff = -273;
            result = true;
        }
    }
    return result;
}

bool PmtHvRecordDriver::writeValue(epicsFloat64 value)
{
    bool result = false;
    PmtHvDevDriver* devDriver = m_devManager.getDevDriver(m_configFile, m_devName, m_uartNode);
    if(devDriver != NULL)
    {
        if(!m_slopeParamInited)
            m_slopeParamInited = readAndCalcSlopeParam(&m_eslo, &m_eoff);

        if(m_slopeParamInited)
        {
            uint16_t val = (uint16_t)((value - m_eoff) / m_eslo);       // todo: math.round
            if(m_param == "hvSet")
            {
                result = devDriver->writeRegister(1, 2, val);
            }
            else if(m_param == "hvOn")
            {
                result = devDriver->writeRegister(1, 1, val);
            }
        }
    }
    return result;
}

void PmtHvRecordDriver::parseAddress(const char* address)
{
    std::vector<std::string> parts = splitString(address, ';');
    if(parts.size() == 4)
    {
        m_configFile = parts[0];
        m_devName = parts[1];
        m_uartNode = parts[2];
        m_param = parts[3];
    }
    else
    {
        m_configFile = "";
        m_devName = "";
        m_uartNode = "";
        m_param = "";
    }
}

std::vector<std::string> PmtHvRecordDriver::splitString(const std::string& s, char seperator)
{
    std::vector<std::string> strParts;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        strParts.push_back(substring);
        prev_pos = ++pos;
    }
    strParts.push_back(s.substr(prev_pos, pos - prev_pos));
    return strParts;
}
