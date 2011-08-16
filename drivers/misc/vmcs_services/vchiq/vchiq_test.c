/*=============================================================================
Copyright (c) 2010 Broadcom Europe Ltd. All rights reserved.

Project  :  ARM
Module   :  vchiq_arm

FILE DESCRIPTION:
A simple test for vchiq.

==============================================================================*/

#ifndef __KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <asm/uaccess.h> 
#endif

//#include "vcfw/logging/logging.h"

#include "vchiq_test.h"

#define NUM_BULK_BUFS 4
#define BULK_SIZE (1024*256)
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifdef __KERNEL__
#define malloc(x) kmalloc(GFP_KERNEL,x)
#define free(x) kfree(x)
#define atoi(x) simple_strtol(x,NULL,10)
#define printf(...) printk(KERN_INFO __VA_ARGS__)
#define exit(x) return x
#define fprintf(x,...) printf(__VA_ARGS__) 
#endif

VCHIQ_STATUS_T vchiq_shutdown(VCHIQ_INSTANCE_T instance)
{
  return 1;
}

struct test_params
{
   int blocksize;
   int iters;
};

static struct test_params g_params = { 64, 100 };

static VCOS_EVENT_T g_server_reply;
static VCOS_EVENT_T g_shutdown;
static VCOS_MUTEX_T g_mutex;

static const char *g_server_error = NULL;

static VCOS_EVENT_T func_test_sync;
static int run_functional_test = 0;
static int fourcc = 0;
static int fun2_error = 0;

char *bulk_tx_data[NUM_BULK_BUFS];
char *bulk_rx_data[NUM_BULK_BUFS];

static int bulk_tx_sent = 0;
static int bulk_rx_sent = 0;
static int bulk_tx_received = 0;
static int bulk_rx_received = 0;

static char clnt_service1_data[SERVICE1_DATA_SIZE];
static char clnt_service2_data[SERVICE2_DATA_SIZE];

#ifdef VCHIQ_LOCAL
static void *vchiq_test_server(void *);
#endif
static VCHIQ_STATUS_T clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                    VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static VCHIQ_STATUS_T vchiq_functional_test(void);
static VCHIQ_STATUS_T func_data_test(VCHIQ_SERVICE_HANDLE_T service, int size, int align);
static VCHIQ_STATUS_T func_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                         VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static VCHIQ_STATUS_T fun2_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                         VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata);
static void usage(void);


int vchiq_test(int argc, char **argv)
{
   VCHIQ_ELEMENT_T elements[4];
   VCHIQ_ELEMENT_T *element;
   VCHIQ_INSTANCE_T vchiq_instance;
   VCHIQ_SERVICE_HANDLE_T vchiq_service;
   const char *servname = "echo";
   int argn;
   int i;

   goto done_parsing;

   argn = 1;
   while ((argn < argc) && (argv[argn][0] == '-'))
   {
      const char *arg = argv[argn++];
      if (strcmp(arg, "-s") == 0)
      {
         servname = argv[argn++];
         if (!servname || (strlen(servname) != 4))
         {
            usage();
         }
      }
      else if (strcmp(arg, "-b") == 0)
      {
         g_params.blocksize = atoi(argv[argn++]);
      }
      else if (strcmp(arg, "-h") == 0)
      {
         usage();
      }
      else if (strcmp(arg, "-f") == 0)
      {
         run_functional_test = 1;
      }
      else
      {
         printf("* unknown option '%s'\n", arg);
         usage();
      }
   }

   if (argn < argc)
   {
      g_params.iters = atoi(argv[argn++]);
      if (argn != argc)
      {
         usage();
      }
   }

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

   if (run_functional_test)
   {
      int i;
      for (i = 0; i < g_params.iters; i++)
      {
         VCOS_TRACE("======== vchiq_test iteration %d ========", i+1);

         if (vchiq_functional_test() != VCHIQ_SUCCESS)
            return -1;
      }
      return 0;
   }
done_parsing:
   printf("Service:'%s', block size:%dKB, iters:%d\n", servname, g_params.blocksize, g_params.iters);

   g_params.blocksize *= 1024;
   fourcc = VCHIQ_MAKE_FOURCC(servname[0], servname[1], servname[2], servname[3]);

   for (i = 0; i < NUM_BULK_BUFS; i++)
   {
      bulk_tx_data[i] = malloc(g_params.blocksize);
      bulk_rx_data[i] = malloc(g_params.blocksize);
      if (!bulk_tx_data[i] || !bulk_rx_data[i])
      {
         printf("* out of memory\n");
         exit(1);
      }
   }

   vcos_event_create(&g_server_reply, "g_server_reply");
   vcos_event_create(&g_shutdown, "g_shutdown");
   vcos_mutex_create(&g_mutex, "g_mutex");

//   fprintf(stderr, "vchiq_test: opening vchiq\n");

   if (vchiq_initialise(&vchiq_instance) != VCHIQ_SUCCESS)
   {
      printf("* failed to open vchiq instance\n");
      return -1;
   }

//   fprintf(stderr, "vchiq_test: connecting\n");

   vchiq_connect(vchiq_instance);

//   fprintf(stderr, "vchiq_test: connected\n");

   if (vchiq_open_service(vchiq_instance, fourcc, clnt_callback, "clnt userdata", &vchiq_service) != VCHIQ_SUCCESS)
   {
      printf("* failed to open service - already in use?\n");
      exit(1);
   }

   element = elements;
   element->data = &g_params;
   element->size = sizeof(g_params);
   element++;

   vchiq_queue_message(vchiq_service, elements, element - elements);

   vcos_event_wait(&g_server_reply);

   if (g_server_error)
   {
      printf("* server error: %s\n", g_server_error);
      exit(1);
   }

   vcos_mutex_lock(&g_mutex);

   for (i = 0; i < NUM_BULK_BUFS; i++)
   {
      sprintf(bulk_tx_data[i], "%d: The quick brown fox jumped over the lazy dog\n", bulk_tx_sent);

      vchiq_queue_bulk_transmit(vchiq_service, bulk_tx_data[i], g_params.blocksize, (void *)i);

//      fprintf(stderr, "vchiq_test: queued bulk tx %d\n", i);
      bulk_tx_sent++;

      vchiq_queue_bulk_receive(vchiq_service, bulk_rx_data[i], g_params.blocksize, (void *)i);

//      fprintf(stderr, "vchiq_test: queued bulk rx %d\n", i);
      bulk_rx_sent++;
   }

   vcos_mutex_unlock(&g_mutex);

//   fprintf(stderr, "Sent all messages\n");

//   fprintf(stderr, "vchiq_test: waiting for shutdown\n");

   vcos_event_wait(&g_shutdown);

   vchiq_remove_service(vchiq_service);

//   fprintf(stderr, "vchiq_test: shutting down\n");

   vchiq_shutdown(vchiq_instance);

   return 0;
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

VCHIQ_STATUS_T clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                             VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata)
{
   //printf("Callback: %d, %x, %x, %x\n", reason, header, service_userdata, bulk_userdata);
   vcos_mutex_lock(&g_mutex);
   if (reason == VCHIQ_MESSAGE_AVAILABLE)
   {
      if (header->size == 0)
         vchiq_release_message(service, header);
      else
         g_server_error = header->data;
      vcos_event_signal(&g_server_reply);
   }
   else if (reason == VCHIQ_BULK_TRANSMIT_DONE)
   {
      int i = (int)bulk_userdata;
//      fprintf(stderr, "  BULK_TRANSMIT_DONE(%d)\n", i);
      vcos_assert(i == bulk_tx_received);
      bulk_tx_received++;
      if (bulk_tx_sent < g_params.iters)
      {
         sprintf(bulk_tx_data[i % NUM_BULK_BUFS], "%d: The quick brown fox jumped over the lazy dog\n", bulk_tx_sent);
         vchiq_queue_bulk_transmit(service, bulk_tx_data[i % NUM_BULK_BUFS], g_params.blocksize, (void *)bulk_tx_sent);
         bulk_tx_sent++;
      }
   }
   else if (reason == VCHIQ_BULK_RECEIVE_DONE)
   {
      int i = (int)bulk_userdata;
//      fprintf(stderr, "  BULK_RECEIVE_DONE(%d): data '%s'\n", i, bulk_rx_data[i % NUM_BULK_BUFS]);
      vcos_assert(i == bulk_rx_received);
      bulk_rx_received++;
      if (bulk_rx_sent < g_params.iters)
      {
         vchiq_queue_bulk_receive(service, bulk_rx_data[i % NUM_BULK_BUFS], g_params.blocksize, (void *)bulk_rx_sent);
         bulk_rx_sent++;
      }
   }
   else if (reason == VCHIQ_BULK_TRANSMIT_ABORTED)
   {
      int i = (int)bulk_userdata;
      fprintf(stderr, "  BULK_TRANSMIT_ABORTED(%d)\n", i);
   }
   else if (reason == VCHIQ_BULK_RECEIVE_ABORTED)
   {
      int i = (int)bulk_userdata;
      fprintf(stderr, "  BULK_RECEIVE_ABORTED(%d)\n", i);
   }
   if ((bulk_tx_received == g_params.iters) && (bulk_rx_received == g_params.iters))
      vcos_event_signal(&g_shutdown);
   vcos_mutex_unlock(&g_mutex);
   return VCHIQ_SUCCESS;
}

static VCHIQ_STATUS_T vchiq_functional_test(void)
{
   VCHIQ_ELEMENT_T elements[4];
   VCHIQ_INSTANCE_T instance;
   VCHIQ_SERVICE_HANDLE_T service, service2, service3;
   int size, i;

   vcos_event_create(&func_test_sync, "test_sync");

   EXPECT(vchiq_initialise(&instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, func_clnt_callback, (void *)1, &service), VCHIQ_SUCCESS);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, func_clnt_callback, (void *)2, &service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, clnt_callback, (void *)3, &service3), VCHIQ_ERROR); // callback doesn't match
   EXPECT(vchiq_remove_service(service), VCHIQ_SUCCESS);
   EXPECT(vchiq_remove_service(service), VCHIQ_ERROR); // service already removed
   EXPECT(vchiq_remove_service(service2), VCHIQ_SUCCESS);
   EXPECT(vchiq_queue_message(service, NULL, 0), VCHIQ_ERROR); // service not valid
   EXPECT(vchiq_add_service(instance, FUNC_FOURCC, clnt_callback, (void *)3, &service3), VCHIQ_SUCCESS);
   EXPECT(vchiq_queue_message(service, NULL, 0), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_queue_bulk_transmit(service, clnt_service1_data, sizeof(clnt_service1_data), (void *)1), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_queue_bulk_receive(service2, clnt_service2_data, sizeof(clnt_service2_data), (void *)2), VCHIQ_ERROR); // service not open
   EXPECT(vchiq_shutdown(instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_initialise(&instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)1, &service), VCHIQ_ERROR); // not connected
   EXPECT(vchiq_connect(instance), VCHIQ_SUCCESS);
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)1, &service), VCHIQ_ERROR); // rejected
   EXPECT(vchiq_open_service(instance, FUNC_FOURCC, func_clnt_callback, (void*)1, &service), VCHIQ_SUCCESS);
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


static VCHIQ_STATUS_T func_data_test(VCHIQ_SERVICE_HANDLE_T service, int datalen, int align)
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
         VCOS_TRACE("Prologue corrupted at %d (datalen %d)", i, datalen);
         VCOS_BKPT;
         success = 0;
         break;
      }
      if (epilogue[i] != '\xff')
      {
         VCOS_TRACE("Epilogue corrupted at %d (datalen %d)", i, datalen);
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
         VCOS_TRACE("Data corrupted at %d (datalen %d)", i, datalen);
         VCOS_BKPT;
      }
   }

   return VCHIQ_SUCCESS;

error_exit:
   return VCHIQ_ERROR;
}


static VCHIQ_STATUS_T func_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
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
      EXPECT(memcmp(header->data, "abcdefghijklmnopqrstuvwxyz", 26), 0);
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
      EXPECT(memcmp(clnt_service1_data, clnt_service2_data, sizeof(clnt_service1_data)), 0);
      EXPECT(memcmp(clnt_service1_data, clnt_service2_data + sizeof(clnt_service1_data), sizeof(clnt_service1_data)), 0);
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

static VCHIQ_STATUS_T fun2_clnt_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                                         VCHIQ_SERVICE_HANDLE_T service, void *bulk_userdata)
{
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

static void usage(void)
{
   //printf("Usage: %s [<options>] <iters>\n");
   printf("  where <iters> is the number of test iterations\n");
   printf("  and <options> is any of:\n");
   printf("    -b <size> (in kilobytes)\n");
   printf("    -s ???? (any 4 characters)\n");
   //exit(1);
}

#ifdef VCOS_APPLICATION_INITIALIZE

static VCOS_THREAD_T   Task_0;

void *main_task(void *param)
{
   vchiq_test(rtos_argc, rtos_argv);

   VCOS_BKPT;

   return NULL;
}

void VCOS_APPLICATION_INITIALIZE(void *first_available_memory)
{
   const int      stack_size = 64*1024;
   void          *pointer = NULL;
   VCOS_STATUS_T  status;

   logging_init();
   logging_level(LOGGING_VMCS);

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



#ifdef __KERNEL__

/*
 *
 */
static int mydev_proc_test_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
  int len = 0;
  
  if (offset > 0)
	{	
	  *eof = 1;	  
	  return 0;	  
	}
  len += sprintf(buf + len," mydev_proc_ogltest_read\n");
  *eof = 1;
  return len;
}

static int mydev_proc_test_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
  char testnum_str[21];
  int testnum = 0;
  
  count = count> 20 ? 20 : count;
  if(copy_from_user(&testnum_str, buffer, count))
    {
	  return -EFAULT;
    }
  if(1 != sscanf(testnum_str, "%d", &testnum))
    {
      printk(KERN_ALERT "mydev_proc_fbtest_write: invalid parameter: %s", testnum_str);
      return count;
    }
  
  printk(KERN_INFO "calling vchiq_test(%d)\n",testnum);

  switch(testnum)
  {
    default:
      vchiq_test(0,NULL);
  }

  return count;
}


int mydev_proc_init(void)
{
  struct proc_dir_entry *mydev_procdir_test;
        
   //
   // /proc/VC03_ogltest
   //
   mydev_procdir_test = create_proc_entry("mydev_test", 0644, NULL);
	
   if (mydev_procdir_test == NULL)
	 {
	   remove_proc_entry("mydev_test", NULL);
	   printk(KERN_ALERT "could not initialize /proc/mydev_test");
	   return -ENOMEM;
	 }

   mydev_procdir_test->read_proc  = mydev_proc_test_read;
   mydev_procdir_test->write_proc = mydev_proc_test_write;
   //mydev_procdir_test->owner 	  = THIS_MODULE;
   mydev_procdir_test->mode 	  = S_IFREG | S_IRUGO;
   mydev_procdir_test->uid 	  = 0;
   mydev_procdir_test->gid 	  = 0;
   mydev_procdir_test->size 	  = 37;
      
   return 0;
}

#endif
