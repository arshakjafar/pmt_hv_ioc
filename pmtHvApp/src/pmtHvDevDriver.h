#ifndef __PMT_HV_DEV_DRIVER_H_
#define __PMT_HV_DEV_DRIVER_H_

#include <string>
#include <mutex>

class PmtGcuDevDriver;

class PmtHvDevDriver
{
public:
    PmtHvDevDriver(PmtGcuDevDriver* gcuDriver, std::string uartNode);
    ~PmtHvDevDriver();

public:
    bool writeRegister(int cell, int subaddress, uint16_t value);
    bool readRegister(int cell, int subaddress, uint16_t* pvalue);
    
private:
    bool init();
    bool clear();

private:
    PmtGcuDevDriver* m_gcuDriver;
    std::string m_uartNode;
    std::mutex m_mutex;
    bool m_inited;
};

#endif
