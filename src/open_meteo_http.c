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


#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(open_meteo, LOG_LEVEL_DBG);

/* HTTP server to connect to */
#define SERVER_HOST "api.open-meteo.com"

/* Port to connect to, as string */
#define SERVER_PORT "80"

/* HTTP path to request */
#define HTTP_PATH "/v1/forecast?latitude=41.5744&longitude=-3.2042&current=temperature_2m"

/* Build request message */
#define REQUEST "GET " HTTP_PATH " HTTP/1.0\r\nHost: " SERVER_HOST "\r\n\r\n"

#define SSTRLEN(s) (sizeof(s) - 1)
#define CHECK(r) { if (r == -1) { LOG_ERR("Error: " #r "\n"); return; } }
static char response[1024];


void dump_addrinfo(const struct addrinfo *ai)
{
	LOG_INF("addrinfo @%p: ai_family=%d, ai_socktype=%d, ai_protocol=%d, "
	       "sa_family=%d, sin_port=%x",
	       ai, ai->ai_family, ai->ai_socktype, ai->ai_protocol,
	       ai->ai_addr->sa_family,
	       ((struct sockaddr_in *)ai->ai_addr)->sin_port);
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

	dump_addrinfo(result);

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


	CHECK(send(sock, REQUEST, SSTRLEN(REQUEST), 0));

	int len = recv(sock, response, sizeof(response) - 1, 0);

	if (len < 0) {
		LOG_ERR("Error reading response");
		return;
	}

	response[len] = 0;
	LOG_INF("Response:\n%s\n", response);

	//TODO: Parse json response into variables

	(void)close(sock);
	LOG_INF("socket closed");
	return;
}
