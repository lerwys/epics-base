/* devEventTestIoEvent.c */
/* share/src/dev $Id$ */

/* devEventTestIoEvent.c - Device Support Routines for ioEvent*/
/*
 *      Author:  	Marty Kraimer
 *      Date:           01/09/92
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
 * .00  12-13-91        jba     Initial definition
 * .02	03-13-92	jba	ANSI C changes
 *      ...
 */

#include	<vxWorks.h>
#include	<types.h>
#include	<stdioLib.h>
#include	<wdLib.h>
#include	<string.h>

#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<dbScan.h>
#include	<recSup.h>
#include	<devSup.h>
#include	<eventRecord.h>
/* Create the dset for devEventTestIoEvent */
long init();
long get_ioint_info();
long read_event();
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_event;
	DEVSUPFUN	special_linconv;
}devEventTestIoEvent={
	6,
	NULL,
	init,
	NULL,
	get_ioint_info,
	read_event,
	NULL};

IOSCANPVT ioscanpvt;
WDOG_ID wd_id=NULL;


static long init()
{
    scanIoInit(&ioscanpvt);
    return(0);
}


static long get_ioint_info(
	int 			*cmd,
	struct eventRecord 	*pr,
	IOSCANPVT		*ppvt)
{

    *ppvt = ioscanpvt;
    return(0);
}

static long read_event(pevent)
    struct eventRecord	*pevent;
{
	int	wait_time;

	wait_time = (int)(pevent->proc * vxTicksPerSecond);
	if(wait_time<=0) return(0);
	pevent->udf = FALSE;
	if(wd_id==NULL) wd_id = wdCreate();
	printf("%s Requesting Next ioEnevt\n",pevent->name);
	wdStart(wd_id,wait_time,scanIoRequest,(int)ioscanpvt);
	return(0);
}
