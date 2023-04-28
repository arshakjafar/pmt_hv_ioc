#include "errlog.h"

#include "pmtGcuDevManager.h"
#include "pmtGcuDevDriver.h"
#include "uhal/uhal.hpp"

using namespace uhal;

PmtGcuDevManager PmtGcuDevManager::m_instance;

PmtGcuDevManager::PmtGcuDevManager()
{

}

PmtGcuDevManager::~PmtGcuDevManager()
{
    for(auto iter = m_gcuDevMap.begin(); iter != m_gcuDevMap.end(); iter++)
    {
        if(iter->second != NULL)
            delete iter->second;
    }
    m_gcuDevMap.clear();
}

PmtGcuDevManager* PmtGcuDevManager::GetInstance()
{
    return &m_instance;
}

PmtGcuDevDriver* PmtGcuDevManager::getGcuDevDriver(std::string configFile, std::string devName)
{
    PmtGcuDevDriver* driver = NULL;
    std::string key = configFile + "$$" + devName;
    if(m_gcuDevMap.find(key) != m_gcuDevMap.end())
    {
        driver = m_gcuDevMap[key];
    }
    else
    {
        try
        {
            ConnectionManager manager(configFile);
            HwInterface* hw = new HwInterface(manager.getDevice(devName));
	    //A method to modify the timeout period for any pending or future transactions
	    //@param aTimeoutPeriod the desired timeout period in milliseconds
	    //hw->setTimeoutPeriod(61000);
            driver = new PmtGcuDevDriver(hw, devName);
            m_gcuDevMap[key] = driver;
        }
        catch(const std::exception& e)
        {
            //std::cerr << e.what() << '\n';
            errlogPrintf("Device connection failed, dev name: %s, exception: %s.\n", devName.c_str(), e.what());
        }
    }
    return driver;
}
