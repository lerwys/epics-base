/*************************************************************************\
* Copyright (c) 2008 UChicago Argonne LLC, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 *      Original Authors: Bob Dalesio and Marty Kraimer
 *      Date: 3/6/91
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alarm.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "epicsMath.h"
#include "recGbl.h"
#include "devSup.h"
#include "aiRecord.h"
#include "epicsExport.h"

/* Create the dset for devAiSoft */
static long init_record(aiRecord *prec);
static long read_ai(aiRecord *prec);

struct {
    long      number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN read_ai;
    DEVSUPFUN special_linconv;
} devAiSoft = {
    6,
    NULL,
    NULL,
    init_record,
    NULL,
    read_ai,
    NULL
};
epicsExportAddress(dset, devAiSoft);

static long init_record(aiRecord *prec)
{
    if (recGblInitConstantLink(&prec->inp, DBF_DOUBLE, &prec->val))
        prec->udf = FALSE;

    return 0;
}

static long readLocked(struct link *pinp, void *dummy)
{
    aiRecord *prec = (aiRecord *) pinp->precord;
    double val;
    long status = dbGetLink(pinp, DBR_DOUBLE, &val, 0, 0);

    if (status) return status;

    /* Apply smoothing algorithm */
    if (prec->smoo != 0.0 && prec->dpvt && finite(prec->val))
        prec->val = val * (1.00 - prec->smoo) + (prec->val * prec->smoo);
    else
        prec->val = val;

    prec->udf = FALSE;
    prec->dpvt = &devAiSoft;        /* Any non-zero value */

    if (dbLinkIsConstant(&prec->tsel) &&
        prec->tse == epicsTimeEventDeviceTime)
        dbGetTimeStamp(pinp, &prec->time);

    return 0;
}

static long read_ai(aiRecord *prec)
{
    long status;

    if (dbLinkIsConstant(&prec->inp))
        return 2;

    status = dbLinkDoLocked(&prec->inp, readLocked, NULL);
    if (status == S_db_noLSET)
        status = readLocked(&prec->inp, NULL);

    if (status)
        prec->dpvt = NULL;

    return 2;
}
