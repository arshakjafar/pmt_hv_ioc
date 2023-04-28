/* pmtHvMain.cpp */
/* Author:  Marty Kraimer Date:    17MAR2000 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "epicsExit.h"
#include "epicsThread.h"
#include "iocsh.h"

int main(int argc,char *argv[])
{
    if(argc>=2) {    
        iocsh(argv[1]);
        epicsThreadSleep(.2);
    }
    if(argc >= 3 && strcmp("-S", argv[2]) == 0) {
        // non-interactive IOC.  spin forever
        while(true) {
            epicsThreadSleep(1.0);
        }
    }
    else {
        iocsh(NULL);
    }
    epicsExit(0);
    return(0);
}
