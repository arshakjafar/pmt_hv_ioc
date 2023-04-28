#include "errlog.h"

#include "pmtHvDevDriver.h"
#include "pmtGcuDevDriver.h"

PmtHvDevDriver::PmtHvDevDriver(PmtGcuDevDriver* gcuDriver, std::string uartNode)
{
    m_gcuDriver = gcuDriver;
    m_uartNode = uartNode;
    m_inited = false;
}

PmtHvDevDriver::~PmtHvDevDriver()
{
}

bool PmtHvDevDriver::writeRegister(int cell, int subaddress, uint16_t value)
{
    bool result = false;

    if(m_gcuDriver != NULL)
    {
        if(!m_inited)
            m_inited = init();
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            if(m_inited)
                result = m_gcuDriver->writeRegister(m_uartNode, cell, subaddress, value);
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Write register exception, dev name: %s, uartNode: %s, cell: %d, subaddress: %d, exception: %s.\n", 
                m_gcuDriver->getDevName().c_str(), m_uartNode.c_str(), cell, subaddress, e.what());
            m_inited = false;
        }
    }

    return result;
}

bool PmtHvDevDriver::readRegister(int cell, int subaddress, uint16_t* pvalue)
{
    bool result = false;

    if(m_gcuDriver != NULL)
    {
    	//errlogPrintf("start read register, uartNode: %s.\n", m_uartNode.c_str());
        if(!m_inited)
            m_inited = init();
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            if(m_inited)
                result = m_gcuDriver->readRegister(m_uartNode, cell, subaddress, pvalue);
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Read register exception, dev name: %s, uartNode: %s, cell: %d, subaddress: %d, exception: %s.\n", 
                m_gcuDriver->getDevName().c_str(), m_uartNode.c_str(), cell, subaddress, e.what());
            m_inited = false;
        }
    }

    return result;
}

bool PmtHvDevDriver::init()
{
    bool inited = false;
    if(m_gcuDriver != NULL)
    {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            if(!m_inited)
                m_gcuDriver->init(m_uartNode);
            inited = true;
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Init exception, dev name: %s, uartNode: %s, exception: %s.\n", 
                m_gcuDriver->getDevName().c_str(), m_uartNode.c_str(), e.what());
        }
    }
    return inited;
}

bool PmtHvDevDriver::clear()
{
    bool result = false;
    if(m_gcuDriver != NULL)
    {
        if(!m_inited)
            m_inited = init();
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            if(m_inited)
            {
                m_gcuDriver->clear(m_uartNode);
                result = true;
            }
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Clear operation exception, dev name: %s, uartNode: %s, exception: %s.\n", 
                m_gcuDriver->getDevName().c_str(), m_uartNode.c_str(), e.what());
            m_inited = false;
        }
    }
    return result;
}
