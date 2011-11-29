/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  osal

FILE DESCRIPTION
VideoCore OS Abstraction Layer - vcos_assert support module.
=============================================================================*/

#include "interface/vcos/vcos.h"
#include <stdlib.h>

int vcos_verify_bkpts = 0;

int vcos_verify_bkpts_enabled(void)
{
   return vcos_verify_bkpts;
}

int vcos_verify_bkpts_enable(int enable)
{
   int old = vcos_verify_bkpts;
   vcos_verify_bkpts = enable;
   return old;
}

/**
  * Call the fatal error handler.
  */
void vcos_abort(void)
{
#ifdef __VIDEOCORE__
   _bkpt();
#endif

#if defined(VCOS_HAVE_BACKTRACE) && !defined(NDEBUG)
   vcos_backtrace_self();
#endif

   /* Insert chosen fatal error handler here */
#if defined __VIDEOCORE__ && !defined(NDEBUG)
   while(1); /* allow us to attach a debugger after the fact and see where we came from. */
#else
   abort(); /* on vc this ends up in _exit_halt which doesn't give us any stack backtrace */
#endif
}
