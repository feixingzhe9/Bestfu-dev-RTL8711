/******************************************************************************
 *
 * Copyright(c) 2007 - 2015 Realtek Corporation. All rights reserved.
 *
 *
 ******************************************************************************/
#include <platform_opts.h>
   
#if CONFIG_EXAMPLE_MDNS
#include <mdns/example_mdns.h>
#endif

#if CONFIG_EXAMPLE_MCAST
#include <mcast/example_mcast.h>
#endif

#if CONFIG_EXAMPLE_XML
#include <xml/example_xml.h>
#endif

#if CONFIG_EXAMPLE_SOCKET_SELECT
#include <socket_select/example_socket_select.h>
#endif

#if CONFIG_EXAMPLE_SSL_DOWNLOAD
#include <ssl_download/example_ssl_download.h>
#endif

#if CONFIG_EXAMPLE_GOOGLE_NEST
#include <googlenest/example_google.h>  
#define FromDevice            1
#define ToDevice     		2 
#define TYPE         "ToDevice"
#endif   

#if CONFIG_EXAMPLE_UVC
#include <uvc/example_uvc.h>
#endif

#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#include <wlan_fast_connect/example_wlan_fast_connect.h>
#endif

#if CONFIG_EXAMPLE_WIGADGET
#include <wigadget/wigadget.h>
#endif
/*
	Preprocessor of example
*/
void pre_example_entry(void)
{
#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
	example_wlan_fast_connect();
#endif
#if CONFIG_JD_SMART
	example_jdsmart_init();
#endif
#if CONFIG_EXAMPLE_UART_ADAPTER
	example_uart_adapter_init();
#endif
}

/*
  	All of the examples are disabled by default for code size consideration
   	The configuration is enabled in platform_opts.h
*/
void example_entry(void)
{
#if (CONFIG_EXAMPLE_MDNS && !CONFIG_EXAMPLE_UART_ADAPTER) 
	example_mdns();
#endif

#if CONFIG_EXAMPLE_MCAST
	example_mcast();
#endif

#if CONFIG_EXAMPLE_XML
	example_xml();
#endif

#if CONFIG_EXAMPLE_SOCKET_SELECT
	example_socket_select();
#endif

#if CONFIG_EXAMPLE_SSL_DOWNLOAD
	example_ssl_download();
#endif

#if CONFIG_EXAMPLE_GOOGLE_NEST
	example_google(TYPE);
#endif
        
#if CONFIG_EXAMPLE_UVC
	example_uvc();
#endif

#if CONFIG_UART_UPDATE
	example_uart_update();
#endif  

#if CONFIG_EXAMPLE_WIFI_MANAGER
    example_wifi_manager();
#endif

#if CONFIG_EXAMPLE_WIGADGET	
        if(configTOTAL_HEAP_SIZE < ( ( size_t ) ( 115 * 1024 ) ))
          printf("\r\n\r\n\r\n\r\nERROR!!!!\r\n\r\n Please goto FreeRTOSConfig.h to change configTOTAL_HEAP_SIZE more than 115*1024!!!\r\n\r\n\r\n\r\n\r\n\r\n");
        else
          example_wigadget();
          
#endif
}
