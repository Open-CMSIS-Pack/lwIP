#include "RTE_Components.h"
#include CMSIS_device_header
#include "stm32f2xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
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

#include "Board_LED.h"
#include "Board_GLCD.h"
#include "GLCD_Config.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

static void net_init (void);
static void net_periodic (uint32_t tick);
static void net_timer (uint32_t *tick);

static struct netif netif;

/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

/// System Clock Configuration
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
}

static void net_init (void) {
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  lwip_init ();

#if LWIP_DHCP
  ipaddr.addr  = IPADDR_ANY;
  netmask.addr = IPADDR_ANY;
  gw.addr      = IPADDR_ANY;
#else
  IP4_ADDR (&ipaddr,  192,168,0,10);
  IP4_ADDR (&netmask, 255,255,255,0);
  IP4_ADDR (&gw,      192,168,0,1);
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
  static uint32_t old_tick;
  static uint8_t LEDs;
  char buff[24];

  if (tick == old_tick) {
    return;
  }
  old_tick = tick;

  ethernetif_check_link (&netif);
  if (netif_is_link_up(&netif)) {
    LEDs |= 0x80;
    /* Print IP address on LCD display */
    if (ip.addr != netif.ip_addr.addr) {
      ip.addr = netif.ip_addr.addr;
      sprintf (buff, "IP: %-16s", ipaddr_ntoa(&ip));
      GLCD_DrawString (0, 4*24, buff);
    }
  }
  else {
    LEDs &= ~0x80;
    if (ip.addr != IPADDR_ANY) {
      ip.addr = IPADDR_ANY;
      GLCD_DrawString (0, 4*24, "Link Down...        ");
    }
  }
  LEDs ^= 0x01;
  LED_SetOut (LEDs);
}

/* Tick timer callback */
static void net_timer (uint32_t *tick) {
  ++*tick;
}

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {
  static uint32_t tick;
  osTimerId_t id;

  LED_Initialize ();

  GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
  GLCD_DrawString         (0, 1*24, "MDK and lwIP");
  GLCD_DrawString         (0, 2*24, "HTTP demo example");

  /* Create tick timer, tick interval = 500ms */
  id = osTimerNew ((osTimerFunc_t)&net_timer, osTimerPeriodic, &tick, NULL);
  osTimerStart (id, 500);

  net_init ();
  httpd_init ();

  /* Infinite loop */
  while (1) {
    /* check if any packet received */
    ethernetif_poll (&netif);
    /* handle system timers for lwIP */
    sys_check_timeouts ();
    net_periodic (tick);
  }
}

int main (void) {
  SystemClock_Config ();
  // System Initialization
  SystemCoreClockUpdate();

  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
