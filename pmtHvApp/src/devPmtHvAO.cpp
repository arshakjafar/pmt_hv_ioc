/* devPmtHvAO.cpp */
/* PMT-HV device support module */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits>

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
#include "pmtHvRecordDriver.h"

/*Create the dset for devPmtHvAO */
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
}devPmtHvAO={
	6,
	NULL,
	init,
	init_record,
	NULL,
	write_ao,
	NULL,
};
epicsExportAddress(dset,devPmtHvAO);

static long add_record(struct dbCommon *precord);
static long del_record(struct dbCommon *precord);
static struct dsxt devPmtHvAoDsxt = {
	add_record,
	del_record
};

static long init(void* pass)
{
	long v = (long)pass;
	if(v == 0)
		devExtend(&devPmtHvAoDsxt);
	return 0;
}

static long init_record(void* pRecord)
{
    struct aoRecord	*pAo = (struct aoRecord	*)pRecord;
	if(pAo->out.type == INST_IO)
	{
		CALLBACK* pcallback = (CALLBACK*)(calloc(1, sizeof(CALLBACK)));
		PmtHvRecordDriver* recordDriver = new PmtHvRecordDriver(pAo->out.value.instio.string, pcallback);
		pAo->dpvt = recordDriver;
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
	}
	else
	{
		recGblRecordError(S_db_badField, (void*)pAo, "devPmtHvAO (init_record) Illegal OUT field.");
		return(S_db_badField);
	}
    return(2);
}

static long write_ao(void* pRecord)
{
    struct aoRecord	*pAo = (struct aoRecord	*)pRecord;
	PmtHvRecordDriver* recordDriver = (PmtHvRecordDriver*)pAo->dpvt;
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
		PmtHvRecordDriver* recordDriver = (PmtHvRecordDriver*)pAo->dpvt;
		CALLBACK* pcallback = recordDriver->getCallback();
		free(pcallback);
		delete recordDriver;
		pAo->dpvt = NULL;
	}
	return 0;
}
