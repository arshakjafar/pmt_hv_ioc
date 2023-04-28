#include "pmtGcuDevDriver.h"
#include "uhal/uhal.hpp"
#include <thread>
#include <iostream>
#include "errlog.h"
#include "utility.h"

using namespace uhal;

#define COMMAND_SLEEP_TIME_MAX      (500000)
#define COMMAND_SLEEP_TIME_ONCE     (3000)

PmtGcuDevDriver::PmtGcuDevDriver(HwInterface* hw, std::string devName)
{
    m_hw = hw;
    m_devName = devName;
}

PmtGcuDevDriver::~PmtGcuDevDriver()
{
    if(m_hw != NULL)
        delete m_hw;
}

bool PmtGcuDevDriver::writeNode(std::string nodeName, uint32_t value)
{
    bool result = false;
    if(m_hw != NULL)
    {
        std::thread::id tid = std::this_thread::get_id();
        std::size_t tidhash = std::hash<std::thread::id>{}(tid);
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            m_hw->getNode(nodeName).write(value);
            m_hw->dispatch();
            result = true;
            errlogPrintf("Thread: %lu, writeNode finished at %s, succeed: %d, dev: %s, node: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), nodeName.c_str());
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Thread: %lu, writeNode failed, dev name: %s, node name: %s, exception: %s.\n", tidhash, m_devName.c_str(), nodeName.c_str(), e.what());
        }
    }
    return result;
}

bool PmtGcuDevDriver::readNode(std::string nodeName, uint32_t* pvalue)
{
    bool result = false;
    if(m_hw != NULL)
    {
        std::thread::id tid = std::this_thread::get_id();
        std::size_t tidhash = std::hash<std::thread::id>{}(tid);
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        try
        {
            ValWord<uint32_t> mem = m_hw->getNode(nodeName).read();
            m_hw->dispatch();
            *pvalue = mem.value();
            result = true;
            errlogPrintf("Thread: %lu, readNode finished at %s, succeed: %d, dev: %s, node: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), nodeName.c_str());
        }
        catch(const std::exception& e)
        {
            errlogPrintf("Thread: %lu, readNode failed, dev name: %s, node name: %s, exception: %s.\n", tidhash, m_devName.c_str(), nodeName.c_str(), e.what());
        }
    }
    return result;
}

bool PmtGcuDevDriver::writeRegister(std::string uartNode, int cell, int subaddress, uint16_t value)
{
    bool result = false;

    if(m_hw != NULL)
    {
        char tx[16];
        tx[0] = 'w';
        sprintf(&tx[1], "%02x", cell);
        sprintf(&tx[3], "%02x", subaddress);
        sprintf(&tx[5], "%04x", value);
        tx[9] = calcCRC(tx, 1, 8);
        tx[10] = '\n';
        tx[11] = 0;     // not send
        int txLen = 11;

        std::thread::id tid = std::this_thread::get_id();
        std::size_t tidhash = std::hash<std::thread::id>{}(tid);
        {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            // result = writeBuffer(uartNode, tx, txLen, true);
            result = writeBuffer(uartNode, tx, txLen, false);
            //errlogPrintf("Thread: %lu, writeRegister write buffer finished at %s, succeed: %d, dev: %s, uart: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), uartNode.c_str());
            if(result)
            {
                uint16_t valReadBack;
                bool desireCRC = true;
                // result = readBufferAndParse(uartNode, tx, txLen, desireCRC, &valReadBack, false);
                result = readBufferAndParse(uartNode, tx, 0, desireCRC, &valReadBack, false);
                errlogPrintf("Thread: %lu, writeRegister read buffer finished at %s, succeed: %d, dev: %s, uart: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), uartNode.c_str());
            }
            else
            {
                errlogPrintf("Thread: %lu, writeRegister write buffer failed, dev: %s, uart: %s.\n", tidhash, m_devName.c_str(), uartNode.c_str());
            }
            
        }
    }

    return result;
}

bool PmtGcuDevDriver::readRegister(std::string uartNode, int cell, int subaddress, uint16_t* pvalue)
{
    bool result = false;

    if(m_hw != NULL)
    {
        char tx[8];
        tx[0] = 'r';
        sprintf(&tx[1], "%02x", cell);
        sprintf(&tx[3], "%02x", subaddress);
        tx[5] = calcCRC(tx, 1, 4);
        tx[6] = '\n';
        tx[7] = 0;      // not send
        int txLen = 7;
	
        std::thread::id tid = std::this_thread::get_id();
        std::size_t tidhash = std::hash<std::thread::id>{}(tid);
        {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            result = writeBuffer(uartNode, tx, txLen, false);
            //errlogPrintf("Thread: %lu, readRegister write buffer finished at %s, succeed: %d, dev: %s, uart: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), uartNode.c_str());
            if(result)
            {
                bool desireCRC = true;
                // result = readBufferAndParse(uartNode, tx, txLen, desireCRC, pvalue, true);
                result = readBufferAndParse(uartNode, tx, 0, desireCRC, pvalue, true);
                errlogPrintf("Thread: %lu, readRegister read buffer finished at %s, succeed: %d, dev: %s, uart: %s.\n", tidhash, utility::makeCurrentTimeStr().c_str(), result, m_devName.c_str(), uartNode.c_str());
            }
            else
            {
                errlogPrintf("Thread: %lu, readRegister write buffer failed, dev: %s, uart: %s.\n", tidhash, m_devName.c_str(), uartNode.c_str());
            }
            
        }
    }

    return result;
}

bool PmtGcuDevDriver::writeBuffer(std::string uartNode, const char* tx, int len, bool checkEchoBackChar/*= false*/)
{
    bool result = false;
    if(m_hw != NULL)
    {
        result = !checkEchoBackChar;
        for(int i = 0; i < len; i++)
        {
            m_hw->getNode(uartNode + ".tx_data").write(int(tx[i]));
            m_hw->dispatch();

            if(checkEchoBackChar)
            {
                result = checkOneCharEchoBack(uartNode, tx[i]);
                if(!result)
                    break;
            }
        }
    }
    return result;
}

int PmtGcuDevDriver::readBuffer(std::string uartNode, int fifoLen, char* rx, int bufferLen)
{
    int dataLen = fifoLen;
    if(dataLen > bufferLen - 1)
        dataLen = bufferLen - 1;
    for(int i = 0; i < dataLen; i++)
    {
        ValWord<uint32_t> rx_data = m_hw->getNode(uartNode + ".rx_data").read();
	    m_hw->dispatch();
        rx[i] = char(rx_data.value() & 0xff);
    }
    rx[dataLen] = '\0';
    return dataLen + 1;
}

int PmtGcuDevDriver::readBufferSize(std::string uartNode)
{
    ValWord<uint32_t> fill = m_hw->getNode(uartNode + ".fifo.rx_fifo_fill").read();
    m_hw->dispatch();
    return (int)fill.value();
}

bool PmtGcuDevDriver::checkOneCharEchoBack(std::string uartNode, char txChar)
{
    bool result = false;
    int sleepTimeTotal = 0;
    do
    {
        int fifoLen = readBufferSize(uartNode);
        if(fifoLen >= 1)
        {
            result = checkEchoBack(uartNode, &txChar, 1);
            break;
        }

        if(sleepTimeTotal + COMMAND_SLEEP_TIME_ONCE < COMMAND_SLEEP_TIME_MAX)
        {
            usleep(COMMAND_SLEEP_TIME_ONCE);
            sleepTimeTotal += COMMAND_SLEEP_TIME_ONCE;
        }
        else
        {
            errlogPrintf("Wait too long time for check one char echo back, dev name: %s, uart node: %s, total wait time: %d ms.\n", m_devName.c_str(), uartNode.c_str(), sleepTimeTotal / 1000);
            break;
        }
    } while(sleepTimeTotal < COMMAND_SLEEP_TIME_MAX);
    return result;
}

bool PmtGcuDevDriver::readBufferAndParse(std::string uartNode, const char* tx, int txLen, bool desireCRC, uint16_t* pvalue, bool needCheckEchoBack)
{
    bool result = false;
    int desiredRetFifoLen = desireCRC ? 6 : 5;
    int sleepTimeTotal = 0;
    bool echoBackChecked = !needCheckEchoBack;
    do
    {
        int fifoLen = readBufferSize(uartNode);
        if(fifoLen >= txLen + desiredRetFifoLen && !echoBackChecked)
        {
            result = checkEchoBack(uartNode, tx, txLen);
            if(result)
                result = readBufferAndParse(uartNode, desireCRC, fifoLen - txLen, pvalue);
            break;
        }
        else if(fifoLen >= txLen && !echoBackChecked)
        {
            echoBackChecked = checkEchoBack(uartNode, tx, txLen);
            if(!echoBackChecked)
                break;
        }
        else if(fifoLen >= desiredRetFifoLen && echoBackChecked)
        {
            result = readBufferAndParse(uartNode, desireCRC, fifoLen, pvalue);
            break;
        }

        if(sleepTimeTotal + COMMAND_SLEEP_TIME_ONCE < COMMAND_SLEEP_TIME_MAX)
        {
            usleep(COMMAND_SLEEP_TIME_ONCE);
            sleepTimeTotal += COMMAND_SLEEP_TIME_ONCE;
        }
        else
        {
            errlogPrintf("Wait too long time for ret buffer, read failed, dev name: %s, uart node: %s, total wait time: %d ms.\n", m_devName.c_str(), uartNode.c_str(), sleepTimeTotal / 1000);
            clearWithoutLock(uartNode);
            break;
        }
    } while(sleepTimeTotal < COMMAND_SLEEP_TIME_MAX);
    return result;
}

bool PmtGcuDevDriver::checkEchoBack(std::string uartNode, const char* tx, int txLen)
{
    bool result = false;
    char rx[32];
    int rxLen = readBuffer(uartNode, txLen, rx, sizeof(rx));
    if(rxLen >= txLen)
    {
        bool equal = true;
        for(int i = 0; i < txLen; i++)
            if(rx[i] != tx[i])
            {
                equal = false;
                break;
            }
        if(!equal)
            errlogPrintf("Write command echo back check failed, dev name: %s, uart node: %s, tx: %s, tx hex str: %s, rx: %s, rx hex str: %s.\n", 
                m_devName.c_str(), uartNode.c_str(), tx, convertToHexString(tx, 0, txLen).c_str(), rx, convertToHexString(rx, 0, rxLen).c_str());
        result = equal;
    }
    return result;
}

bool PmtGcuDevDriver::readBufferAndParse(std::string uartNode, bool desireCRC, int fifoLen, uint16_t* pvalue)
{
    bool result = false;
    char rx[32];
    int rxLen = readBuffer(uartNode, fifoLen, rx, sizeof(rx));
	//printf("read buffer: %s.\n", rx);
    int retValueLen = desireCRC ? 6 : 5;
    if(rxLen >= retValueLen)
    {
        bool crcOK = true;
        if(desireCRC)
        {
            char crcCode = calcCRC(rx, 0, 4);
            crcOK = (crcCode == rx[4]);
        }

        if(crcOK)
        {
            std::string valueStr(rx);
            valueStr = valueStr.substr(0, 4);
            try
            {
                int val = std::stoi(valueStr, 0, 16);
                *pvalue = (uint16_t)val;
                result = true;
            }
            catch(const std::exception& e)
            {
                errlogPrintf("Parse value exception, dev name: %s, uart node: %s, value str: %s, value hex str: %s.\n", m_devName.c_str(), uartNode.c_str(), valueStr.c_str(), convertToHexString(valueStr).c_str());
            }
        }
        else
        {
            errlogPrintf("CRC failed, dev name: %s, uart node: %s, buffer str: %s.\n", m_devName.c_str(), uartNode.c_str(), rx);
        }
    }
    return result;
}

void PmtGcuDevDriver::init(std::string uartNode)
{
    if(m_hw != NULL)
    {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        m_hw->getNode( uartNode + ".setup.clks" ).write(542);
        m_hw->dispatch();
        m_hw->getNode( uartNode + ".setup.parity" ).write(0x0);
        m_hw->dispatch();
        m_hw->getNode( uartNode + ".setup.stop" ).write(0);
        m_hw->dispatch();
        m_hw->getNode( uartNode + ".setup.word" ).write(0x0);
        m_hw->dispatch();
        m_hw->getNode( uartNode + ".rx_data.rx_reset" ).write(1);
        m_hw->dispatch();
        m_hw->getNode( uartNode + ".rx_data.clear" ).write(0x3);
        m_hw->dispatch();

        usleep(100000);
    }
}

void PmtGcuDevDriver::clear(std::string uartNode)
{
    if(m_hw != NULL)
    {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        m_hw->getNode( uartNode + ".rx_data.clear" ).write(0x3);
        m_hw->dispatch();

        usleep(100000);
    }
}

void PmtGcuDevDriver::clearWithoutLock(std::string uartNode)
{
    if(m_hw != NULL)
    {
        m_hw->getNode( uartNode + ".rx_data.clear" ).write(0x3);
        m_hw->dispatch();
    }
}

std::string PmtGcuDevDriver::getDevName()
{
    return m_devName;
}

char PmtGcuDevDriver::calcCRC(const char* buf, int startPos, int len)
{
    char code = '0';
    try
    {
        int sum = 0;
        for(int i = 0; i < len; i++)
        {
            char c = buf[startPos + i];
            if(c >= '0' && c <= '9')
                sum += c - '0';
            else if(c >= 'a' && c <= 'f')
                sum += 10 + (c - 'a');
            else if(c >= 'A' && c <= 'F')
                sum += 10 + (c = 'A');
        }
        sum = ~sum;
        sum = (sum & 0x0f);
        code = sum >= 10 ? 'a' + sum - 10 : '0' + sum;
    }
    catch(const std::exception& e)
    {
        errlogPrintf("Calc CRC failed, dev name: %s, buffer str: %s, CRC part hex str: %s, exception: %s.\n", m_devName.c_str(), buf, convertToHexString(buf, startPos, len).c_str(), e.what());
    }
    
    return code;
}

std::string PmtGcuDevDriver::convertToHexString(const char* data, int startPos, int len)
{
    char buf[512];
    memset(buf, 0, sizeof(buf));
    buf[0] = '0';
    buf[1] = 'x';
    for(int i = 0; i < len; i++)
    {
        std::snprintf(buf + 2 + i * 3, sizeof(buf) - (2 + i * 3), "%02X,", (unsigned char)data[startPos + i]);
    }
    if(len > 0)
        buf[2 + len * 3 - 1] = 0;
    return std::string(buf);
}

std::string PmtGcuDevDriver::convertToHexString(std::string str)
{
    return convertToHexString(str.c_str(), 0, str.length());
}
