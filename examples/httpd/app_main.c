/*---------------------------------------------------------------------------
 * Copyright (c) 2019-2024 Arm Limited (or its affiliates).
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *---------------------------------------------------------------------------*/

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"

#include <stdio.h>
#include <stdint.h>

#include "ethernetif.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/apps/httpd.h"

static struct netif netif;

/* Initialize lwIP */
static void net_init (void) {
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  lwip_init();

#if LWIP_DHCP
  ipaddr.addr  = IPADDR_ANY;
  netmask.addr = IPADDR_ANY;
  gw.addr      = IPADDR_ANY;
#else
  IP4_ADDR(&ipaddr,  192,168,0,10);
  IP4_ADDR(&netmask, 255,255,255,0);
  IP4_ADDR(&gw,      192,168,0,1);
#endif

  /* Add your network interface to the netif_list. */
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /* Register the default network interface. */
  netif_set_default(&netif);
  netif_set_up(&netif);

#if LWIP_DHCP
  dhcp_start (&netif);
#endif
}

/* Link check and LED blink */
static void net_periodic (uint32_t tick) {
  static ip4_addr_t ip = {1};
  static uint32_t old_tick = 0U;

  if (tick == old_tick) {
    return;
  }
  old_tick = tick;

  ethernetif_check_link(&netif);
  if (netif_is_link_up(&netif)) {
    /* Print IP address on terminal */
    if (ip.addr != netif.ip_addr.addr) {
      ip.addr = netif.ip_addr.addr;
      printf("IP: %s\n", ipaddr_ntoa(&ip));
    }
  } else {
    if (ip.addr != IPADDR_ANY) {
      ip.addr = IPADDR_ANY;
      printf("Link Down...\n");
    }
  }
}

/* Tick timer callback */
static void net_timer (uint32_t *tick) {
  *tick += 1U;
}

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/

__NO_RETURN static void app_main_thread (void *argument) {
  osTimerId_t id;
  uint32_t tick;
  (void)argument;

  /* Create tick timer, tick interval = 500ms */
  id = osTimerNew((osTimerFunc_t)&net_timer, osTimerPeriodic, &tick, NULL);
  osTimerStart(id, 500U);

  net_init();
  httpd_init();

  /* Infinite loop */
  while (1) {
    /* check if any packet received */
    ethernetif_poll(&netif);
    /* handle system timers for lwIP */
    sys_check_timeouts();
    net_periodic(tick);
  }
}

int app_main (void) {

  osKernelInitialize();                         // Initialize CMSIS-RTOS
  osThreadNew(app_main_thread, NULL, NULL);     // Create application main thread
  osKernelStart();                              // Start thread execution
  return 0;
}
