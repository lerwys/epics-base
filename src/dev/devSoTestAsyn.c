/* devSoTestAsyn.c */
 /* share/src/dev   $Id$ */

/* devSoTestAsyn.c - Device Support Routines for testing asynchronous processing*/
/*
 *      Author:          Janet Anderson
 *      Date:            5-1-91
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  11-11-91        jba     Moved set of alarm stat and sevr to macros
 * .02  01-08-92        jba     Added cast in call to wdStart to avoid compile warning msg
 * .03  02-05-92	jba	Changed function arguments from paddr to precord 
 * .04	03-13-92	jba	ANSI C changes
 *      ...
 */


#include	<vxWorks.h>
#include	<types.h>
#include	<stdioLib.h>
#include	<wdLib.h>
#include	<memLib.h>
#include	<string.h>

#include	<alarm.h>
#include	<callback.h>
#include	<cvtTable.h>
#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<link.h>
#include	<dbCommon.h>
#include	<stringoutRecord.h>

/* Create the dset for devSoTestAsyn */
long init_record();
long write_stringout();
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_stringout;
	DEVSUPFUN	special_linconv;
}devSoTestAsyn={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_stringout,
	NULL};

/* control block for callback*/
struct callback {
        CALLBACK        callback;
        struct dbCommon *precord;
        WDOG_ID wd_id;
};

static void myCallback(pcallback)
    struct callback *pcallback;
{
    struct dbCommon *precord=pcallback->precord;
    struct rset     *prset=(struct rset *)(precord->rset);

    dbScanLock(precord);
    (*prset->process)(precord);
    dbScanUnlock(precord);
}
    

static long init_record(pstringout)
    struct stringoutRecord	*pstringout;
{
    char message[100];
    struct callback *pcallback;

    /* stringout.out must be a CONSTANT*/
    switch (pstringout->out.type) {
    case (CONSTANT) :
	pcallback = (struct callback *)(calloc(1,sizeof(struct callback)));
	pstringout->dpvt = (void *)pcallback;
	callbackSetCallback(myCallback,pcallback);
        pcallback->precord = (struct dbCommon *)pstringout;
	pcallback->wd_id = wdCreate();
	break;
    default :
	strcpy(message,pstringout->name);
	strcat(message,": devSoTestAsyn (init_record) Illegal OUT field");
	errMessage(S_db_badField,message);
	return(S_db_badField);
    }
    return(0);
}

static long write_stringout(pstringout)
    struct stringoutRecord	*pstringout;
{
    char message[100];
    struct callback *pcallback=(struct callback *)(pstringout->dpvt);
    int		wait_time;

    /* stringout.out must be a CONSTANT*/
    switch (pstringout->out.type) {
    case (CONSTANT) :
	if(pstringout->pact) {
		printf("%s Completed\n",pstringout->name);
		return(0); /* don`t convert*/
	} else {
		wait_time = (int)(pstringout->disv * vxTicksPerSecond);
		if(wait_time<=0) return(0);
		callbackSetPriority(pstringout->prio,pcallback);
		printf("%s Starting asynchronous processing\n",pstringout->name);
		wdStart(pcallback->wd_id,wait_time,callbackRequest,(int)pcallback);
		return(1);
	}
    default :
        if(recGblSetSevr(pstringout,SOFT_ALARM,VALID_ALARM)){
		if(pstringout->stat!=SOFT_ALARM) {
			strcpy(message,pstringout->name);
			strcat(message,": devSoTestAsyn (read_stringout) Illegal OUT field");
			errMessage(S_db_badField,message);
		}
	}
    }
    return(0);
}
