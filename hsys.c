// $Id: hsys.c 5483 2009-10-12 05:49:53Z fish $
//
// $Log$
// Revision 1.9  2008/12/24 15:42:14  jj
// Add debug entry point for sclp event masks
//
// Revision 1.8  2008/12/22 13:10:22  jj
// Add sclp debug entry points
//
// Revision 1.7  2008/02/12 08:42:15  fish
// dyngui tweaks: new def devlist fmt, new debug_cd_cmd hook
//
// Revision 1.6  2006/12/08 09:43:28  jj
// Add CVS message log
//

#include "hstdinc.h"

#define _HSYS_C_

#include "hercules.h"

DLL_EXPORT SYSBLK sysblk;

#if defined(EXTERNALGUI)
DLL_EXPORT int extgui = 0;
#endif

#if defined(OPTION_W32_CTCI)
DLL_EXPORT int (*debug_tt32_stats)(int) = NULL;
DLL_EXPORT void (*debug_tt32_tracing)(int) = NULL;
#endif
#if defined(OPTION_DYNAMIC_LOAD)

DLL_EXPORT void *(*panel_command) (void *);
DLL_EXPORT void  (*panel_display) (void);
DLL_EXPORT void  (*daemon_task) (void);
DLL_EXPORT int   (*config_command) (int argc, char *argv[], char *cmdline);
DLL_EXPORT int   (*system_command) (int argc, char *argv[], char *cmdline);
DLL_EXPORT void *(*debug_cpu_state) (REGS *);
DLL_EXPORT void *(*debug_cd_cmd) (char *);
DLL_EXPORT void *(*debug_device_state) (DEVBLK *);
DLL_EXPORT void *(*debug_program_interrupt) (REGS *, int);
DLL_EXPORT void *(*debug_diagnose) (U32, int, int, REGS *);
DLL_EXPORT void *(*debug_iucv) (int, VADR, REGS *);
DLL_EXPORT void *(*debug_sclp_unknown_command) (U32, void *, REGS *);
DLL_EXPORT void *(*debug_sclp_unknown_event) (void *, void *, REGS *);
DLL_EXPORT void *(*debug_sclp_unknown_event_mask) (void *, void *, REGS *);
DLL_EXPORT void *(*debug_sclp_event_data) (void *, void *, REGS *);
DLL_EXPORT void *(*debug_chsc_unknown_request) (void *, void *, REGS *);
DLL_EXPORT void *(*debug_watchdog_signal) (REGS *);

#endif
