/*
** LuaProfiler
** Copyright Kepler Project 2005-2007 (http://www.keplerproject.org/luaprofiler)
** $Id: function_meter.h,v 1.5 2007/08/22 19:23:53 carregal Exp $
*/

/*****************************************************************************
function_meter.c:
   Module to compute the times for functions (local times and total times)

Design:
   'lprofM_init'            set up the function times meter service
   'lprofM_enter_function'  called when the function stack increases one level
   'lprofM_leave_function'  called when the function stack decreases one level

   'lprofM_resume_function'   called when the profiler is returning from a time
                              consuming task
   'lprofM_resume_total_time' idem
   'lprofM_resume_local_time' called when a child function returns the execution 
                              to it's caller (current function)
   'lprofM_pause_function'    called when the profiler need to do things that
                              may take too long (writing a log, for example)
   'lprofM_pause_total_time'  idem
   'lprofM_pause_local_time'  called when the current function has called
                              another one or when the function terminates
*****************************************************************************/

#include "stack.h"


/* compute the local time for the current function */
void lprofM_pause_local_time(lprofP_STATE* S);

/* pause the total timer for all the functions that are in the stack */
void lprofM_pause_total_time(lprofP_STATE* S);

/* pause the local and total timers for all functions in the stack */
void lprofM_pause_function(lprofP_STATE* S);

/* resume the local timer for the current function */
void lprofM_resume_local_time(lprofP_STATE* S);

/* resume the total timer for all the functions in the stack */
void lprofM_resume_total_time(lprofP_STATE* S);

/* resume the local and total timers for all functions in the stack */
void lprofM_resume_function(lprofP_STATE* S);

/* the local time for the parent function is paused */
/* and the local and total time markers are started */
void lprofM_enter_function(lprofP_STATE* S, char *file_defined, char *fcn_name, long linedefined, long currentline);

/* computes times and remove the top of the stack         */
/* 'isto_resume' specifies if the parent function's timer */
/* should be restarted automatically. If it's false,      */
/* 'resume_local_time()' must be called when the resume   */
/* should be done                                         */
/* returns the funcinfo structure                         */
/* warning: use it before another call to this function,  */
/* because the funcinfo will be overwritten               */
lprofS_STACK_RECORD *lprofM_leave_function(lprofP_STATE* S, int isto_resume);

/* init stack */
lprofP_STATE* lprofM_init();
