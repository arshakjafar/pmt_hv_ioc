#include "errlog.h"

#include "pmtHvDevManager.h"
#include "pmtHvDevDriver.h"
#include "pmtGcuDevDriver.h"
#include "pmtGcuDevManager.h"
#include "uhal/uhal.hpp"

using namespace uhal;

PmtHvDevManager::PmtHvDevManager()
{

}

PmtHvDevManager::~PmtHvDevManager()
{
    for(auto iter = m_hvDevMap.begin(); iter != m_hvDevMap.end(); iter++)
    {
        if(iter->second != NULL)
            delete iter->second;
    }
    m_hvDevMap.clear();
}

PmtHvDevDriver* PmtHvDevManager::getDevDriver(std::string configFile, std::string devName, std::string uartNode)
{
    PmtHvDevDriver* driver = NULL;
    std::lock_guard<std::mutex> lockGuard(m_mutex);
    std::string key = configFile + "$$" + devName + "$$" + uartNode;
    if(m_hvDevMap.find(key) != m_hvDevMap.end())
    {
        driver = m_hvDevMap[key];
    }
    else
    {
        PmtGcuDevDriver* gcuDriver = getGcuDevDriver(configFile, devName);
        if(gcuDriver != NULL)
        {
            driver = new PmtHvDevDriver(gcuDriver, uartNode);
            m_hvDevMap[key] = driver;
        }
    }
    return driver;
}

PmtGcuDevDriver* PmtHvDevManager::getGcuDevDriver(std::string configFile, std::string devName)
{
    PmtGcuDevDriver* driver = PmtGcuDevManager::GetInstance()->getGcuDevDriver(configFile, devName);
    return driver;
}
