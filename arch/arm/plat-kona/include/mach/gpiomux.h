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
*  @file    gpiomux.h
*
*  @brief   GPIOMUX utility
*
*/
/****************************************************************************/

#ifndef GPIOMUX_H
#define GPIOMUX_H

/* ---- Include Files ----------------------------------------------------- */
// SARU #include <mach/csp/chal_types.h>
// SARU #include <mach/csp/chipregHw_inline.h>
#include <mach/gpio_defs.h>
#include <mach/chipregHw_inline.h>

/* ---- Public Constants and Types ---------------------------------------- */
#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))
#endif

#define GPIOMUX_FUNC(x) 		chipregHw_PIN_FUNCTION_##x
#define GPIOMUX_TYPE(x) 		chipregHw_PIN_INPUTTYPE_##x
#define GPIOMUX_PULL(x) 		chipregHw_PIN_PULL_##x
#define GPIOMUX_CURR(x) 		chipregHw_PIN_CURRENT_STRENGTH_##x
#define GPIOMUX_SLEW(x) 		chipregHw_PIN_SLEW_RATE_##x

#define GPIOMUX_ENTRY(pin, func, type, pull, curr, slew) \
pin, \
GPIOMUX_FUNC(func), \
GPIOMUX_TYPE(type), \
GPIOMUX_PULL(pull), \
GPIOMUX_CURR(curr), \
GPIOMUX_SLEW(slew)

typedef struct 
{
   const gpio_defs_e pin;
   const chipregHw_PIN_FUNCTION_e function;     /* pin function       */
   const chipregHw_PIN_INPUTTYPE_e inputType;   /* input type         */
   const chipregHw_PIN_PULL_e pull;             /* pull-up/pull-down  */
   const chipregHw_PIN_CURRENT_STRENGTH_e curr; /* drive strength     */
   const chipregHw_PIN_SLEW_RATE_e slewRate;    /* slew rate          */
}
gpiomux_pinconfig_t;

/* A group is a logical collection of pins. They all
 * are set to the same chipc gpio function. */
typedef struct
{
   const gpiomux_group_e group;              /* group identifier */
   const unsigned char numPins;                    /* Number of pins for this group */
   const gpiomux_pinconfig_t *listp;         /* An array of pins for this group */
   const unsigned char id;                         /* ID to identify multiple variants of a group */
}
gpiomux_group_t;

/* Function return codes */
typedef enum
{
   gpiomux_rc_SUCCESS = 0,
   gpiomux_rc_UNINITIALIZED,  /* Calling functions before initialization */
   gpiomux_rc_ASSIGNED,       /* Pin already assigned, cannot request */
   gpiomux_rc_UNASSIGNED,     /* Pin unassigned, cannot free */
   gpiomux_rc_CONFLICT,       /* Conflict in external request/free */
   gpiomux_rc_BADPIN,         /* Bad pin enum parameter */
   gpiomux_rc_BADGROUP        /* Bad group enum parameter */
}
gpiomux_rc_e;

/* Initialization structure */
typedef struct
{
   /* If not NULL, this function is called to find out of a gpio pin has already
    * been requested. In linux this would presumably call the gpiolib is_requested
    * function to find out if a gpio_request() had been called. If the pin has
    * not been requested, then the gpiomux request calls should also fail.
    * Returns the original label if the function was already requested, else NULL. */
   const char * (*is_requested)(int pin);

   /* If not NULL, this function is called to request a gpio pin. In linux this
    * would presumably call the gpiolib request_gpio function to allocate the
    * pin. This keeps gpiomux and gpiolib in sync, and only gpiomux calls are
    * required to both allocate and setup the mux.
    * Returns 0 on success, < 0 on failure (busy or invalid pin) */
   int (*request_gpio)(int pin, const char *label);

   /* If not NULL, this function is called to free a gpio pin. In linux this
    * would presumably call the gpiolib free_gpio function to free the pin. */
   void (*free_gpio)(int pin);

	/* init will call this initialization function from the caller if non-NULL.
	 * If this is NULL then all pins will be reset to defaults. */
	void (*initfunc)(void);
}
gpiomux_init_t;

/* ---- Public Variables -------------------------------------------------- */
/* ---- Public Function Prototypes ---------------------------------------- */

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
void gpiomux_earlyinit
(
	/* gpiomux will use this table for it's specific pin settings.
	 * This allows different boards to specify different gpio pinmux mappings */
	const unsigned int groupListEntries,
	const gpiomux_group_t *groupList
	
);

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
);

/****************************************************************************/
/**
*  @brief   gpiomux_request
*
*  Used to register a chip function with the gpiomux
*     and use the default pin parameters
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
);

/****************************************************************************/
/**
*  @brief   gpiomux_request_config
*
*  Used to register a chip function with the gpiomux
*     and specify the pin parameters explicitly
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
);

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
   gpio_defs_e pin                   /* [IN] GPIO pin */
);
/****************************************************************************/
/**
*  @brief   gpiomux_requestGroup
*
*  Used to register a group of pin functions with the gpiomux
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
);

/****************************************************************************/
/**
*  @brief   gpiomux_freeGroup
*
*  Used to deregister a group of pin assignments
*
*  @return
*     success - assigned group was unregistered
*     notfound - there was not registered group for this pin
*/
/****************************************************************************/
gpiomux_rc_e gpiomux_freeGroup
(
   gpiomux_group_e group,              /* [IN] GPIO group */
   const unsigned char id                    /* [IN] ID within group */
);


/****************************************************************************/
/**
*  @brief   gpiomux_dumpTables
*
*  Used to dump tables
*
*  @return
*/
/****************************************************************************/
void gpiomux_dumpTables(void);

#endif /* GPIOMUX_H */
