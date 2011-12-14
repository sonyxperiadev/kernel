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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vchiq_test.h"
#ifndef USE_VCHIQ_ARM
#define USE_VCHIQ_ARM
#endif
#include "interface/vchi/vchi.h"

#define NUM_BULK_BUFS 2
#define BULK_SIZE (1024*256)
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

static struct test_params g_params = { MSG_CONFIG, 64, 100, 1, 1 };
static const char *g_servname = "echo";

static VCOS_EVENT_T g_server_reply;
static VCOS_EVENT_T g_shutdown;
static VCOS_MUTEX_T g_mutex;

static const char *g_server_error = NULL;

static volatile int g_sync_mode = 0;

static VCOS_EVENT_T func_test_sync;
static int want_echo = 1;
static int fun2_error = 0;

char *bulk_tx_data[NUM_BULK_BUFS];
char *bulk_rx_data[NUM_BULK_BUFS];

static int ctrl_received = 0;
static int bulk_tx_sent = 0;
static int bulk_rx_sent = 0;
static int bulk_tx_received = 0;
static int bulk_rx_received = 0;

static char clnt_service1_data[SERVICE1_DATA_SIZE];
static char clnt_service2_data[SERVICE2_DATA_SIZE];

static VCOS_LOG_CAT_T vchiq_test_log_category;

static int vchiq_test(int argc, char **argv);
static VCHIQ_STATUS_T vchiq_bulk_test(void);
static VCHIQ_STATUS_T vchiq_ctrl_test(void);
static VCHIQ_STATUS_T vchiq_functional_test(void);
static VCHIQ_STATUS_T vchiq_ping_test(void);

static VCHIQ_STATUS_T do_functional_test(void);
static void do_ping_test(VCHIQ_SERVICE_HANDLE_T service, int size, int async, int oneway, int iters);
static void do_vchi_ping_test(VCHI_SERVICE_HANDLE_T service, int size, int async, int oneway, int iters);

static VCHIQ_STATUS_T func_data_test(VCHIQ_SERVICE_HANDLE_T service, int size, int align);

#ifdef VCHIQ_LOCAL
static void *vchiq_test_server(void *);
#endif

static VCHIQ_STATUS_T
clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
   VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static void
vchi_clnt_callback(void *callback_param, VCHI_CALLBACK_REASON_T reason,
   void *handle);
static VCHIQ_STATUS_T func_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                         VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static VCHIQ_STATUS_T fun2_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                         VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static int mem_check(const void *expected, const void *actual, int size);
static void usage(void);
static void check_timer(void);

static int vchiq_test(int argc, char **argv)
{
   VCHIQ_STATUS_T status;
   int run_bulk_test = 0;
   int run_ctrl_test = 0;
   int run_functional_test = 0;
   int run_ping_test = 0;
   int verbose = 0;
   int argn;
 
   argn = 1;
   while ((argn < argc) && (argv[argn][0] == '-'))
   {
      const char *arg = argv[argn++];
      if (strcmp(arg, "-s") == 0)
      {
         g_servname = argv[argn++];
         if (!g_servname || (strlen(g_servname) != 4))
         {
            usage();
         }
      }
      else if (strcmp(arg, "-b") == 0)
      {
         run_bulk_test = 1;
         g_params.blocksize = atoi(argv[argn++]);
      }
      else if (strcmp(arg, "-c") == 0)
      {
         run_ctrl_test = 1;
         g_params.blocksize = atoi(argv[argn++]);
      }
      else if (strcmp(arg, "-e") == 0)
      {
         want_echo = 0;
      }
      else if (strcmp(arg, "-f") == 0)
      {
         run_functional_test = 1;
      }
      else if (strcmp(arg, "-h") == 0)
      {
         usage();
      }
      else if (strcmp(arg, "-p") == 0)
      {
         run_ping_test = 1;
         g_params.iters = 1000;
      }
      else if (strcmp(arg, "-q") == 0)
      {
         g_params.verify = 0;
      }
      else if (strcmp(arg, "-t") == 0)
      {
         check_timer();
         exit(0);
      }
      else if (strcmp(arg, "-v") == 0)
      {
         verbose = 1;
      }
      else
      {
         printf("* unknown option '%s'\n", arg);
         usage();
      }
   }

   if ((run_ctrl_test + run_bulk_test + run_functional_test + run_ping_test) != 1)
      usage();

   if (argn < argc)
   {
      g_params.iters = atoi(argv[argn++]);
      if (argn != argc)
      {
         usage();
      }
   }

   vcos_log_set_level(VCOS_LOG_CATEGORY, verbose ? VCOS_LOG_TRACE : VCOS_LOG_INFO);
   vcos_log_register("vchiq_test", VCOS_LOG_CATEGORY);

#ifdef VCHIQ_LOCAL
   {
      static VCOS_THREAD_T server_task;
      void          *pointer = NULL;
      int stack_size = 4096;

#if VCOS_CAN_SET_STACK_ADDR
      pointer = malloc(stack_size);
      vcos_demand(pointer);
#endif
      vcos_thread_create_classic(&server_task, "vchiq_test server", vchiq_test_server, (void *)0, pointer, stack_size,
                                 10 | VCOS_AFFINITY_CPU1, 20, VCOS_START);
   }
#endif

   vcos_event_create(&g_server_reply, "g_server_reply");
   vcos_event_create(&g_shutdown, "g_shutdown");
   vcos_mutex_create(&g_mutex, "g_mutex");


   status = VCHIQ_ERROR;

   if (run_bulk_test)
      status = vchiq_bulk_test();
   else if (run_ctrl_test)
      status = vchiq_ctrl_test();
   else if (run_functional_test)
      status = vchiq_functional_test();
   else if (run_ping_test)
      status = vchiq_ping_test();

   return (status == VCHIQ_SUCCESS) ? 0 : -1;
}

static VCHIQ_STATUS_T
vchiq_bulk_test(void)
{
   VCHIQ_INSTANCE_T vchiq_instance;
   VCHIQ_SERVICE_HANDLE_T vchiq_service;
   VCHIQ_SERVICE_PARAMS_T service_params;
   VCHIQ_ELEMENT_T elements[4];
   VCHIQ_ELEMENT_T *element;
   int num_bulk_bufs = NUM_BULK_BUFS;
   uint32_t start, end;
   int i;

   g_params.blocksize *= 1024;

   for (i = 0; i < NUM_BULK_BUFS; i++)
   {
      int j;
      bulk_tx_data[i] = malloc(g_params.blocksize);
      bulk_rx_data[i] = malloc(g_params.blocksize);
      if (!bulk_tx_data[i] || !bulk_rx_data[i])
      {
         printf("* out of memory\n");
         return VCHIQ_ERROR;
      }
      for (j = 0; j < g_params.blocksize; j+=4)
      {
         *(unsigned int *)(bulk_tx_data[i] + j) = ((0x80 | i) << 24) + j;
      }
      memset(bulk_rx_data[i], 0xff, g_params.blocksize);
   }

//   fprintf(stderr, "vchiq_test: opening vchiq\n");

   if (vchiq_initialise(&vchiq_instance) != VCHIQ_SUCCESS)
   {
      printf("* failed to open vchiq instance\n");
      return VCHIQ_ERROR;
   }

   vchiq_connect(vchiq_instance);

   memset(&service_params, 0, sizeof(service_params));

   service_params.fourcc = VCHIQ_MAKE_FOURCC(g_servname[0], g_servname[1], g_servname[2], g_servname[3]);
   service_params.callback = clnt_callback;
   service_params.userdata = "clnt userdata";
   service_params.version = 0;
   service_params.version_min = 0;

   if (vchiq_open_service_params(vchiq_instance, &service_params, &vchiq_service) != VCHIQ_SUCCESS)
   {
      printf("* failed to open service - already in use?\n");
      return VCHIQ_ERROR;
   }

   printf("Bulk test - service:%s, block size:%d, iters:%d\n", g_servname, g_params.blocksize, g_params.iters);

   g_params.echo = want_echo;
   element = elements;
   element->data = &g_params;
   element->size = sizeof(g_params);
   element++;

   vchiq_queue_message(vchiq_service, elements, element - elements);

   vcos_event_wait(&g_server_reply);

   if (g_server_error)
   {
      printf("* server error: %s\n", g_server_error);
      return VCHIQ_ERROR;
   }

   if ( num_bulk_bufs > g_params.iters )
      num_bulk_bufs = g_params.iters;

   start = vcos_getmicrosecs();

   vcos_mutex_lock(&g_mutex);

   for (i = 0; i < num_bulk_bufs; i++)
   {
      vchiq_queue_bulk_transmit(vchiq_service, bulk_tx_data[i], g_params.blocksize, (void *)i);

      vcos_log_trace("vchiq_test: queued bulk tx %d", i);
      bulk_tx_sent++;

      if (g_params.echo)
      {
         vchiq_queue_bulk_receive(vchiq_service, bulk_rx_data[i], g_params.blocksize, (void *)i);

         vcos_log_trace("vchiq_test: queued bulk rx %d", i);
         bulk_rx_sent++;
      }
   }

   vcos_mutex_unlock(&g_mutex);

   vcos_log_trace("Sent all messages");

   vcos_log_trace("vchiq_test: waiting for shutdown");

   vcos_event_wait(&g_shutdown);

   end = vcos_getmicrosecs();

   vchiq_remove_service(vchiq_service);

   vcos_log_trace("vchiq_test: shutting down");

   vchiq_shutdown(vchiq_instance);

   printf("Elapsed time: %dus per iteration\n", (end - start) / g_params.iters);

   return VCHIQ_SUCCESS;
}

static VCHIQ_STATUS_T
vchiq_ctrl_test(void)
{
   VCHIQ_INSTANCE_T vchiq_instance;
   VCHIQ_SERVICE_HANDLE_T vchiq_service;
   VCHIQ_SERVICE_PARAMS_T service_params;
   uint32_t start, end;
   int i;

   ctrl_received = 0;
   if (g_params.blocksize < 4)
      g_params.blocksize = 4;

   for (i = 0; i < NUM_BULK_BUFS; i++)
   {
      int j;
      bulk_tx_data[i] = malloc(g_params.blocksize);
      if (!bulk_tx_data[i])
      {
         printf("* out of memory\n");
         return VCHIQ_ERROR;
      }
      *(int *)bulk_tx_data[i] = MSG_ECHO;
      for (j = 4; j < g_params.blocksize; j+=4)
      {
         *(unsigned int *)(bulk_tx_data[i] + j) = ((0x80 | i) << 24) + j;
      }
   }

   if (vchiq_initialise(&vchiq_instance) != VCHIQ_SUCCESS)
   {
      printf("* failed to open vchiq instance\n");
      return VCHIQ_ERROR;
   }

   vchiq_connect(vchiq_instance);

   memset(&service_params, 0, sizeof(service_params));

   service_params.fourcc = VCHIQ_MAKE_FOURCC(g_servname[0], g_servname[1], g_servname[2], g_servname[3]);
   service_params.callback = clnt_callback;
   service_params.userdata = "clnt userdata";
   service_params.version = 0;
   service_params.version_min = 0;

   if (vchiq_open_service_params(vchiq_instance, &service_params, &vchiq_service) != VCHIQ_SUCCESS)
   {
      printf("* failed to open service - already in use?\n");
      return VCHIQ_ERROR;
   }

   printf("Ctrl test - service:%s, block size:%d, iters:%d\n", g_servname, g_params.blocksize, g_params.iters);

   start = vcos_getmicrosecs();

   for (i = 0; i < g_params.iters; i++)
   {
      VCHIQ_ELEMENT_T element;
      element.data = bulk_tx_data[i % NUM_BULK_BUFS];
      element.size = g_params.blocksize;

      if (vchiq_queue_message(vchiq_service, &element, 1) != VCHIQ_SUCCESS)
      {
         printf("* failed to send a message\n");
         goto error_exit;
      }
      if (g_server_error)
      {
         printf("* error - %s\n", g_server_error);
         goto error_exit;
      }
   }

   vcos_log_trace("Sent all messages");

   if (g_params.echo)
   {
      vcos_log_trace("vchiq_test: waiting for shutdown");

      vcos_event_wait(&g_shutdown);
   }

   if (g_server_error)
   {
      printf("* error - %s\n", g_server_error);
      goto error_exit;
   }

   end = vcos_getmicrosecs();

   vchiq_remove_service(vchiq_service);

   vcos_log_trace("vchiq_test: shutting down");

   vchiq_shutdown(vchiq_instance);

   printf("Elapsed time: %dus per iteration\n", (end - start) / g_params.iters);

   return VCHIQ_SUCCESS;

error_exit:
   vchiq_remove_service(vchiq_service);
   vchiq_shutdown(vchiq_instance);
   return VCHIQ_ERROR;
}

static VCHIQ_STATUS_T
vchiq_functional_test(void)
{
   int i;
   printf("Functional test - iters:%d\n", g_params.iters);
   for (i = 0; i < g_params.iters; i++)
   {
      printf("======== iteration %d ========\n", i+1);

      if (do_functional_test() != VCHIQ_SUCCESS)
         return VCHIQ_ERROR;
   }
   return VCHIQ_SUCCESS;
}

static VCHIQ_STATUS_T
vchiq_ping_test(void)
{
   /* Measure message round trip time for various sizes*/
   VCHIQ_INSTANCE_T vchiq_instance;
   VCHIQ_SERVICE_HANDLE_T vchiq_service;
   VCHI_SERVICE_HANDLE_T vchi_service;
   SERVICE_CREATION_T service_params;
   int fourcc;

   static int sizes[] = { 0, 1024, 2048, VCHIQ_MAX_MSG_SIZE };
   unsigned int i;

   fourcc = VCHIQ_MAKE_FOURCC(g_servname[0], g_servname[1], g_servname[2], g_servname[3]);

   printf("Ping test - service:%s, iters:%d\n", g_servname, g_params.iters);

   if (vchiq_initialise(&vchiq_instance) != VCHIQ_SUCCESS)
   {
      printf("* failed to open vchiq instance\n");
      return VCHIQ_ERROR;
   }

   vchiq_connect(vchiq_instance);

   service_params.service_id = fourcc;
   service_params.callback = vchi_clnt_callback;
   service_params.callback_param = &vchi_service;

   if (vchi_service_open((VCHI_INSTANCE_T)vchiq_instance, &service_params, &vchi_service) != VCHIQ_SUCCESS)
   {
      printf("* failed to open service - already in use?\n");
      return VCHIQ_ERROR;
   }

   for (i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++)
   {
      const int iter_count = g_params.iters;
      do_vchi_ping_test(vchi_service, sizes[i], 0, 0, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 0, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 1, 0, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 2, 0, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 10, 0, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 1, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 2, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 10, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 10, 10, iter_count);
      do_vchi_ping_test(vchi_service, sizes[i], 100, 0, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 100, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 100, 100, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 200, 0, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 200, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 200, 200, iter_count/10);
      do_vchi_ping_test(vchi_service, sizes[i], 400, 0, iter_count/20);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 400, iter_count/20);
      do_vchi_ping_test(vchi_service, sizes[i], 400, 400, iter_count/20);
      do_vchi_ping_test(vchi_service, sizes[i], 1000, 0, iter_count/50);
      do_vchi_ping_test(vchi_service, sizes[i], 0, 1000, iter_count/50);
      do_vchi_ping_test(vchi_service, sizes[i], 1000, 1000, iter_count/50);
   }

   vchi_service_close(vchi_service);

   if (vchiq_open_service(vchiq_instance, fourcc, clnt_callback, "clnt userdata", &vchiq_service) != VCHIQ_SUCCESS)
   {
      printf("* failed to open service - already in use?\n");
      return VCHIQ_ERROR;
   }

   for (i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++)
   {
      const int iter_count = g_params.iters;
      do_ping_test(vchiq_service, sizes[i], 0, 0, iter_count);
      do_ping_test(vchiq_service, sizes[i], 0, 0, iter_count);
      do_ping_test(vchiq_service, sizes[i], 1, 0, iter_count);
      do_ping_test(vchiq_service, sizes[i], 2, 0, iter_count);
      do_ping_test(vchiq_service, sizes[i], 10, 0, iter_count);
      do_ping_test(vchiq_service, sizes[i], 0, 1, iter_count);
      do_ping_test(vchiq_service, sizes[i], 0, 2, iter_count);
      do_ping_test(vchiq_service, sizes[i], 0, 10, iter_count);
      do_ping_test(vchiq_service, sizes[i], 10, 10, iter_count);
      do_ping_test(vchiq_service, sizes[i], 100, 0, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 0, 100, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 100, 100, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 200, 0, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 0, 200, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 200, 200, iter_count/10);
      do_ping_test(vchiq_service, sizes[i], 400, 0, iter_count/20);
      do_ping_test(vchiq_service, sizes[i], 0, 400, iter_count/20);
      do_ping_test(vchiq_service, sizes[i], 400, 400, iter_count/20);
      do_ping_test(vchiq_service, sizes[i], 1000, 0, iter_count/50);
      do_ping_test(vchiq_service, sizes[i], 0, 1000, iter_count/50);
      do_ping_test(vchiq_service, sizes[i], 1000, 1000, iter_count/50);
   }

   vchiq_remove_service(vchiq_service);

   return VCHIQ_SUCCESS;
}

static VCHIQ_STATUS_T
do_functional_test(void)
{
   VCHIQ_ELEMENT_T elements[4];
   VCHIQ_INSTANCE_T instance;
   VCHIQ_SERVICE_HANDLE_T service, service2, service3;
   VCHIQ_SERVICE_PARAMS_T service_params;
   VCHIQ_CONFIG_T config;
   unsigned int size, i;

   vcos_event_create(&func_test_sync, "test_sync");

   EXPECT(vchiq_initialise(&instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_get_config(instance, sizeof(config) - 1, &config), VCHIQ_SUCCESS); // too small, but allowed for backwards compatibility
   EXPECT(vchiq_get_config(instance, sizeof(config) + 1, &config), VCHIQ_ERROR);   // too large
   EXPECT(vchiq_get_config(instance, sizeof(config), &config), VCHIQ_SUCCESS);    // just right
   EXPECT(config.max_msg_size, VCHIQ_MAX_MSG_SIZE);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, func_clnt_callback, (void *)1, &service), VCHIQ_SUCCESS);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, func_clnt_callback, (void *)2, &service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, clnt_callback, (void *)3, &service3), VCHIQ_ERROR); // callback doesn't match
   EXPECT(vchiq_set_service_option(service, VCHIQ_SERVICE_OPTION_AUTOCLOSE, 0), VCHIQ_SUCCESS);
   EXPECT(vchiq_set_service_option(service, VCHIQ_SERVICE_OPTION_AUTOCLOSE, 1), VCHIQ_SUCCESS);
   EXPECT(vchiq_set_service_option(service, 42, 1), VCHIQ_ERROR); // invalid option
   EXPECT(vchiq_remove_service(service), VCHIQ_SUCCESS);
   EXPECT(vchiq_remove_service(service), VCHIQ_ERROR); // service already removed
   EXPECT(vchiq_remove_service(service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_queue_message(service, NULL, 0), VCHIQ_ERROR); // service not valid
   EXPECT(vchiq_set_service_option(service, VCHIQ_SERVICE_OPTION_AUTOCLOSE, 0), VCHIQ_ERROR); // service not valid
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, clnt_callback, (void *)3, &service3), VCHIQ_SUCCESS);
   EXPECT(vchiq_queue_message(service, NULL, 0), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_queue_bulk_transmit(service, clnt_service1_data, sizeof(clnt_service1_data), (void *)1), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_queue_bulk_receive(service2, clnt_service2_data, sizeof(clnt_service2_data), (void *)2), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_queue_bulk_receive(service, 0, sizeof(clnt_service1_data), (void *)1), VCHIQ_ERROR); // invalid buffer
   EXPECT(vchiq_shutdown(instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_initialise(&instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)1, &service), VCHIQ_ERROR); // not connected
   EXPECT(vchiq_connect(instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)1, &service), VCHIQ_ERROR); // wrong version number
   memset(&service_params, 0, sizeof(service_params));
   service_params.fourcc = FUNC_FOURCC;
   service_params.callback = func_clnt_callback;
   service_params.userdata = (void*)1;
   service_params.version = 1;
   service_params.version_min = 1;
   EXPECT(vchiq_open_service_params(instance, &service_params, &service), VCHIQ_ERROR); // Still the wrong version number
   service_params.version = 4;
   service_params.version_min = 4;
   EXPECT(vchiq_open_service_params(instance, &service_params, &service), VCHIQ_ERROR); // Still the wrong version number
   service_params.version_min = 2;
   EXPECT(vchiq_open_service_params(instance, &service_params, &service), VCHIQ_SUCCESS); // That's better
   EXPECT(vchiq_open_service(instance, VCHIQ_MAKE_FOURCC('n','o','n','e'), func_clnt_callback, (void*)2, &service2), VCHIQ_ERROR); // no listener
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)2, &service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)3, &service3), VCHIQ_ERROR); // no more listeners
   EXPECT(vchiq_remove_service(service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)2, &service2), VCHIQ_SUCCESS);

   elements[0].data = "a";
   elements[0].size = 1;
   elements[1].data = "bcdef";
   elements[1].size = 5;
   elements[2].data = "ghijklmnopq";
   elements[2].size = 11;
   elements[3].data = "rstuvwxyz";
   elements[3].size = 9;
   EXPECT(vchiq_queue_message(service, elements, 4), VCHIQ_SUCCESS);

   EXPECT(vchiq_queue_bulk_transmit(service2, clnt_service2_data, sizeof(clnt_service2_data), (void *)0x2001), VCHIQ_SUCCESS);
   for (i = 0; i < sizeof(clnt_service1_data); i++)
   {
      clnt_service1_data[i] = i;
   }
   EXPECT(vchiq_queue_bulk_transmit(service, clnt_service1_data, sizeof(clnt_service1_data), (void*)0x1001), VCHIQ_SUCCESS);

   vcos_event_wait(&func_test_sync);
   EXPECT(vchiq_remove_service(service), VCHIQ_SUCCESS);
   vcos_event_wait(&func_test_sync);

   EXPECT(vchiq_shutdown(instance), VCHIQ_SUCCESS);

   vcos_event_wait(&func_test_sync);

   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)0, &service), VCHIQ_ERROR); /* Instance not initialised */
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)0, &service), VCHIQ_ERROR); /* Instance not initialised */
   EXPECT(vchiq_connect(instance), VCHIQ_ERROR); /* Instance not initialised */

   /* Now test the bulk data transfers */
   EXPECT(vchiq_initialise(&instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_connect(instance), VCHIQ_SUCCESS);

   EXPECT(vchiq_open_service(instance, FUN2_FOURCC, fun2_clnt_callback, (void*)0, &service), VCHIQ_SUCCESS);

   for (size = 1; size < 64; size++)
   {
      int align;
      for (align = 0; align < 32; align++)
      {
         EXPECT(func_data_test(service, size, align), VCHIQ_SUCCESS);
      }
      for (align = PAGE_SIZE - 32; align < PAGE_SIZE; align++)
      {
         EXPECT(func_data_test(service, size, align), VCHIQ_SUCCESS);
      }
   }

   for (size = 64; size < FUN2_MAX_DATA_SIZE; size<<=1)
   {
      EXPECT(func_data_test(service, size-1, 0), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size-1, 1), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size-1, 31), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size-1, PAGE_SIZE - 32), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size-1, PAGE_SIZE - 31), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size-1, PAGE_SIZE - 1), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, 0), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, 1), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, 31), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, PAGE_SIZE - 32), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, PAGE_SIZE - 31), VCHIQ_SUCCESS);
      EXPECT(func_data_test(service, size, PAGE_SIZE - 1), VCHIQ_SUCCESS);
   }

   EXPECT(vchiq_shutdown(instance), VCHIQ_SUCCESS);

   vcos_event_delete(&func_test_sync);

   return VCHIQ_SUCCESS;

error_exit:
   return VCHIQ_ERROR;
}

static void
do_ping_test(VCHIQ_SERVICE_HANDLE_T service, int size, int async, int oneway, int iters)
{
   uint32_t start, end;
   char *ping_buf = malloc(size + sizeof(struct test_params));
   struct test_params *params = (struct test_params *)ping_buf;
   VCHIQ_ELEMENT_T element;
   int i;

   /* Allow enough room for the type header */
   element.size = (size < 4) ? 4 : size;
   element.data = ping_buf;

   bulk_tx_received = -1;

   start = vcos_getmicrosecs();
   for (i = 0; i < iters; i++)
   {
      int j;
      for (j = 0; j < vcos_max(async, oneway); j++)
      {
         if (j < async)
         {
            params->magic = MSG_ASYNC;
            vchiq_queue_message(service, &element, 1);
         }
         if (j < oneway)
         {
            params->magic = MSG_ONEWAY;
            vchiq_queue_message(service, &element, 1);
         }
      }
      params->magic = MSG_SYNC;
      vchiq_queue_message(service, &element, 1);
      vcos_event_wait(&g_server_reply);
   }
   end = vcos_getmicrosecs();

   printf("ping (size %d, %d async, %d oneway) -> %fus\n", size, async, oneway, ((float)(end - start))/iters);

   vcos_sleep(20);

   if ((async == 0) && (oneway == 0))
   {
      params->magic = MSG_CONFIG;
      params->blocksize = size ? size : 8;
      params->iters = iters;
      params->verify = 0;
      params->echo = 0;

      element.size = sizeof(*params);
      vchiq_queue_message(service, &element, 1);
      vcos_event_wait(&g_server_reply);

      vcos_sleep(30);

      start = vcos_getmicrosecs();
      for (i = 0; i < iters; i++)
      {
         vchiq_queue_bulk_transmit(service, ping_buf, params->blocksize, 0);
         vcos_event_wait(&g_server_reply);
      }
      end = vcos_getmicrosecs();

      printf("bulk (size %d, async) -> %fus\n", size, ((float)(end - start))/iters);

      vcos_sleep(40);
   }

   if (oneway == 0)
   {
      params->magic = MSG_CONFIG;
      params->blocksize = size ? size : 8;
      params->iters = iters * (async + 1);
      params->verify = 0;
      params->echo = 0;

      element.size = sizeof(*params);
      vchiq_queue_message(service, &element, 1);
      vcos_event_wait(&g_server_reply);

      vcos_sleep(50);

      start = vcos_getmicrosecs();
      for (i = 0; i < iters; i++)
      {
         int j;
         for (j = 0; j < async; j++)
            vchiq_bulk_transmit(service, ping_buf, params->blocksize, 0, VCHIQ_BULK_MODE_NOCALLBACK);
         vchiq_bulk_transmit(service, ping_buf, params->blocksize, 0, VCHIQ_BULK_MODE_BLOCKING);
      }
      end = vcos_getmicrosecs();

      printf("bulk (size %d, %d async) -> %fus\n", size, async, ((float)(end - start))/iters);

      vcos_sleep(60);
   }

   free(ping_buf);

   bulk_tx_received = 0;
}

static void
do_vchi_ping_test(VCHI_SERVICE_HANDLE_T service, int size, int async, int oneway, int iters)
{
   uint32_t start, end;
   uint32_t actual;
   char *ping_buf = malloc(size + sizeof(struct test_params));
   char pong_buf[100];
   struct test_params *params = (struct test_params *)ping_buf;
   int msg_size;
   int i;

   /* Allow enough room for the type header */
   msg_size = (size < 4) ? 4 : size;

   bulk_tx_received = -1;

   if ((oneway == 0) && (async == 0))
   {
      params->magic = MSG_SYNC;

      g_sync_mode = 1;

      start = vcos_getmicrosecs();
      for (i = 0; i < iters; i++)
      {
         uint32_t actual;
         vchi_msg_queue(service, ping_buf, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
         vchi_msg_dequeue(service, pong_buf, sizeof(pong_buf), &actual, VCHI_FLAGS_BLOCK_UNTIL_OP_COMPLETE);
      }
      end = vcos_getmicrosecs();

      printf("vchi ping (size %d) -> %fus\n", size, ((float)(end - start))/iters);

      vcos_sleep(10);

      g_sync_mode = 0;
   }

   while (vchi_msg_dequeue(service, pong_buf, sizeof(pong_buf), &actual, VCHI_FLAGS_NONE) != -1)
   {
      printf("* Unexpected message found in queue - size %d\n", actual);
   }

   start = vcos_getmicrosecs();
   for (i = 0; i < iters; i++)
   {
      int j;
      for (j = 0; j < vcos_max(async, oneway); j++)
      {
         if (j < async)
         {
            params->magic = MSG_ASYNC;
            vchi_msg_queue(service, ping_buf, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
         }
         if (j < oneway)
         {
            params->magic = MSG_ONEWAY;
            vchi_msg_queue(service, ping_buf, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
         }
      }
      params->magic = MSG_SYNC;
      vchi_msg_queue(service, ping_buf, msg_size, VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
      vcos_event_wait(&g_server_reply);
   }
   end = vcos_getmicrosecs();

   printf("vchi ping (size %d, %d async, %d oneway) -> %fus\n", size, async, oneway, ((float)(end - start))/iters);

   vcos_sleep(20);

   if ((async == 0) && (oneway == 0))
   {
      params->magic = MSG_CONFIG;
      params->blocksize = size ? size : 8;
      params->iters = iters;
      params->verify = 0;
      params->echo = 0;

      vchi_msg_queue(service, params, sizeof(*params), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
      vcos_event_wait(&g_server_reply);

      vcos_sleep(30);

      start = vcos_getmicrosecs();
      for (i = 0; i < iters; i++)
      {
         vchi_bulk_queue_transmit(service, ping_buf, params->blocksize,
            VCHI_FLAGS_CALLBACK_WHEN_OP_COMPLETE | VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
         vcos_event_wait(&g_server_reply);
      }
      end = vcos_getmicrosecs();

      printf("vchi bulk (size %d, async) -> %fus\n", size, ((float)(end - start))/iters);

      vcos_sleep(40);
   }

   if (oneway == 0)
   {
      params->magic = MSG_CONFIG;
      params->blocksize = size ? size : 8;
      params->iters = iters * (async + 1);
      params->verify = 0;
      params->echo = 0;

      vchi_msg_queue(service, params, sizeof(*params), VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
      vcos_event_wait(&g_server_reply);

      vcos_sleep(50);

      start = vcos_getmicrosecs();
      for (i = 0; i < iters; i++)
      {
         int j;
         for (j = 0; j < async; j++)
            vchi_bulk_queue_transmit(service, ping_buf, params->blocksize, VCHI_FLAGS_NONE, 0);
         vchi_bulk_queue_transmit(service, ping_buf, params->blocksize, VCHI_FLAGS_BLOCK_UNTIL_DATA_READ, 0);
      }
      end = vcos_getmicrosecs();

      printf("vchi bulk (size %d, %d async) -> %fus\n", size, async, ((float)(end - start))/iters);

      vcos_sleep(60);
   }

   free(ping_buf);

   bulk_tx_received = 0;
}

static VCHIQ_STATUS_T
func_data_test(VCHIQ_SERVICE_HANDLE_T service, int datalen, int align)
{
   static uint8_t databuf[PAGE_SIZE + 32 + FUN2_MAX_DATA_SIZE + 32];
   uint8_t *data, *prologue, *epilogue;
   VCHIQ_ELEMENT_T element;
   int success = 1, i;

   if (!vcos_verify(datalen < FUN2_MAX_DATA_SIZE))
      return VCHIQ_ERROR;

   element.size = sizeof(int);
   element.data = &datalen;
   EXPECT(vchiq_queue_message(service, &element, 1), VCHIQ_SUCCESS);

   data = (uint8_t *)((uint32_t)databuf & ~(PAGE_SIZE - 1)) + align;
   if (data < (databuf + 32))
      data += PAGE_SIZE;
   prologue = data - 32;
   epilogue = data + datalen;

   memset(prologue, 0xff, 32);
   memset(epilogue, 0xff, 32);

   for (i = 0; i < (datalen - 1); i++)
   {
      data[i] = (uint8_t)(((i & 0x1f) == 0) ? (i >> 5) : i);
   }
   data[i] = '\0';

   EXPECT(vchiq_queue_bulk_transmit(service, data, datalen, NULL), VCHIQ_SUCCESS);
   vcos_event_wait(&func_test_sync);
   memset(databuf, 0xff, sizeof(databuf));
   EXPECT(vchiq_queue_bulk_receive(service, data, datalen, NULL), VCHIQ_SUCCESS);
   vcos_event_wait(&func_test_sync);

   for (i = 0; i < 32; i++)
   {
      if (prologue[i] != '\xff')
      {
         vcos_log_error("Prologue corrupted at %d (datalen %d, align %d)", i, datalen, align);
         VCOS_BKPT;
         success = 0;
         break;
      }
      if (epilogue[i] != '\xff')
      {
         vcos_log_trace("Epilogue corrupted at %d (datalen %d, align %d)", i, datalen, align);
         VCOS_BKPT;
         success = 0;
         break;
      }
   }

   if (success)
   {
      for (i = 0; i < (datalen - 1); i++)
      {
         if ((i & 0x1f) == 0)
         {
            if (data[i] != (uint8_t)(i >> 5))
            {
               success = 0;
               break;
            }
         }
         else if (data[i] != (uint8_t)i)
         {
            success = 0;
            break;
         }
      }
      if (success && (data[i] != 0))
         success = 0;

      if (!success)
      {
         vcos_log_error("Data corrupted at %d (datalen %d, align %d)", i, datalen, align);
         VCOS_BKPT;
      }
   }

   return VCHIQ_SUCCESS;

error_exit:
   return VCHIQ_ERROR;
}


#ifdef VCHIQ_LOCAL

static void *vchiq_test_server(void *param)
{
   VCHIQ_INSTANCE_T instance;

   vcos_demand(vchiq_initialise(&instance) == VCHIQ_SUCCESS);
   vchiq_test_start_services(instance);
   vchiq_connect(instance);
   printf("test server started\n");
   return 0;
}

#endif

static VCHIQ_STATUS_T
clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
   VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata)
{
   vcos_mutex_lock(&g_mutex);
   if (reason == VCHIQ_MESSAGE_AVAILABLE)
   {
      if (header->size <= 1)
         vchiq_release_message(service, header);
      else
      /* Responses of length 0 are not sync points */
      if ((header->size >= 4) && (*(int *)header->data == MSG_ECHO))
      {
         /* This is a complete echoed packet */
         if (g_params.verify && (mem_check(header->data, bulk_tx_data[ctrl_received % NUM_BULK_BUFS], g_params.blocksize) != 0))
            g_server_error = "corrupt data";
         else
            ctrl_received++;
         if (g_server_error || (ctrl_received == g_params.iters))
            vcos_event_signal(&g_shutdown);
      }
      else if (header->size != 0)
         g_server_error = header->data;

      if (g_server_error)
         vcos_event_signal(&g_server_reply);
   }
   else if (reason == VCHIQ_BULK_TRANSMIT_DONE)
   {
      int i = (int)bulk_userdata;
      vcos_log_trace("  BULK_TRANSMIT_DONE(%d)", i);
      if (bulk_tx_received < 0)
         vcos_event_signal(&g_server_reply);
      else
      {
         vcos_assert(i == bulk_tx_received);
         bulk_tx_received++;
         if (bulk_tx_sent < g_params.iters)
         {
            vchiq_queue_bulk_transmit(service, bulk_tx_data[i % NUM_BULK_BUFS], g_params.blocksize, (void *)bulk_tx_sent);
            bulk_tx_sent++;
         }
      }
   }
   else if (reason == VCHIQ_BULK_RECEIVE_DONE)
   {
      int i = (int)bulk_userdata;
      vcos_log_trace("  BULK_RECEIVE_DONE(%d): data '%s'", i, bulk_rx_data[i % NUM_BULK_BUFS]);
      vcos_assert(i == bulk_rx_received);
      if (g_params.verify && (mem_check(bulk_tx_data[i % NUM_BULK_BUFS], bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize) != 0))
      {
         vcos_log_error("* Data corruption - %d: %x, %x, %x", i, (unsigned int)bulk_tx_data[i % NUM_BULK_BUFS], (unsigned int)bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize);
         VCOS_BKPT;
      }
      bulk_rx_received++;
      if (bulk_rx_sent < g_params.iters)
      {
         if (g_params.verify)
            memset(bulk_rx_data[i % NUM_BULK_BUFS], 0xff, g_params.blocksize);
         vchiq_queue_bulk_receive(service, bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize, (void *)bulk_rx_sent);
         bulk_rx_sent++;
      }
   }
   else if (reason == VCHIQ_BULK_TRANSMIT_ABORTED)
   {
      int i = (int)bulk_userdata;
      vcos_log_info("  BULK_TRANSMIT_ABORTED(%d)", i);
   }
   else if (reason == VCHIQ_BULK_RECEIVE_ABORTED)
   {
      int i = (int)bulk_userdata;
      vcos_log_info("  BULK_RECEIVE_ABORTED(%d)", i);
   }
   if ((bulk_tx_received == g_params.iters) &&
      ((g_params.echo == 0) || (bulk_rx_received == g_params.iters)))
      vcos_event_signal(&g_shutdown);
   vcos_mutex_unlock(&g_mutex);
   return VCHIQ_SUCCESS;
}

static void
vchi_clnt_callback(void *callback_param,
   VCHI_CALLBACK_REASON_T reason,
   void *handle)
{
   VCHI_SERVICE_HANDLE_T service = *(VCHI_SERVICE_HANDLE_T *)callback_param;
   vcos_mutex_lock(&g_mutex);
   if (reason == VCHI_CALLBACK_MSG_AVAILABLE)
   {
      if (!g_sync_mode)
      {
         static char pong_buf[100];
         uint32_t actual;
         while (vchi_msg_dequeue(service, pong_buf, sizeof(pong_buf), &actual, VCHI_FLAGS_NONE) == 0)
         {
            if (actual > 1)
               g_server_error = pong_buf;
            if (actual != 0)
               /* Responses of length 0 are not sync points */
               vcos_event_signal(&g_server_reply);
         }
      }
   }
   else if (reason == VCHI_CALLBACK_BULK_SENT)
   {
      int i = (int)handle;
      vcos_log_trace("  BULK_TRANSMIT_DONE(%d)", i);
      if (bulk_tx_received < 0)
         vcos_event_signal(&g_server_reply);
      else
      {
         vcos_assert(i == bulk_tx_received);
         bulk_tx_received++;
         if (bulk_tx_sent < g_params.iters)
         {
            vchi_bulk_queue_transmit(service, bulk_tx_data[i % NUM_BULK_BUFS],
               g_params.blocksize,
               VCHI_FLAGS_CALLBACK_WHEN_OP_COMPLETE | VCHI_FLAGS_BLOCK_UNTIL_QUEUED,
               (void *)bulk_tx_sent);
            bulk_tx_sent++;
         }
      }
   }
   else if (reason == VCHI_CALLBACK_BULK_RECEIVED)
   {
      int i = (int)handle;
      vcos_log_trace("  BULK_RECEIVE_DONE(%d): data '%s'", i, bulk_rx_data[i % NUM_BULK_BUFS]);
      vcos_assert(i == bulk_rx_received);
      if (g_params.verify && (mem_check(bulk_tx_data[i % NUM_BULK_BUFS], bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize) != 0))
      {
         vcos_log_error("* Data corruption - %x, %x, %x", (unsigned int)bulk_tx_data[i % NUM_BULK_BUFS], (unsigned int)bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize);
         VCOS_BKPT;
      }
      bulk_rx_received++;
      if (bulk_rx_sent < g_params.iters)
      {
         if (g_params.verify)
            memset(bulk_rx_data[i % NUM_BULK_BUFS], 0xff, g_params.blocksize);
         vchi_bulk_queue_receive(service, bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize,
            VCHI_FLAGS_CALLBACK_WHEN_OP_COMPLETE | VCHI_FLAGS_BLOCK_UNTIL_QUEUED,
            (void *)bulk_rx_sent);
         bulk_rx_sent++;
      }
   }
   else if (reason == VCHI_CALLBACK_BULK_TRANSMIT_ABORTED)
   {
      int i = (int)handle;
      vcos_log_info("  BULK_TRANSMIT_ABORTED(%d)", i);
   }
   else if (reason == VCHI_CALLBACK_BULK_RECEIVE_ABORTED)
   {
      int i = (int)handle;
      vcos_log_info("  BULK_RECEIVE_ABORTED(%d)", i);
   }
   if ((bulk_tx_received == g_params.iters) && (bulk_rx_received == g_params.iters))
      vcos_event_signal(&g_shutdown);
   vcos_mutex_unlock(&g_mutex);
}

static VCHIQ_STATUS_T
func_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
   VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata)
{
   static int callback_count = 0, bulk_count = 0;
   int callback_index = 0, bulk_index = 0;

   if (reason < VCHIQ_BULK_TRANSMIT_DONE)
   {
      callback_count++;

      START_CALLBACK(VCHIQ_SERVICE_CLOSED, 2)
      END_CALLBACK(VCHIQ_SUCCESS)

      START_CALLBACK(VCHIQ_MESSAGE_AVAILABLE, 1)
      EXPECT(bulk_userdata, NULL);
      EXPECT(header->size, 26);
      EXPECT(mem_check(header->data, "abcdefghijklmnopqrstuvwxyz", 26), 0);
      vchiq_release_message(service, header);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_CALLBACK(VCHIQ_MESSAGE_AVAILABLE, 1)
      EXPECT(bulk_userdata, NULL);
      EXPECT(header->size, 0);
      vchiq_release_message(service, header);
      EXPECT(vchiq_queue_bulk_receive(service, clnt_service2_data, sizeof(clnt_service2_data), (void*)0x1004), VCHIQ_SUCCESS);
      vcos_event_signal(&func_test_sync);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_CALLBACK(VCHIQ_SERVICE_CLOSED, 1)
      vcos_event_signal(&func_test_sync);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_CALLBACK(VCHIQ_SERVICE_CLOSED, 2)
      vcos_event_signal(&func_test_sync);
      callback_count = 0;
      bulk_count = 0;
      END_CALLBACK(VCHIQ_SUCCESS)
   }
   else
   {
      bulk_count++;

      START_BULK_CALLBACK(VCHIQ_BULK_TRANSMIT_DONE, 1, 0x1001)
      memset(clnt_service2_data, 0xff, sizeof(clnt_service2_data));
      EXPECT(vchiq_queue_bulk_receive(service, clnt_service2_data, sizeof(clnt_service2_data), (void*)0x1002), VCHIQ_SUCCESS);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_BULK_CALLBACK(VCHIQ_BULK_RECEIVE_ABORTED, 1, 0x1002)
      EXPECT(vchiq_queue_bulk_receive(service, clnt_service2_data, sizeof(clnt_service2_data), (void*)0x1003), VCHIQ_SUCCESS);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_BULK_CALLBACK(VCHIQ_BULK_RECEIVE_DONE, 1, 0x1003)
      (void)(mem_check(clnt_service1_data, clnt_service2_data, sizeof(clnt_service1_data)), 0);
      (void)(mem_check(clnt_service1_data, clnt_service2_data + sizeof(clnt_service1_data), sizeof(clnt_service1_data)), 0);
      END_CALLBACK(VCHIQ_SUCCESS)

      START_BULK_CALLBACK(VCHIQ_BULK_RECEIVE_ABORTED, 1, 0x1004)
      END_CALLBACK(VCHIQ_SUCCESS)

      START_BULK_CALLBACK(VCHIQ_BULK_TRANSMIT_ABORTED, 2, 0x2001)
      END_CALLBACK(VCHIQ_SUCCESS)
   }

error_exit:
   callback_count = 0;
   bulk_count = 0;

   return VCHIQ_ERROR;
}

static VCHIQ_STATUS_T
fun2_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
   VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata)
{
   vcos_unused(header);
   vcos_unused(service);
   vcos_unused(bulk_userdata);

   switch (reason)
   {
   case VCHIQ_SERVICE_OPENED:
   case VCHIQ_SERVICE_CLOSED:
      break;
   case VCHIQ_BULK_TRANSMIT_DONE:
   case VCHIQ_BULK_RECEIVE_DONE:
      vcos_event_signal(&func_test_sync);
      break;
   default:
      fun2_error = 1;
      vcos_event_signal(&func_test_sync);
      break;
   }

   return VCHIQ_SUCCESS;
}

static int mem_check(const void *expected, const void *actual, int size)
{
   if (memcmp(expected, actual, size) != 0)
   {
      int i;
      for (i = 0; i < size; i++)
      {
         int ce = ((const char *)expected)[i];
         int ca = ((const char *)actual)[i];
         if (ca != ce)
            printf("%08x,%x: %02x <-> %02x\n", i + (unsigned int)actual, i, ce, ca);
      }
      printf("mem_check failed - buffer %x, size %d\n", (unsigned int)actual, size);
      return 1;
   }
   return 0;
}

static void usage(void)
{
   printf("Usage: vchiq_test [<options>] <mode> <iters>\n");
   printf("  where <options> is any of:\n");
   printf("    -e          disable echoing in the main bulk transfer mode\n");
   printf("    -q          disable data verification\n");
   printf("    -s ????     service (any 4 characters)\n");
   printf("    -v          enable more verbose output\n");
   printf("  and <mode> is one of:\n");
   printf("    -c <size>   control test (size in bytes)\n");
   printf("    -b <size>   bulk test (size in kilobytes)\n");
   printf("    -f          functional test\n");
   printf("    -p          ping test\n");
   printf("    -t          check the timer\n");
   printf("  and <iters> is the number of test iterations\n");
   exit(1);
}

static void check_timer(void)
{
   uint32_t start = vcos_getmicrosecs();
   uint32_t sleep_time = 1000;

   printf("0\n");

   while (1)
   {
      uint32_t now;
      vcos_sleep(sleep_time);
      now = vcos_getmicrosecs();
      printf("%d - sleep %d\n", now - start, sleep_time);
   }
}

#ifdef VCOS_APPLICATION_INITIALIZE

static VCOS_THREAD_T   Task_0;

void *main_task(void *param)
{
   vchiq_test(rtos_argc, rtos_argv);

   VCOS_BKPT;

   return NULL;
}

#include "vcfw/logging/logging.h"

void VCOS_APPLICATION_INITIALIZE(void *first_available_memory)
{
   const int      stack_size = 64*1024;
   void          *pointer = NULL;
   VCOS_STATUS_T  status;

   logging_init();
   logging_level(LOGGING_VCOS);
   vcos_init();

   /* Create task 0.  */
#if VCOS_CAN_SET_STACK_ADDR
   pointer = malloc(stack_size);
   vcos_demand(pointer);
#endif
   status = vcos_thread_create_classic( &Task_0, "TASK 0", main_task, (void *)0, pointer, stack_size,
                                        10 | VCOS_AFFINITY_DEFAULT, 20, VCOS_START );
   vcos_demand(status == VCOS_SUCCESS);
}

#else

int main(int argc, char **argv)
{
   vcos_init();
   return vchiq_test(argc, argv);
}

#endif
