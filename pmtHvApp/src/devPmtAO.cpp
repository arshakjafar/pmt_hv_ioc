/* devPmtAO.cpp */
/* PMT device support module */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <functional>

#include "alarm.h"
#include "cvtTable.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "link.h"
#include "aoRecord.h"
#include "epicsExport.h"
#include "callback.h"
#include "errlog.h"
#include "pmtRecordDriver.h"
#include "utility.h"
#include "epicsMath.h"

/*Create the dset for devPmtAI */
static long init(void*);
static long init_record(void*);
static long write_ao(void*);
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
}devPmtAO={
	6,
	NULL,
	init,
	init_record,
	NULL,
	write_ao,
	NULL,
};
epicsExportAddress(dset,devPmtAO);

static long add_record(struct dbCommon *precord);
static long del_record(struct dbCommon *precord);
static struct dsxt devPmtAoDsxt = {
	add_record,
	del_record
};

static long init(void* pass)
{
	long v = (long)pass;
	if(v == 0)
		devExtend(&devPmtAoDsxt);
	return 0;
}

static long init_record(void* pRecord)
{
    struct aoRecord	*pAo = (struct aoRecord	*)pRecord;
	if(pAo->out.type == INST_IO)
	{
		CALLBACK* pcallback = (CALLBACK*)(calloc(1, sizeof(CALLBACK)));
		PmtRecordDriver* recordDriver = new PmtRecordDriver(pAo->out.value.instio.string, pcallback);
		pAo->dpvt = recordDriver;
		//*************
		//printf("Record out: inst_io type, value: %s.\n", pAo->out.value.instio.string);
		epicsFloat64 val = 0;
		bool ret = recordDriver->readValue(&val);
		errlogPrintf("Record name: %s, Init read value: %f, succeed: %d.\n", pAo->name, val, ret);
		if(ret)
		{
			pAo->val = val;
			pAo->udf = FALSE;
		}
		else
		{
			pAo->val = std::numeric_limits<double>::quiet_NaN();
			pAo->udf = TRUE;
		}
		//*************
	
	
	
	}
	else
	{
		recGblRecordError(S_db_badField, (void*)pAo, "devPmtAO (init_record) Illegal INP field.");
		return(S_db_badField);
	}
    return(2);
}

// static long read_ai(void* pRecord)
// {
//     struct aiRecord	*pAi = (struct aiRecord	*)pRecord;
// 	PmtRecordDriver* recordDriver = (PmtRecordDriver*)pAi->dpvt;
// 	if(pAi->pact)
// 	{
// 		epicsFloat64 val = 0;
// 		bool ret = recordDriver->readValue(&val);
// 		if(ret)
// 		{
// 			pAi->val = val;
// 			pAi->udf = FALSE;
// 		}
// 		else
// 		{
// 			pAi->val = epicsNAN;
// 			//pAi->udf = TRUE;
// 		}
		
// 		// temp code for test performance
// 		if(pAi->hihi == 100)
// 		{
// 			std::thread::id thisid = std::this_thread::get_id();
// 			std::size_t idhash = std::hash<std::thread::id>{}(thisid);
// 			errlogPrintf("Thread: %lu, record name: %s, finished read data process at %s.\n", idhash, pAi->name, utility::makeCurrentTimeStr().c_str());
// 		}
// 	}
// 	else
// 	{
// 		// temp code for test performance
// 		if(pAi->hihi == 100)
// 		{
// 			std::thread::id thisid = std::this_thread::get_id();
// 			std::size_t idhash = std::hash<std::thread::id>{}(thisid);
// 			errlogPrintf("Thread: %lu, record name: %s, start read data callback at %s.\n", idhash, pAi->name, utility::makeCurrentTimeStr().c_str());
// 		}

// 		pAi->pact = TRUE;
// 		callbackRequestProcessCallbackDelayed(recordDriver->getCallback(), priorityMedium, pAi, 0.1);
// 	}
//     return(2);
// }

static long write_ao(void* pRecord)
{
    struct aoRecord	*pAo = (struct aoRecord	*)pRecord;
	PmtRecordDriver* recordDriver = (PmtRecordDriver*)pAo->dpvt;
	if(pAo->pact)
	{
		epicsFloat64 val = pAo->val;
		bool ret = recordDriver->writeValue(val);
		if(!ret)
		{
			pAo->val = std::numeric_limits<double>::quiet_NaN();
			pAo->udf = TRUE;
			errlogPrintf("Record name: %s, write value failed.\n", pAo->name);
		}
		return(ret ? 0 : 1);
	}
	else
	{
		pAo->pact = TRUE;
		callbackRequestProcessCallbackDelayed(recordDriver->getCallback(), priorityMedium, pAo, 0.1);
	}
    return(0);
}

static long add_record(struct dbCommon *pRecord)
{
	errlogPrintf("Add AO record called.\n");
	return 0;
}

static long del_record(struct dbCommon *pRecord)
{
	errlogPrintf("Del AO record called.\n");
	struct aoRecord	*pAo = (struct aoRecord	*)pRecord;
	if(pAo->dpvt != NULL)
	{
		PmtRecordDriver* recordDriver = (PmtRecordDriver*)pAo->dpvt;
		CALLBACK* pcallback = recordDriver->getCallback();
		free(pcallback);
		delete recordDriver;
		pAo->dpvt = NULL;
	}
	return 0;
}
