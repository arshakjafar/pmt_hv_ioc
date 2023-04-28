#ifndef __PMT_HV_DEV_MANAGER_H_
#define __PMT_HV_DEV_MANAGER_H_

#include <map>
#include <string>
#include <mutex>

class PmtHvDevDriver;
class PmtGcuDevDriver;

class PmtHvDevManager
{
public:
    PmtHvDevManager();
    ~PmtHvDevManager();

public:
    PmtHvDevDriver* getDevDriver(std::string configFile, std::string devName, std::string uartNode);

private:
    PmtGcuDevDriver* getGcuDevDriver(std::string configFile, std::string devName);

private:
    std::map<std::string, PmtHvDevDriver*> m_hvDevMap;
    std::mutex m_mutex;
};

#endif