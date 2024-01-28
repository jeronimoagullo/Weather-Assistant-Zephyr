/**
 * @file wifi.c
 * @author Jeronimo Agullo (jeronimoagullo97@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wifi, LOG_LEVEL_DBG);

static struct k_sem net_cb_sem;
static struct net_mgmt_event_callback mgmt_cb;

static void handler(struct net_mgmt_event_callback *cb,
		    uint32_t mgmt_event,
		    struct net_if *iface)
{
	int i = 0;

	if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
		return;
	}

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
		char buf[NET_IPV4_ADDR_LEN];

		if (iface->config.ip.ipv4->unicast[i].addr_type !=
							NET_ADDR_DHCP) {
			continue;
		}

		LOG_INF("   Address[%d]: %s", net_if_get_by_iface(iface),
			net_addr_ntop(AF_INET,
			    &iface->config.ip.ipv4->unicast[i].address.in_addr,
						  buf, sizeof(buf)));
		LOG_INF("    Subnet[%d]: %s", net_if_get_by_iface(iface),
			net_addr_ntop(AF_INET,
				       &iface->config.ip.ipv4->netmask,
				       buf, sizeof(buf)));
		LOG_INF("    Router[%d]: %s", net_if_get_by_iface(iface),
			net_addr_ntop(AF_INET,
						 &iface->config.ip.ipv4->gw,
						 buf, sizeof(buf)));
		LOG_INF("Lease time[%d]: %u seconds", net_if_get_by_iface(iface),
			iface->config.dhcpv4.lease_time);
	}

	// Connected
	k_sem_give(&net_cb_sem);
}


void init_wifi(){
	k_sem_init(&net_cb_sem, 0, 1);

	LOG_INF("STARTING WIFI...");

	struct net_if *iface = net_if_get_default();

	if(iface == NULL){
		LOG_ERR("iface is NULL");
	}

	net_mgmt_init_event_callback(&mgmt_cb, handler,
				     NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&mgmt_cb);

	static struct wifi_connect_req_params wifi_conn_params;

    // Wifi parameters
	wifi_conn_params.ssid = "<your_ssid>";
	wifi_conn_params.ssid_length = strlen("<your_ssid>");
	wifi_conn_params.psk  = "<your_password>";
	wifi_conn_params.psk_length  = strlen("<your_password>");
	wifi_conn_params.channel = WIFI_CHANNEL_ANY;
	wifi_conn_params.security = WIFI_SECURITY_TYPE_PSK ;

	if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
			&wifi_conn_params, sizeof(struct wifi_connect_req_params))) {
		LOG_ERR("Connection failed");
		return;
	} else {
		LOG_INF("Connection requested...");
		// Wait for connection.....
		k_sem_take(&net_cb_sem, K_FOREVER );
	}

	LOG_INF("Connection successed");


}