#!../../bin/linux-x86_64/pmtHv

## You may have to change pmtHv to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

#epicsEnvSet("EPICS_IOC_LOG_INET", "localhost")
#epicsEnvSet("EPICS_IOC_LOG_PORT", "6500")
#errlogInit2(204800, 2048)
#iocLogInit()
#eltc(0)     ## param = 1 make the program log to interactive shell.

## Register all support components
dbLoadDatabase "dbd/pmtHv.dbd"
pmtHv_registerRecordDeviceDriver pdbbase

## Set hv err check mask
# 5: ACerr & On, default is 5.
#var g_hvErrCheckMask 5

## Load record instances
#dbLoadRecords("db/pmt-hv.db","prefix=juno,configFilePath=file://./gcu1f3/xml_files/connections.xml,devName=GCU1F3_CH_0,uartNode=uart_1")
dbLoadTemplate("db/pmt-hv.substitutions")
dbLoadTemplate("db/pmt-gcu.substitutions")

callbackSetQueueSize(1000)
callbackParallelThreads(128, "")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=juno"
