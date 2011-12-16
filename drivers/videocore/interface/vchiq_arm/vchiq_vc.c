/*
 * Copyright (c) 2010-2011 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "vchiq_vc.h"

#if defined( __VC4_CAPRI__ )
#include "vcinclude/hardware.h"
#endif

int vchiq_num_instances;

int mutex_initialised;
static VCOS_MUTEX_T initialise_mutex;

VCOS_LOG_LEVEL_T vchiq_default_vc_log_level = VCOS_LOG_ERROR;
VCOS_LOG_CAT_T vchiq_vc_log_category;

static int is_valid_instance(VCHIQ_INSTANCE_T instance)
{
   int i;
   for (i = 0; i < vchiq_num_instances; i++)
   {
      if (instance == &vchiq_instances[i])
         return instance->state.initialised;
   }
   return 0;
}

VCHIQ_STATUS_T
vchiq_initialise(VCHIQ_INSTANCE_T *instance)
{
   VCHIQ_INSTANCE_T inst = NULL;
   VCHIQ_STATUS_T status;
   int i;

   vcos_global_lock();
   if (!mutex_initialised)
   {
      vcos_mutex_create(&initialise_mutex, "vchiq-init");
      vcos_log_set_level(VCOS_LOG_CATEGORY, vchiq_default_vc_log_level);
      vcos_log_register("vchiq_vc", VCOS_LOG_CATEGORY);

#if defined( __VC4_CAPRI__ )
      vcos_log_error( "VPU_AXI_SS_SDRAM_SPLIT_SEL = 0x%08x", VPU_AXI_SS_SDRAM_SPLIT_SEL );
#endif

      mutex_initialised = 1;
   }
   vcos_global_unlock();

   vcos_mutex_lock(&initialise_mutex);

   status = vchiq_platform_init();
   if (status != VCHIQ_SUCCESS)
      return status;

   for (i = 0; i < vchiq_num_instances; i++)
   {
      if (!vchiq_instances[i].state.initialised)
      {
         inst = &vchiq_instances[i];
         inst->connected = 0;
         inst->state.id = i;
         inst->state.initialised = 1;
         break;
      }
   }

   vcos_mutex_unlock(&initialise_mutex);

   *instance = inst;

   return (inst != NULL) ? VCHIQ_SUCCESS : VCHIQ_ERROR;
}

VCHIQ_STATUS_T
vchiq_shutdown(VCHIQ_INSTANCE_T instance)
{
   VCHIQ_STATE_T *state = &instance->state;
   int i;

   if (!is_valid_instance(instance))
      return VCHIQ_ERROR;

   /* Remove all services */

   for (i = 0; i < VCHIQ_MAX_SERVICES; i++)
   {
      VCHIQ_SERVICE_T *service = state->services[i];
      if (service && (service->srvstate != VCHIQ_SRVSTATE_FREE))
      {
         vchiq_remove_service(&service->base);
      }
   }

   /* Release the state/instance */
   state->initialised = 0;

   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_connect(VCHIQ_INSTANCE_T instance)
{
   VCHIQ_STATUS_T status;

   if (!is_valid_instance(instance))
      return VCHIQ_ERROR;

   vcos_mutex_lock(&instance->state.mutex);
   status = vchiq_connect_internal(&instance->state, instance);
   if (status == VCHIQ_SUCCESS)
      instance->connected = 1;
   vcos_mutex_unlock(&instance->state.mutex);
   return status;
}

VCHIQ_STATUS_T
vchiq_add_service(VCHIQ_INSTANCE_T instance, int fourcc, VCHIQ_CALLBACK_T callback, void *userdata, VCHIQ_SERVICE_HANDLE_T *pservice)
{
   VCHIQ_SERVICE_PARAMS_T params;

   params.fourcc        = fourcc;
   params.callback      = callback;
   params.userdata      = userdata;
   params.version       = 0;
   params.version_min   = 0;

   return vchiq_add_service_params(instance, &params, pservice);
}

VCHIQ_STATUS_T
vchiq_open_service(VCHIQ_INSTANCE_T instance, int fourcc, VCHIQ_CALLBACK_T callback, void *userdata, VCHIQ_SERVICE_HANDLE_T *pservice)
{
   VCHIQ_SERVICE_PARAMS_T params;

   params.fourcc        = fourcc;
   params.callback      = callback;
   params.userdata      = userdata;
   params.version       = 0;
   params.version_min   = 0;

   return vchiq_open_service_params(instance, &params, pservice);
}

VCHIQ_STATUS_T
vchiq_add_service_params(VCHIQ_INSTANCE_T instance,
   const VCHIQ_SERVICE_PARAMS_T *params,
   VCHIQ_SERVICE_HANDLE_T *pservice)
{
   VCHIQ_SERVICE_T *service = NULL;

   vcos_assert(params->fourcc != VCHIQ_FOURCC_INVALID);
   vcos_assert(params->callback != NULL);
   vcos_assert(params->version >= params->version_min);

   if (!is_valid_instance(instance))
      return VCHIQ_ERROR;

   vcos_mutex_lock(&instance->state.mutex);

   service = vchiq_add_service_internal(&instance->state, params,
                                        instance->connected ? VCHIQ_SRVSTATE_LISTENING :  VCHIQ_SRVSTATE_HIDDEN,
                                        instance);

   vcos_mutex_unlock(&instance->state.mutex);

   *pservice = &service->base;
   return (service != NULL) ? VCHIQ_SUCCESS : VCHIQ_ERROR;
}

VCHIQ_STATUS_T
vchiq_open_service_params(VCHIQ_INSTANCE_T instance,
   const VCHIQ_SERVICE_PARAMS_T *params,
   VCHIQ_SERVICE_HANDLE_T *pservice)
{
   VCHIQ_STATE_T *state = &instance->state;
   VCHIQ_SERVICE_T *service;
   VCHIQ_STATUS_T status = VCHIQ_ERROR;

   vcos_assert(params->fourcc != VCHIQ_FOURCC_INVALID);
   vcos_assert(params->callback != NULL);
   vcos_assert(params->version >= params->version_min);

   if (!is_valid_instance(instance))
      return VCHIQ_ERROR;

   if (!instance->connected)
      return VCHIQ_ERROR;

   vcos_mutex_lock(&state->mutex);

   service = vchiq_add_service_internal(state, params, VCHIQ_SRVSTATE_OPENING, instance);

   vcos_mutex_unlock(&state->mutex);

   if (service)
   {
      /* On VideoCore, there is effectively only one process, which is
       * given the arbitrary pid of 1.
       */
      status = vchiq_open_service_internal(service, 1);
      if (status != VCHIQ_SUCCESS)
      {
         vchiq_remove_service(&service->base);
         service = NULL;
      }
   }
   *pservice = &service->base;
   return status;
}

VCHIQ_STATUS_T
vchiq_vc_init_state(VCHIQ_VC_STATE_T* vc_state)
{
   return vcos_event_create(&vc_state->remote_use_active, "remote_use_active") == VCOS_SUCCESS ? VCHIQ_SUCCESS : VCHIQ_ERROR;
}


/* This is a private api only intended for use by powerman. */
VCHIQ_STATUS_T
vchiq_remote_use(VCHIQ_INSTANCE_T instance, VCHIQ_REMOTE_USE_CALLBACK_T callback, void* cb_arg)
{
   VCHIQ_STATE_T *state = &instance->state;
   VCHIQ_VC_STATE_T *vc_state = vchiq_platform_get_vc_state(state);
   VCHIQ_STATUS_T status = vc_state ? VCHIQ_RETRY : VCHIQ_ERROR; // vc_state null probably means not supported on this platform, so no point retrying
   if(vc_state && (state->conn_state > VCHIQ_CONNSTATE_DISCONNECTED) && !vc_state->remote_use_callback)
   {
      /* NOTE - use counting should be done externally. This function only supports
          one client and will always send a message to the host on calling */

      vc_state->remote_use_callback = callback;
      vc_state->remote_use_cb_arg = cb_arg;
      status = vchiq_send_remote_use(state);
   }
   return status;
}

/* This is a private api only intended for use by powerman. */
VCHIQ_STATUS_T
vchiq_remote_release(VCHIQ_INSTANCE_T instance)
{
   VCHIQ_STATE_T *state = &instance->state;
   VCHIQ_VC_STATE_T *vc_state = vchiq_platform_get_vc_state(state);
   VCHIQ_STATUS_T status = vc_state ? VCHIQ_RETRY : VCHIQ_ERROR; // vc_state null probably means not supported on this platform, so no point retrying
   if(vc_state && (state->conn_state > VCHIQ_CONNSTATE_DISCONNECTED) && !vc_state->remote_use_callback)
   {
      status = vchiq_send_remote_release(state);
   }
   return status;
}

/* This is called when we receive the acknowledgement from the host of the remote use */
void
vchiq_on_remote_use_active(VCHIQ_STATE_T *state)
{
   VCHIQ_VC_STATE_T *vc_state = vchiq_platform_get_vc_state(state);
   if(vc_state && vc_state->remote_use_callback)
   {
      vc_state->remote_use_callback(vc_state->remote_use_cb_arg);
      vc_state->remote_use_callback = 0;
   }
}



/* stub functions */

VCHIQ_STATUS_T
vchiq_on_remote_use(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   return VCHIQ_SUCCESS;
}
VCHIQ_STATUS_T
vchiq_on_remote_release(VCHIQ_STATE_T *state)
{
   vcos_unused(state);
   return VCHIQ_SUCCESS;
}

void
vchiq_dump_platform_state(void *dump_context)
{
   vcos_unused(dump_context);
}

void
vchiq_dump_platform_instances(void *dump_context)
{
   vcos_unused(dump_context);
}

void
vchiq_dump_platform_service_state(void *dump_context,
   VCHIQ_SERVICE_T *service)
{
   vcos_unused(dump_context);
   vcos_unused(service);
}

VCHIQ_STATUS_T
vchiq_use_service(VCHIQ_SERVICE_HANDLE_T handle)
{
   vcos_unused(handle);
   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_release_service(VCHIQ_SERVICE_HANDLE_T handle)
{
   vcos_unused(handle);
   return VCHIQ_SUCCESS;
}

VCHIQ_STATUS_T
vchiq_check_service(VCHIQ_SERVICE_HANDLE_T handle)
{
   vcos_unused(handle);
   return VCHIQ_SUCCESS;
}

