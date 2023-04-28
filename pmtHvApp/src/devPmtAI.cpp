/* devPmtAI.cpp */
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
#include "aiRecord.h"
#include "epicsExport.h"
#include "callback.h"
#include "errlog.h"
#include "pmtRecordDriver.h"
#include "utility.h"
#include "epicsMath.h"

/*Create the dset for devPmtAI */
static long init(void*);
static long init_record(void*);
static long read_ai(void*);
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
}devPmtAI={
	6,
	NULL,
	init,
	init_record,
	NULL,
	read_ai,
	NULL,
};
epicsExportAddress(dset,devPmtAI);

static long add_record(struct dbCommon *precord);
static long del_record(struct dbCommon *precord);
static struct dsxt devPmtAiDsxt = {
	add_record,
	del_record
};

static long init(void* pass)
{
	long v = (long)pass;
	if(v == 0)
		devExtend(&devPmtAiDsxt);
	return 0;
}

static long init_record(void* pRecord)
{
    struct aiRecord	*pAi = (struct aiRecord	*)pRecord;
	if(pAi->inp.type == INST_IO)
	{
		CALLBACK* pcallback = (CALLBACK*)(calloc(1, sizeof(CALLBACK)));
		PmtRecordDriver* recordDriver = new PmtRecordDriver(pAi->inp.value.instio.string, pcallback);
		pAi->dpvt = recordDriver;
		// printf("Record inp: inst_io type, value: %s.\n", pAi->inp.value.instio.string);
	}
	else
	{
		recGblRecordError(S_db_badField, (void*)pAi, "devPmtAI (init_record) Illegal INP field.");
		return(S_db_badField);
	}
    return(2);
}

static long read_ai(void* pRecord)
{
    struct aiRecord	*pAi = (struct aiRecord	*)pRecord;
	PmtRecordDriver* recordDriver = (PmtRecordDriver*)pAi->dpvt;
	if(pAi->pact)
	{
		epicsFloat64 val = 0;
		bool ret = recordDriver->readValue(&val);
		if(ret)
		{
			pAi->val = val;
			pAi->udf = FALSE;
		}
		else
		{
			pAi->val = epicsNAN;
			//pAi->udf = TRUE;
		}
		
		// temp code for test performance
		if(pAi->hihi == 100)
		{
			std::thread::id thisid = std::this_thread::get_id();
			std::size_t idhash = std::hash<std::thread::id>{}(thisid);
			errlogPrintf("Thread: %lu, record name: %s, finished read data process at %s.\n", idhash, pAi->name, utility::makeCurrentTimeStr().c_str());
		}
	}
	else
	{
		// temp code for test performance
		if(pAi->hihi == 100)
		{
			std::thread::id thisid = std::this_thread::get_id();
			std::size_t idhash = std::hash<std::thread::id>{}(thisid);
			errlogPrintf("Thread: %lu, record name: %s, start read data callback at %s.\n", idhash, pAi->name, utility::makeCurrentTimeStr().c_str());
		}

		pAi->pact = TRUE;
		callbackRequestProcessCallbackDelayed(recordDriver->getCallback(), priorityMedium, pAi, 0.1);
	}
    return(2);
}

static long add_record(struct dbCommon *pRecord)
{
	errlogPrintf("Add AI record called.\n");
	return 0;
}

static long del_record(struct dbCommon *pRecord)
{
	errlogPrintf("Del AI record called.\n");
	struct aiRecord	*pAi = (struct aiRecord	*)pRecord;
	if(pAi->dpvt != NULL)
	{
		PmtRecordDriver* recordDriver = (PmtRecordDriver*)pAi->dpvt;
		CALLBACK* pcallback = recordDriver->getCallback();
		free(pcallback);
		delete recordDriver;
		pAi->dpvt = NULL;
	}
	return 0;
}
