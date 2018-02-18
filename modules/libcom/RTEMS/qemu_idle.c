/*************************************************************************\
* Copyright (c) 2018 Michael Davidsaver
* EPICS BASE is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/
/* Hack to enable idle state power management.
 * Saves my laptop battery when running in QEMU.
 * Use in other situations is not supported!
 */

#include <stdio.h>

#if defined(__rtems__)

#include <rtems.h>
#include <rtems/error.h>

#endif /* __rtems__ */

#include <iocsh.h>
#include <epicsThread.h>

#if defined(__rtems__) && defined(__PPC__)
#define HAS_IDLER

#include <libcpu/spr.h>

// e500 specific version register
#ifndef SVR
#  define SVR 1023
#endif
#ifndef PVR
#  ifdef PPC_PVR
#    define PVR PPC_PVR
#  else
#    define PVR 287
#  endif
#endif

SPR_RW(HID0)
SPR_RO(PVR)
SPR_RO(SVR)

static
void qemu_e500_idle_thread(void *unused)
{
    while(1) {
        /* A bit of a hack here as RTEMS doesn't manage concurrent access to HID0.
         * So there is a possibility that our RMW will clobber concurrent changes.
         * However, I don't think there is other code making changes.
         */
        _write_HID0(_read_HID0() | HID0_DOZE);
        // MSR[POW] is the same bit as MSR[WE] in mpc8540 ref manual
        _write_MSR(_read_MSR() | MSR_POW);
    }
}

static
void qemu_idle_start(void)
{
    static unsigned once;
    if(once)
        return;
    once = 1;

    uint32_t pvr = _read_PVR();
    if(pvr>>16 == 0x8020) {
        // e500
        uint32_t svr = _read_SVR();
        switch(svr>>16) {
        case 0x8030:
            // mpc8540 (mvme3100)
            epicsThreadMustCreate("QIdLe",
                                  epicsThreadPriorityMin,
                                  epicsThreadGetStackSize(epicsThreadStackSmall),
                                  &qemu_e500_idle_thread, 0);
            break;
        default:
            break;
        }
    }
}

#endif /* __rtems__ && __PPC__ */

#ifdef HAS_IDLER

static const iocshArg * const qemu_idle_startArgs[0] = {};
static const iocshFuncDef qemu_idle_startFuncDef =
    {"qemu_idle_start", 0, qemu_idle_startArgs};
static void qemu_idle_startCallFunc(const iocshArgBuf *args)
{
    qemu_idle_start();
}

#endif /* HAS_IDLER */

void qemu_idle_register(void)
{
#ifdef HAS_IDLER
    iocshRegister(&qemu_idle_startFuncDef, qemu_idle_startCallFunc);
#endif /* HAS_IDLER */
}
