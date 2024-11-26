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
LOG_MODULE_REGISTER(wifi, LOG_LEVEL_INF);

static struct k_sem net_cb_sem;
static struct net_mgmt_event_callback mgmt_cb;

#define NET_EVENT_WIFI_MASK							\
	(NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

static void handler(struct net_mgmt_event_callback *cb,
		    uint32_t mgmt_event,
		    struct net_if *iface)
{
	char buf[NET_IPV4_ADDR_LEN];

	switch (mgmt_event) {
		case NET_EVENT_WIFI_CONNECT_RESULT:
			LOG_INF("Connected to %s", WIFI_SSID);

			LOG_INF("  IP adress: %s", net_addr_ntop(AF_INET,
					&iface->config.ip.ipv4->unicast[0].ipv4.address.in_addr,
					buf, sizeof(buf)));
			LOG_INF("  Subnet: %s", net_addr_ntop(AF_INET,
					&iface->config.ip.ipv4->unicast[0].netmask,
					buf, sizeof(buf)));
			LOG_INF("  Router: %s",	net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw,
					buf, sizeof(buf)));

			k_sem_give(&net_cb_sem);
			break;
		case NET_EVENT_WIFI_DISCONNECT_RESULT:
			LOG_INF("Disconnected from %s", WIFI_SSID);
			break;
	}
}


void init_wifi(){
	k_sem_init(&net_cb_sem, 0, 1);

	LOG_INF("STARTING WIFI...");

	struct net_if *iface = net_if_get_default();

	if(iface == NULL){
		LOG_ERR("iface is NULL");
	}

	net_mgmt_init_event_callback(&mgmt_cb, handler,
				     NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&mgmt_cb);

	static struct wifi_connect_req_params wifi_conn_params;

	// Wifi parameters
	wifi_conn_params.ssid = WIFI_SSID;
	wifi_conn_params.ssid_length = strlen(WIFI_SSID);
	wifi_conn_params.psk  = WIFI_PSK;
	wifi_conn_params.psk_length  = strlen(WIFI_PSK);
	wifi_conn_params.channel = WIFI_CHANNEL_ANY;
	wifi_conn_params.security = WIFI_SECURITY_TYPE_PSK ;
	wifi_conn_params.band = WIFI_FREQ_BAND_2_4_GHZ;

	LOG_INF("Connecting to SSID: %s\n", wifi_conn_params.ssid);

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