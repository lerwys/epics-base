/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/* devMbboDirectSoftCallback.c */
/*
 *      Author:  Marty Kraimer
 *      Date:    04NOV2003
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alarm.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "mbboDirectRecord.h"
#include "epicsExport.h"

/* Create the dset for devMbboSoft */
static long init_record();
static long write_mbbo();
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_mbbo;
}devMbboDirectSoftCallback={
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_mbbo
};
epicsExportAddress(dset,devMbboDirectSoftCallback);

static void putCallback(struct link *plink)
{
    dbCommon *pdbCommon = (dbCommon *)plink->value.pv_link.precord;

    dbScanLock(pdbCommon);
    (*pdbCommon->rset->process)(pdbCommon);
    dbScanUnlock(pdbCommon);
}

static long init_record(mbboDirectRecord *pmbbo)
{
    long status = 0;
 
    /* dont convert */
    status = 2;
    return status;
 
} /* end init_record() */

static long write_mbbo(mbboDirectRecord	*pmbbo)
{
    struct link *plink = &pmbbo->out;
    long status;

    if(pmbbo->pact) return(0);
    if(plink->type!=CA_LINK) {
        status = dbPutLink(&pmbbo->out,DBR_USHORT,&pmbbo->val,1);
        return(status);
    }
    status = dbCaPutLinkCallback(plink,DBR_USHORT,&pmbbo->val,1,putCallback);
    if(status) {
        recGblSetSevr(pmbbo,LINK_ALARM,INVALID_ALARM);
        return(status);
    }
    pmbbo->pact = TRUE;
    return(0);
}
