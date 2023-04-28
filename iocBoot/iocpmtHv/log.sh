#!/bin/sh
#
# System-V init script for the EPICS IOC Log Server.
#

INSTALL_BIN=$EPICS_BASE/bin/linux-x86_64

# To change the default values for the EPICS Environment parameters,
# uncomment and modify the relevant lines below.

EPICS_IOC_LOG_PORT="6500" export EPICS_IOC_LOG_PORT 
EPICS_IOC_LOG_FILE_NAME="/home/dcs/dcs3.0/IOC/Test/pmt-hv-ioc/iocBoot/iocpmtHv/hv.log" export EPICS_IOC_LOG_FILE_NAME
EPICS_IOC_LOG_FILE_LIMIT="1000000000" export EPICS_IOC_LOG_FILE_LIMIT

if [ $1 = "start" ]; then
    if [ -x $INSTALL_BIN/iocLogServer ]; then
        echo "Starting EPICS Log Server "
        $INSTALL_BIN/iocLogServer &
    fi
else
    if [ $1 = "stop" ]; then
        pid=`ps -e | sed -ne '/iocLogSe/s/^ *\([1-9][0-9]*\).*$/\1/p'`
        if [ "${pid}" != "" ]; then
            echo "Stopping EPICS Log Server "
            kill ${pid}
        fi
    fi
fi
