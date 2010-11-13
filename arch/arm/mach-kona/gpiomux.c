/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/



/****************************************************************************/
/**
*  @file    gpiomux.c
*
*  @brief   Lower level GPIOMUX utility. Note that critical section protection
*           is the responsibility of the higher level software.
*/
/****************************************************************************/


/* ---- Include Files ----------------------------------------------------- */

#include <mach/gpiomux_groups.h>

// SARU #include <csp/module.h>
#include <mach/gpiomux.h>

/* ---- Private Constants and Types ---------------------------------------- */

#define GMUX_DEBUG 0

#if GMUX_DEBUG
#if defined(__KERNEL__) && !defined(STANDALONE)
#define DBGPRINT(fmt,args...) printk("%s: " fmt,__FUNCTION__,##args)
#else
#include <csp/stdio.h>
#define DBGPRINT(fmt,args...) printf("%s: " fmt,__FUNCTION__,##args)
#endif /* defined(__KERNEL__) */
#else
#define DBGPRINT(level,fmt,args...)
#endif /* GMUX_DEBUG */

#if defined(__KERNEL__)

#define FORCE_CONFLICT_CHECK 1

#else

/*
 * For Linux, conflict checking is on by default. For csptest, we can turn it on
 * with this flag for local testing. Production csptest must turn
 * this flag off unless the registrations/deregistrations are balanced.
 */
#define FORCE_CONFLICT_CHECK 1

#endif


/* ---- Private Variables ------------------------------------------------ */
/* init called before other functions check */
static int early_initialized = 0;
static int initialized = 0;

/* global init structure */
static gpiomux_init_t gInit;

static unsigned int gGroupListEntries;
static gpiomux_group_t *gGroupList;

/* ---- Private Functions ------------------------------------------------- */
/* ---- Functions ----------------------------------------------------------*/

/****************************************************************************/
/**
*  @brief   gpiomux_dumpTables
*
*  Used to dump tables
*
*  @return
*/
/****************************************************************************/
void gpiomux_dumpTables(void)
{
   unsigned int groupidx;
   const gpiomux_group_t *groupp;

   /* Check for conflicts before assigning functions */
   for ( groupidx = 0; groupidx < gGroupListEntries; groupidx++ )
   {
		DBGPRINT("Group %d\n", groupidx);
		
      groupp = &gGroupList[groupidx];
      {
         unsigned int pinidx;
         for (pinidx = 0; pinidx < groupp->numPins; pinidx++  )
         {
				DBGPRINT("pin %d, function %d, type %d, pull %d, curr %d, slew %d\n", 
							groupp->listp[pinidx].pin, 
							groupp->listp[pinidx].function, 
							groupp->listp[pinidx].inputType, 
							groupp->listp[pinidx].pull, 
							groupp->listp[pinidx].curr, 
							groupp->listp[pinidx].slewRate);
         }
      }
   }
}


/****************************************************************************/
/**
*  @brief   gpiomux_earlyinit
*
*  Used to initialize the mux table and set all pin functions to GPIO.
*
*  @return
*     none
*/
/****************************************************************************/
void gpiomux_earlyinit
(
	const unsigned int groupListEntries,
	const gpiomux_group_t *groupList
)
{
	gGroupListEntries = (unsigned int)groupListEntries;
	gGroupList = (gpiomux_group_t *)groupList;

	if (groupListEntries && groupList) 
	{
		early_initialized = 1;
	}
}


/****************************************************************************/
/**
*  @brief   gpiomux_Init
*
*  Used to initialize the mux table and set all pin functions to GPIO.
*
*  @return
*     none
*/
/****************************************************************************/
void gpiomux_Init
(
   gpiomux_init_t *initp   /* [IN] initialization structure pointer */
)
{
	/*gpiomux_dumpTables();*/
	
   gInit = *initp;      /* structure copy */

	if (gInit.initfunc) 
	{
		/* Caller can override normal 'reset all to default' behaviour. */
		gInit.initfunc();
	}
	else
	{
		unsigned int i;
		for ( i = 0; i < gpio_defs_MAX_PINS; i++ )
		{
			chipregHw_setPinFunction( (int)i, CHIPREGHW_PIN_FUNCTION_DEFAULT );
			chipregHw_setPinInputType( (int)i, CHIPREGHW_PIN_INPUTTYPE_DEFAULT );
			chipregHw_setPinPullup( (int)i, CHIPREGHW_PIN_PULL_DEFAULT );
			chipregHw_setPinOutputCurrent( (int)i, CHIPREGHW_PIN_CURRENT_STRENGTH_DEFAULT );
			chipregHw_setPinSlewRate( (int)i, CHIPREGHW_PIN_SLEW_RATE_DEFAULT );
		}
	}
	if (early_initialized) 
	{
		/* Don't let init succeed unless tables have also been registered */
		initialized = 1;
	}
}

/****************************************************************************/
/**
*  @brief   gpiomux_request
*
*  Used to register a pin's function with the gpiomux and use the default
*     pin parameters.
*
*  @return
*     success or conflict return code
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_request
(
   gpio_defs_e pin,                       /* [IN] GPIO pin */
   chipregHw_PIN_FUNCTION_e function,     /* [IN] GPIO pin function */
   const char *label                      /* [IN] description of pin usage */
)
{
   if (!initialized)
   {
		DBGPRINT("%s: gpiomux_rc_UNINITIALIZED\n", __func__);
      return gpiomux_rc_UNINITIALIZED;
   }

   if ( pin >= gpio_defs_MAX_PINS )
   {
		DBGPRINT("%s: gpiomux_rc_BADPIN\n", __func__);
      return gpiomux_rc_BADPIN;
   }

#if FORCE_CONFLICT_CHECK
   if (chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT)
   {
      DBGPRINT("pin=%d gpiomux_rc_ASSIGNED - chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT\n", pin);
      return gpiomux_rc_ASSIGNED;   /* Pin function already assigned, exit */
   }
#endif

   if ((gInit.request_gpio) && (gInit.request_gpio(pin, label) < 0))
   {
      DBGPRINT("pin=%d gpiomux_rc_CONFLICT - (gInit.request_gpio(pin, label) < 0)\n", pin);
      return gpiomux_rc_CONFLICT;   /* Could not request this pin externally */
   }

   DBGPRINT("%s: pin=%d set to function %d\n", __func__, pin, function);

   /* Assign new pin function */
   chipregHw_setPinFunction( (int)pin, function );

   return gpiomux_rc_SUCCESS;
}

/****************************************************************************/
/**
*  @brief   gpiomux_request_config
*
*  Used to register a pin's function with the gpiomux and set up the pin parameters.
*
*  @return
*     success or conflict return code
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_request_config
(
   gpio_defs_e pin,                       /* [IN] GPIO pin */
   chipregHw_PIN_FUNCTION_e function,     /* [IN] GPIO pin function */
   chipregHw_PIN_INPUTTYPE_e inputType,   /* [IN] input type */
   chipregHw_PIN_PULL_e pull,             /* [IN] pullup/pulldown */
   chipregHw_PIN_CURRENT_STRENGTH_e curr, /* [IN] drive strength */
   chipregHw_PIN_SLEW_RATE_e slewRate,    /* [IN] slew rate */
   const char *label                      /* [IN] description of pin usage */
)
{
   if (!initialized)
   {
		DBGPRINT("%s: gpiomux_rc_UNINITIALIZED\n", __func__);
      return gpiomux_rc_UNINITIALIZED;
   }

   if ( pin >= gpio_defs_MAX_PINS )
   {
		DBGPRINT("%s: gpiomux_rc_BADPIN\n", __func__);
      return gpiomux_rc_BADPIN;
   }

#if FORCE_CONFLICT_CHECK
   if (chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT)
   {
      DBGPRINT("pin=%d gpiomux_rc_ASSIGNED - chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT\n", pin);
      return gpiomux_rc_ASSIGNED;   /* Pin function already assigned, exit */
   }
#endif

   if ((gInit.request_gpio) && (gInit.request_gpio(pin, label) < 0))
   {
      DBGPRINT("pin=%d gpiomux_rc_CONFLICT - (gInit.request_gpio(pin, label) < 0)\n", pin);
      return gpiomux_rc_CONFLICT;   /* Could not request this pin externally */
   }

   /* Assign new settings */
   DBGPRINT("%s: pin=%d set to function %d\n", __func__, pin, function);
   chipregHw_setPinFunction( (int)pin, function );
   chipregHw_setPinInputType( (int)pin, inputType );
   chipregHw_setPinPullup( (int)pin, pull );
   chipregHw_setPinOutputCurrent( (int)pin, curr );
   chipregHw_setPinSlewRate( (int)pin, slewRate );

   return gpiomux_rc_SUCCESS;
}

/****************************************************************************/
/**
*  @brief   gpiomux_free
*
*  Used to deregister a pin assignment
*
*  @return
*     success - assigned function was unregistered
*     notfound - there was not registered function for this pin
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_free
(
   gpio_defs_e pin                    /* [IN] GPIO pin */
)
{
  	DBGPRINT("%s: pin %d\n", __func__, pin);
   if (!initialized)
   {
		DBGPRINT("%s: gpiomux_rc_UNINITIALIZED\n", __func__);
      return gpiomux_rc_UNINITIALIZED;
   }

   if ( pin >= gpio_defs_MAX_PINS )
   {
		DBGPRINT("%s: gpiomux_rc_BADPIN\n", __func__);
      return gpiomux_rc_BADPIN;
   }

   /*
    * If a user is reserving a gpio pin for a GPIO function and
    * does not require the mux, then the free must not fail if
    * the assigned function is GPIO, so we don't need to do a
    * conflict check here similar to the gpiomux_freeGroup() (to
    * make sure the current function is assigned).
    */

   /* Free the pin externally */
   if (gInit.free_gpio)
   {
      gInit.free_gpio(pin);
   }

   /* Revert back to default settings */
   chipregHw_setPinFunction( (int)pin, CHIPREGHW_PIN_FUNCTION_DEFAULT );
   chipregHw_setPinInputType( (int)pin, CHIPREGHW_PIN_INPUTTYPE_DEFAULT );
   chipregHw_setPinPullup( (int)pin, CHIPREGHW_PIN_PULL_DEFAULT );
   chipregHw_setPinOutputCurrent( (int)pin, CHIPREGHW_PIN_CURRENT_STRENGTH_DEFAULT );
   chipregHw_setPinSlewRate( (int)pin, CHIPREGHW_PIN_SLEW_RATE_DEFAULT );


   return gpiomux_rc_SUCCESS;
}

/****************************************************************************/
/**
*  @brief   gpiomux_requestGroup
*
*  Used to register a logical group of pin functions with the gpiomux
*
*  @return
*     success or conflict return code
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_requestGroup
(
   const gpiomux_group_e group,        /* [IN] GPIO group */
   const unsigned char id,                   /* [IN] ID within group */
   const char *label                   /* [IN] description of group usage */
)
{
   unsigned int groupidx;
   const gpiomux_group_t *groupp;

  	DBGPRINT("%s: group %d\n", __func__, group);
   if (!initialized)
   {
		DBGPRINT("%s: gpiomux_rc_UNINITIALIZED\n", __func__);
      return gpiomux_rc_UNINITIALIZED;
   }

   if (group >= gpiomux_group_MaxNum)
   {
		DBGPRINT("%s: gpiomux_rc_BADGROUP\n", __func__);
      return gpiomux_rc_BADGROUP;
   }
   /* Check for conflicts before assigning functions */
   for ( groupidx = 0; groupidx < gGroupListEntries; groupidx++ )
   {
      /* Find matching list */
      groupp = &gGroupList[groupidx];

      if ( groupp->group == group && groupp->id == id )
      {
         unsigned int pinidx;

         for (pinidx = 0; pinidx < groupp->numPins; pinidx++  )
         {
            const char *str;
            unsigned int pin = groupp->listp[pinidx].pin;

#if FORCE_CONFLICT_CHECK
            if (chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT)
            {
               DBGPRINT("group=%d pin=%d gpiomux_rc_ASSIGNED - chipregHw_getPinFunction(pin) != CHIPREGHW_PIN_FUNCTION_DEFAULT\n", group, pin);
               return gpiomux_rc_ASSIGNED;   /* Pin function already assigned, exit */
            }
#endif
            // SARU if (gInit.is_requested && ((str = gInit.is_requested(pin)) != NULL))
            if (gInit.is_requested && ((str = gInit.is_requested(pin)) != 0 ))
            {
               DBGPRINT("group=%d pin=%d ggpiomux_rc_CONFLICT - gInit.is_requested && (gInit.is_requested(pin) != NULL) \'%s\'\n", group, pin, str);
               return gpiomux_rc_CONFLICT;   /* Pin already assigned externally, exit */
            }
         }

         /* Pins all unassigned, so assign new functions now. */
         for (pinidx = 0; pinidx < groupp->numPins; pinidx++  )
         {
            int pin = groupp->listp[pinidx].pin;
            int rc = gpiomux_request_config(pin, groupp->listp[pinidx].function, groupp->listp[pinidx].inputType, groupp->listp[pinidx].pull, groupp->listp[pinidx].curr, groupp->listp[pinidx].slewRate, label);
            if (rc != 0)
            {
               DBGPRINT("ERROR gpiomux_request group=%d pin=%d rc=%d\n", group, pin, rc);
               return rc;
            }
         }
         break;
      }
   }
   return gpiomux_rc_SUCCESS;
}

/****************************************************************************/
/**
*  @brief   gpiomux_freeGroup
*
*  Used to deregister a logical set of pin functions
*
*  @return
*     success - assigned function was unregistered
*     notfound - there was not registered function for this pin
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_freeGroup
(
   gpiomux_group_e group,              /* [IN] GPIO group */
   const unsigned char id                    /* [IN] ID within group */
)
{
   unsigned int groupidx;
   const gpiomux_group_t *groupp;

  	DBGPRINT("%s: group %d\n", __func__, group);
   if (!initialized)
   {
		DBGPRINT("%s: gpiomux_rc_UNINITIALIZED\n", __func__);
      return gpiomux_rc_UNINITIALIZED;
   }

   if (group >= gpiomux_group_MaxNum)
   {
		DBGPRINT("%s: gpiomux_rc_BADGROUP\n", __func__);
      return gpiomux_rc_BADGROUP;
   }

   /* Check for conflicts before assigning functions */
   for ( groupidx = 0; groupidx < gGroupListEntries; groupidx++ )
   {
      /* Find matching list */
      groupp = &gGroupList[groupidx];

      if ( groupp->group == group && groupp->id == id )
      {
         unsigned int pinidx;
         for (pinidx = 0; pinidx < groupp->numPins; pinidx++  )
         {
            unsigned int pin = groupp->listp[pinidx].pin;
#if 0 /* do not execute FORCE_CONFLICT_CHECK for freeing pins */
            if (chipregHw_getPinFunction(pin) == CHIPREGHW_PIN_FUNCTION_DEFAULT)
            {
               DBGPRINT("group=%d pin=%d gpiomux_rc_UNASSIGNED - chipregHw_getPinFunction(pin) == CHIPREGHW_PIN_FUNCTION_DEFAULT\n", group, pin);
               return gpiomux_rc_UNASSIGNED;   /* pin already unassigned, cannot free */
            }
#endif
            /* Do not free an already externally freed pin */
            // SARU if (gInit.is_requested && (gInit.is_requested(pin) == NULL))
            if (gInit.is_requested && (gInit.is_requested(pin) == 0))
            {
               DBGPRINT("group=%d pin=%d gpiomux_rc_CONFLICT - gInit.is_requested && (gInit.is_requested(pin) != NULL)\n", group, pin);
               return gpiomux_rc_CONFLICT;   /* Pin already assigned externally, exit */
            }
         }
         /* Pins all unassigned, so revert to GPIO function now. */
         for (pinidx = 0; pinidx < groupp->numPins; pinidx++  )
         {
            unsigned int pin = groupp->listp[pinidx].pin;
            gpiomux_free(pin); /* cannot fail or we have a logic error */
         }
         break;
      }
   }
   return gpiomux_rc_SUCCESS;
}

EXPORT_SYMBOL( gpiomux_Init );
EXPORT_SYMBOL( gpiomux_request );
EXPORT_SYMBOL( gpiomux_request_config );
EXPORT_SYMBOL( gpiomux_free );
EXPORT_SYMBOL( gpiomux_requestGroup );
EXPORT_SYMBOL( gpiomux_freeGroup );

