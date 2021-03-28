/*************************************************************************\
* Copyright (c) 2008 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* SPDX-License-Identifier: EPICS
* EPICS BASE is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution.
\*************************************************************************/

/*
 * Device support for EPICS time stamps
 *
 *   Original Author:   Eric Norum
 */

#include <stdlib.h>
#include <string.h>

#include "dbDefs.h"
#include "epicsTime.h"
#include "alarm.h"
#include "devSup.h"
#include "recGbl.h"

#include "dbAccess.h"
#include "dbCommon.h"
#include "aiRecord.h"
#include "stringinRecord.h"
#include "epicsExport.h"


/* Extended device support to allow INP field changes */

static long initAllow(int pass) {
    if (pass == 0) devExtend(&devSoft_DSXT);
    return 0;
}


/* ai record */

static long read_ai(aiRecord *prec)
{
    recGblGetTimeStamp(prec);
    prec->val = prec->time.secPastEpoch + (double)prec->time.nsec * 1e-9;
    prec->udf = FALSE;
    return 2;
}

static
aidset devTimestampAI = {
    {6, NULL, initAllow, NULL, NULL},
    read_ai,  NULL
};
epicsExportAddress(dset, devTimestampAI);


/* stringin record */

static long read_stringin (stringinRecord *prec)
{
    int len;

    recGblGetTimeStamp(prec);
    len = epicsTimeToStrftime(prec->val, sizeof prec->val,
                              prec->inp.value.instio.string, &prec->time);
    if (len >= sizeof prec->val) {
        prec->udf = TRUE;
        recGblSetSevr(prec, UDF_ALARM, prec->udfs);
        return -1;
    }
    prec->udf = FALSE;
    return 0;
}

static
stringindset devTimestampSI = {
    {5, NULL, initAllow, NULL, NULL},
    read_stringin
};
epicsExportAddress(dset, devTimestampSI);

static
long add_record_interval(dbCommon *prec)
{
    epicsTimeStamp *prev = malloc(sizeof(*prev));
    if(!prev) {
        return S_db_noMemory;
    } else {
        (void)epicsTimeGetCurrent(prev);
        prec->dpvt = prev;
        return 0;
    }
}

static
long del_record_interval(dbCommon *prec)
{
    free(prec->dpvt);
    prec->dpvt = NULL;
    return 0;
}

static
dsxt devTimeInterval_DSXT = {add_record_interval, del_record_interval};

static
long init_interval(int pass)
{
    if (pass == 0) devExtend(&devTimeInterval_DSXT);
    return 0;
}

static
long read_interval_ai(aiRecord *prec)
{
    epicsTimeStamp *prev = prec->dpvt;
    epicsTimeStamp now;
    if(!prev || epicsTimeGetCurrent(&now)) {
        recGblSetSevr(prec, READ_ALARM, INVALID_ALARM);
    } else {
        prec->val = epicsTimeDiffInSeconds(&now, prev);
        memcpy(prev, &now, sizeof(*prev));
    }
    return 2;
}

static
aidset devTimeIntervalAI = {
    {6, NULL, init_interval, NULL, NULL},
    read_interval_ai,  NULL
};
epicsExportAddress(dset, devTimeIntervalAI);
