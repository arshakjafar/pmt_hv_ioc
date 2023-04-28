#ifndef __PMT_GCU_DEV_MANAGER_H_
#define __PMT_GCU_DEV_MANAGER_H_

#include <map>
#include <string>
#include <mutex>

class PmtGcuDevDriver;

class PmtGcuDevManager
{
public:
    static PmtGcuDevManager* GetInstance();
    ~PmtGcuDevManager();

private:
    PmtGcuDevManager();

public:
    PmtGcuDevDriver* getGcuDevDriver(std::string configFile, std::string devName);

private:
    static PmtGcuDevManager m_instance;
    std::map<std::string, PmtGcuDevDriver*> m_gcuDevMap;
    std::mutex m_mutex;
};

#endif