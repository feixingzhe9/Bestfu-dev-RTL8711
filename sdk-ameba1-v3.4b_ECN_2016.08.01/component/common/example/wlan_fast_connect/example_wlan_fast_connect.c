/******************************************************************************
 *
 * Copyright(c) 2007 - 2015 Realtek Corporation. All rights reserved.
 *
 *
 ******************************************************************************/

 /** @file

	This example demonstrate how to implement wifi fast reconnection
**/
#include <platform_opts.h>
#include <wlan_fast_connect/example_wlan_fast_connect.h>


#include "task.h"
#include <platform/platform_stdlib.h>
#include <wifi/wifi_conf.h>
#include "flash_api.h"

write_reconnect_ptr p_write_reconnect_ptr;

extern void fATW0(void *arg);
extern void fATW1(void *arg);
extern void fATW2(void *arg);
extern void fATWC(void *arg);

/*
* Usage:
*       wifi connection indication trigger this function to save current
*       wifi profile in flash
*
* Condition: 
*       CONFIG_EXAMPLE_WLAN_FAST_CONNECT flag is set
*/

int wlan_wrtie_reconnect_data_to_flash(u8 *data, uint32_t len)
{
	flash_t flash;
	struct wlan_fast_reconnect read_data = {0};
	if(!data)
            return -1;

	flash_stream_read(&flash, FAST_RECONNECT_DATA, sizeof(struct wlan_fast_reconnect), (u8 *) &read_data);

	//wirte it to flash if different content: SSID, Passphrase, Channel, Security type
	if(memcmp(data, (u8 *) &read_data, sizeof(struct wlan_fast_reconnect)) != 0) {
	    printf("\r\n %s():not the same ssid/passphrase/channel, write new profile to flash", __func__);
	    flash_erase_sector(&flash, FAST_RECONNECT_DATA);
	    flash_stream_write(&flash, FAST_RECONNECT_DATA, len, (uint8_t *) data);
	}

	return 0;
}

/*
* Usage:
*       After wifi init done, waln driver call this function to check whether
*       auto-connect is required.
*
*       This function read previous saved wlan profile in flash and execute connection.
*
* Condition: 
*       CONFIG_EXAMPLE_WLAN_FAST_CONNECT flag is set
*/
int wlan_init_done_callback()
{
	flash_t		flash;
	struct wlan_fast_reconnect *data;
	uint32_t	channel;
	uint32_t    security_type;
	uint8_t     pscan_config;
	char key_id[2] = {0};

#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_set_autoreconnect(1);
#endif
	data = (struct wlan_fast_reconnect *)rtw_zmalloc(sizeof(struct wlan_fast_reconnect));
	if(data){
	    flash_stream_read(&flash, FAST_RECONNECT_DATA, sizeof(struct wlan_fast_reconnect), (uint8_t *)data);
	    if(*((uint32_t *) data) != ~0x0){
		    memcpy(psk_essid, data->psk_essid, sizeof(data->psk_essid));
		    memcpy(psk_passphrase, data->psk_passphrase, sizeof(data->psk_passphrase));
		    memcpy(wpa_global_PSK, data->wpa_global_PSK, sizeof(data->wpa_global_PSK));
		    channel = data->channel;
		    sprintf(key_id,"%d",(char) (channel>>28));
		    channel &= 0xff;
		    security_type = data->security_type;
		    pscan_config = PSCAN_ENABLE | PSCAN_FAST_SURVEY;
		    //set partial scan for entering to listen beacon quickly
		    wifi_set_pscan_chan((uint8_t *)&channel, &pscan_config, 1);
		    
		    //set wifi connect
		    switch(security_type){
	            case RTW_SECURITY_OPEN:
	                fATW0((char*)psk_essid);
	                break;
	            case RTW_SECURITY_WEP_PSK:
	                fATW0((char*)psk_essid);
	                fATW1((char*)psk_passphrase);
	                fATW2(key_id);
	                break;
	            case RTW_SECURITY_WPA_TKIP_PSK:
	            case RTW_SECURITY_WPA2_AES_PSK:
	                fATW0((char*)psk_essid);
	                fATW1((char*)psk_passphrase);
	                break;
	            default:
	                break;
		    }
		    fATWC(NULL);
	    }
	    rtw_mfree(data);
	}

	return 0;
}


void example_wlan_fast_connect()
{
	// Call back from wlan driver after wlan init done
	p_wlan_init_done_callback = wlan_init_done_callback;

	// Call back from application layer after wifi_connection success
	p_write_reconnect_ptr = wlan_wrtie_reconnect_data_to_flash;

}
