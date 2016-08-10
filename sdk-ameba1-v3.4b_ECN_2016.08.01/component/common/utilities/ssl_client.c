#include "FreeRTOS.h"
#include "task.h"
#include "polarssl/config.h"

#include <string.h>
#include <stdio.h>

#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/error.h"
#include "polarssl/memory.h"

#define SERVER_PORT   443
#define SERVER_HOST   "192.168.1.218"
#define GET_REQUEST   "GET / HTTP/1.0\r\n\r\n"
#define DEBUG_LEVEL   0
#define SSL_USE_SRP   0
#define STACKSIZE     1150

static int is_task = 0;
char server_host[16];
#if SSL_USE_SRP
char srp_username[16];
char srp_password[16];
#endif

static void my_debug(void *ctx, int level, const char *str)
{
	if(level <= DEBUG_LEVEL) {
		printf("\n\r%s", str);
	}
}

static unsigned int arc4random(void)
{
	unsigned int res = xTaskGetTickCount();
	static unsigned int seed = 0xDEADB00B;

	seed = ((seed & 0x007F00FF) << 7) ^
		((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
		(res << 13) ^ (res >> 9);    // using the clock too!

	return seed;
}

static void get_random_bytes(void *buf, size_t len)
{
	unsigned int ranbuf;
	unsigned int *lp;
	int i, count;
	count = len / sizeof(unsigned int);
	lp = (unsigned int *) buf;

	for(i = 0; i < count; i ++) {
		lp[i] = arc4random();  
		len -= sizeof(unsigned int);
	}

	if(len > 0) {
		ranbuf = arc4random();
		memcpy(&lp[i], &ranbuf, len);
	}
}

static int my_random(void *p_rng, unsigned char *output, size_t output_len)
{
	get_random_bytes(output, output_len);
	return 0;
}

static size_t min_heap_size = 0;

void* my_malloc(size_t size)
{
	void *ptr = pvPortMalloc(size);
	size_t current_heap_size = xPortGetFreeHeapSize();

	if((current_heap_size < min_heap_size) || (min_heap_size == 0))
		min_heap_size = current_heap_size;

	return ptr;
}
#define my_free		vPortFree

static void ssl_client(void *param)
{
	int ret, len, server_fd = -1;
	unsigned char buf[512];
	ssl_context ssl;

	memory_set_own(my_malloc, my_free);
	/*
	 * 0. Initialize the session data
	 */
	memset(&ssl, 0, sizeof(ssl_context));

	/*
	 * 1. Start the connection
	 */
	printf("\n\r  . Connecting to tcp/%s/%d...", server_host, SERVER_PORT);

	if((ret = net_connect(&server_fd, server_host, SERVER_PORT)) != 0) {
		printf(" failed\n\r  ! net_connect returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	 * 2. Setup stuff
	 */
	printf("\n\r  . Setting up the SSL/TLS structure..." );

	if((ret = ssl_init(&ssl)) != 0) {
		printf(" failed\n\r  ! ssl_init returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, my_random, NULL);
#ifdef POLARSSL_DEBUG_C
	debug_set_threshold(DEBUG_LEVEL);
#endif
	ssl_set_dbg(&ssl, my_debug, NULL);
	ssl_set_bio(&ssl, net_recv, &server_fd, net_send, &server_fd);
#if SSL_USE_SRP
	if(strlen(srp_username))
		ssl_set_srp(&ssl, srp_username, strlen(srp_username), srp_password, strlen(srp_password));
#endif
	/*
	 * 3. Handshake
	 */
	printf("\n\r  . Performing the SSL/TLS handshake...");

	while((ret = ssl_handshake(&ssl)) != 0) {
		if(ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE) {
			printf(" failed\n\r  ! ssl_handshake returned -0x%x\n", -ret);
			goto exit;
		}
	}

	printf(" ok\n");
	printf("\n\r  . Use ciphersuite %s\n", ssl_get_ciphersuite(&ssl));

	/*
	 * 4. Write the GET request
	 */
	printf("\n\r  > Write to server:");

	len = sprintf((char *) buf, GET_REQUEST);

	while((ret = ssl_write(&ssl, buf, len)) <= 0) {
		if(ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE) {
			printf(" failed\n\r  ! ssl_write returned %d\n", ret);
			goto exit;
		}
	}

	len = ret;
	printf(" %d bytes written\n\r\n\r%s\n", len, (char *) buf);

	/*
	 * 5. Read the HTTP response
	 */
	printf("\n\r  < Read from server:");

	do {
		len = sizeof(buf) - 1;
		memset(buf, 0, sizeof(buf));
		ret = ssl_read(&ssl, buf, len);

		if(ret == POLARSSL_ERR_NET_WANT_READ || ret == POLARSSL_ERR_NET_WANT_WRITE)
			continue;

		if(ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
			break;

		if(ret < 0) {
			printf(" failed\n\r  ! ssl_read returned %d\n", ret);
			break;
		}

		if(ret == 0) {
			printf("\n\rEOF\n");
			break;
		}

		len = ret;
		printf(" %d bytes read\n\r\n\r%s\n", len, (char *) buf);
	}
	while(1);

	ssl_close_notify(&ssl);

exit:

#ifdef POLARSSL_ERROR_C
	if(ret != 0) {
		char error_buf[100];
		polarssl_strerror(ret, error_buf, 100);
		printf("\n\rLast error was: %d - %s\n", ret, error_buf);
	}
#endif

	net_close(server_fd);
	ssl_free(&ssl);

	if(is_task) {
#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)
		printf("\n\rMin available stack size of %s = %d * %d bytes\n\r", __FUNCTION__, uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
#endif

		if(min_heap_size > 0)
			printf("\n\rMin available heap size = %d bytes during %s\n\r", min_heap_size, __FUNCTION__);

		vTaskDelete(NULL);
	}

	if(param != NULL)
		*((int *) param) = ret;
}

void start_ssl_client(void)
{
	is_task = 1;
	//strcpy(server_host, SERVER_HOST);

	if(xTaskCreate(ssl_client, "ssl_client", STACKSIZE, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate failed", __FUNCTION__);
}

void do_ssl_connect(void)
{
	int ret;
	static int success = 0;
	static int fail = 0;

	is_task = 0;
	strcpy(server_host, SERVER_HOST);
	ssl_client(&ret);

	if(ret != 0)
		printf("\n\r%s fail (success %d times, fail %d times)\n\r", __FUNCTION__, success, ++ fail);
	else
		printf("\n\r%s success (success %d times, fail %d times)\n\r", __FUNCTION__, ++ success, fail);
}

void cmd_ssl_client(int argc, char **argv)
{
	if(argc == 2) {
		strcpy(server_host, argv[1]);
#if SSL_USE_SRP
		strcpy(srp_username, "");
		strcpy(srp_password, "");
#endif
	}
#if SSL_USE_SRP
	else if(argc == 4) {
		strcpy(server_host, argv[1]);
		strcpy(srp_username, argv[2]);
		strcpy(srp_password, argv[3]);
	}
#endif
	else {
#if SSL_USE_SRP
		printf("\n\rUsage: %s SSL_SERVER_HOST [SRP_USER_NAME SRP_PASSWORD]", argv[0]);
#else
		printf("\n\rUsage: %s SSL_SERVER_HOST", argv[0]);
#endif
		return;
	}

	start_ssl_client();
}
