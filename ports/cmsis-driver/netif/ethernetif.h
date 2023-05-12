#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/netif.h"

#if LWIP_CHECKSUM_CTRL_PER_NETIF
#define NETIF_CHECKSUM_GEN_ALL   (NETIF_CHECKSUM_GEN_IP     | \
                                  NETIF_CHECKSUM_GEN_UDP    | \
                                  NETIF_CHECKSUM_GEN_TCP    | \
                                  NETIF_CHECKSUM_GEN_ICMP   | \
                                  NETIF_CHECKSUM_GEN_ICMP6)

#define NETIF_CHECKSUM_CHECK_ALL (NETIF_CHECKSUM_CHECK_IP   | \
                                  NETIF_CHECKSUM_CHECK_UDP  | \
                                  NETIF_CHECKSUM_CHECK_TCP  | \
                                  NETIF_CHECKSUM_CHECK_ICMP | \
                                  NETIF_CHECKSUM_CHECK_ICMP6)
#endif /* LWIP_CHECKSUM_CTRL_PER_NETIF */

err_t ethernetif_init(struct netif *netif);
void  ethernetif_poll(struct netif *netif);
void  ethernetif_check_link (struct netif *netif);

#endif
