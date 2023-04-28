#ifndef __PMT_GCU_DEV_DRIVER_H_
#define __PMT_GCU_DEV_DRIVER_H_

#include <string>
#include <mutex>

namespace uhal
{
    class HwInterface;
};

class PmtGcuDevDriver
{
public:
    PmtGcuDevDriver(uhal::HwInterface* hw, std::string devName);
    ~PmtGcuDevDriver();

public:
    bool writeNode(std::string nodeName, uint32_t value);
    bool readNode(std::string nodeName, uint32_t* pvalue);
    bool writeRegister(std::string uartNode, int cell, int subaddress, uint16_t value);
    bool readRegister(std::string uartNode, int cell, int subaddress, uint16_t* pvalue);
    void init(std::string uartNode);
    void clear(std::string uartNode);
    std::string getDevName();
    
private:
    bool writeBuffer(std::string uartNode, const char* tx, int len, bool checkEchoBackChar = false);
    int readBuffer(std::string uartNode, int fifoLen, char* rx, int bufferLen);
    int readBufferSize(std::string uartNode);
    bool checkOneCharEchoBack(std::string uartNode, char txChar);
    bool readBufferAndParse(std::string uartNode, const char* tx, int txLen, bool desireCRC, uint16_t* pvalue, bool needCheckEchoBack);
    bool checkEchoBack(std::string uartNode, const char* tx, int txLen);
    bool readBufferAndParse(std::string uartNode, bool desireCRC, int fifoLen, uint16_t* pvalue);
    char calcCRC(const char* buf, int startPos, int len);
    void clearWithoutLock(std::string uartNode);
    std::string convertToHexString(const char* data, int startPos, int len);
    std::string convertToHexString(std::string str);

private:
    uhal::HwInterface* m_hw;
    std::mutex m_mutex;
    std::string m_devName;
};

#endif
