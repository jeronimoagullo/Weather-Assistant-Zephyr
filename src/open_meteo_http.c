/**
 * @file open_meteo_http.c
 * @author Jeronimo Agullo (jeronimoagullo97@gmail.com)
 * @brief This file contains the function to request a GET HTTP to open-meteo by posix socket
 * By the Zephyr's BSD Socket API
 * For more information about socket in Zephyr: https://academy.nordicsemi.com/courses/cellular-iot-fundamentals/lessons/lesson-3-cellular-fundamentals/topic/socket-api/

 * @version 0.1
 * @date 2024-01-29
 *
 * @copyright Copyright (c) 2024
 *
 */


#include <netinet/in.h>
#include <sys/socket.h>
#include <zephyr/data/json.h>
#include <zephyr/net/http/client.h>


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(open_meteo, LOG_LEVEL_DBG);

#include "open_meteo_http.h"

/* HTTP server to connect to */
#define SERVER_HOST "api.open-meteo.com"

/* Port to connect to, as string */
#define SERVER_PORT "80"

/* HTTP path to request */
#define HTTP_PATH "/v1/forecast?latitude=52.52&longitude=13.41&current=temperature_2m,relative_humidity_2m"

#define SSTRLEN(s) (sizeof(s) - 1)
#define CHECK(r) { if (r == -1) { LOG_ERR("Error: " #r "\n"); return; } }

#define RESP_OFFSET 56
#define RESP_LEN (RESP_OFFSET - 9)

static uint8_t recv_buf_ipv4[NET_IPV4_MTU];

K_MSGQ_DEFINE(weather_q, sizeof(struct weather_t), 2, 1);

static int weather_parse_data(uint8_t *weather_update, struct weather_t *weather_data)
{
    int ret;

    memset(weather_data, 0, sizeof(struct weather_t));

    ret = sscanf((char *)weather_update,
                 "\"temperature_2m\":%f,\"relative_humidity_2m\":%f",
                 &weather_data->temperature,
                 &weather_data->humidity);

    if (ret != 2) {
        printf("Failed to parse data.\n");
    }

	return ret;
}

static int http_response_cb(struct http_response *rsp,
		       enum http_final_call final_data,
		       void *user_data)
{
	char weather_update[RESP_LEN];
	struct weather_t weather_data;

	uint16_t weather_offset = rsp->body_frag_len - RESP_OFFSET;

	memcpy(weather_update, rsp->body_frag_start + weather_offset, RESP_LEN);
	weather_update[RESP_LEN] = '\0';

	weather_parse_data(weather_update, &weather_data);

	LOG_INF("Temperature: %.1f°C", weather_data.temperature);
	LOG_INF("Relative Humidity: %.1f%%", weather_data.humidity);

	if (k_msgq_put(&weather_q, &weather_data, K_NO_WAIT) != 0) {
		LOG_ERR("Failed to put new input event");
	}

	return 0;
}

void http_get_open_meteo_forcast(void){
	int ret = 0;
	int sock;

	struct addrinfo *result;
	static struct addrinfo hints = {
	.ai_family = AF_INET,
	.ai_socktype = SOCK_STREAM
	};

	LOG_INF("Preparing HTTP GET request for http://" SERVER_HOST ":" SERVER_PORT HTTP_PATH "");

	ret = getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &result);
	LOG_INF("getaddrinfo status: %d", ret);

	if (ret != 0) {
		LOG_ERR("Unable to resolve address, quitting");
		return;
	}

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	CHECK(sock);
	LOG_INF("sock = %d", sock);

	ret = connect(sock, result->ai_addr, result->ai_addrlen);

	if (ret < 0) {
		LOG_ERR("Cannot connect to socket remote: %d",-errno);
		return;
	} else {
		LOG_INF("Connection successed");
	}

	struct http_request req;
	memset(&req, 0, sizeof(req));

	req.method = HTTP_GET;
	req.url = HTTP_PATH;
	req.host = SERVER_HOST;
	req.protocol = "HTTP/1.1";
	req.response = http_response_cb;
	req.recv_buf = recv_buf_ipv4;
	req.recv_buf_len = sizeof(recv_buf_ipv4);

	ret = http_client_req(sock, &req, CONFIG_NET_SOCKETS_DNS_TIMEOUT, "GET");
	if (ret < 0) {
		LOG_ERR("Client error %d", ret);
	}

	close(sock);

	LOG_INF("socket closed");
	return;
}
