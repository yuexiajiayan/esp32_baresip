/**
 * @file posix/pif.c  POSIX network interface code
 *
 * Copyright (C) 2010 Creytiv.com
 */#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "re_types.h"
#include "re_fmt.h"
#include "re_mbuf.h"
#include "re_sa.h"
#include "re_net.h"
#include "re_dbg.h"

#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_netif_types.h"
#include "lwip/netif.h"      // lwIP netif struct
#include "lwip/ip_addr.h"

/**
 * Get IP address for a given network interface
 *
 * @param ifname  Network interface name
 * @param af      Address Family
 * @param ip      Returned IP address
 *
 * @return 0 if success, otherwise errorcode
 *
 * @deprecated Works for IPv4 only
 */ 
int net_if_getaddr4(const char *ifname, int af, struct sa *ip)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey(ifname);
    if (!netif) {
        DEBUG_WARNING("interface %s not found\n", ifname);
        return ENXIO;
    }

    esp_netif_ip_info_t ip_info;
    esp_err_t err = esp_netif_get_ip_info(netif, &ip_info);
    if (err != ESP_OK) {
        DEBUG_WARNING("failed to get IP for interface %s (%d)\n", ifname, err);
        return EADDRNOTAVAIL;
    }

    if (af == AF_INET) {
        // 将 IPv4 地址转换为字符串形式
        char addr_str[16];
        inet_ntoa_r(ip_info.ip.addr, addr_str, sizeof(addr_str));
        sa_set_str(ip, addr_str, 0);
        return 0;
    }

    return EAFNOSUPPORT;
}

/**
 * Enumerate all network interfaces
 *
 * @param ifh Interface handler
 * @param arg Handler argument
 *
 * @return 0 if success, otherwise errorcode
 *
 * @deprecated Works for IPv4 only
 */int net_if_list(net_ifaddr_h *ifh, void *arg)
{
    struct netif *netif;

    LWIP_ASSERT_CORE_LOCKED();

    NETIF_FOREACH(netif) {
        const char *ifname = netif->hostname;
        ip4_addr_t *ipaddr = &netif->ip_addr.u_addr.ip4;

        if (!ip4_addr_isany_val(*ipaddr)) {
            struct sa sa;
            sa_set_str(&sa, ip4addr_ntoa(ipaddr), 0);

            if (ifh && ifh(ifname, &sa, arg))
                break;
        }
    }

    return 0;
}