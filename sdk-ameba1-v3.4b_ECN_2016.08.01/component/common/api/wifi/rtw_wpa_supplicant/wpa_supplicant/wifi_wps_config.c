#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "queue.h"
#include "utils/os.h"
#include <lwip_netconf.h>
#include <lwip/netif.h>
#include "wifi/wifi_conf.h"
#include "wps/wps_defs.h"
#include <platform/platform_stdlib.h>

/**
 * struct wps_credential - WPS Credential
 * @ssid: SSID
 * @ssid_len: Length of SSID
 * @auth_type: Authentication Type (WPS_AUTH_OPEN, .. flags)
 * @encr_type: Encryption Type (WPS_ENCR_NONE, .. flags)
 * @key_idx: Key index
 * @key: Key
 * @key_len: Key length in octets
 * @mac_addr: MAC address of the Credential receiver
 * @cred_attr: Unparsed Credential attribute data (used only in cred_cb());
 *	this may be %NULL, if not used
 * @cred_attr_len: Length of cred_attr in octets
 * @ap_channel: AP channel
 */
struct dev_credential {
	u8 ssid[32];
	size_t ssid_len;
	u16 auth_type;
	u16 encr_type;
	u8 key_idx;
	u8 key[65];
	size_t key_len;
	u8 mac_addr[6];
	const u8 *cred_attr;
	size_t cred_attr_len;
	u16 ap_channel;
};

typedef struct {
	char *target_ssid;
	u16 config_method;
	_sema scan_sema;
	int isoverlap;
} internal_wps_scan_handler_arg_t;

#define WLAN0_NAME "wlan0"
#ifndef ENABLE
#define ENABLE	(1)
#endif
#ifndef DISABLE
#define	DISABLE (0)
#endif
#define STACKSIZE     512


//static xSemaphoreHandle wps_reconnect_semaphore;
//static struct _WIFI_NETWORK wifi_get_from_certificate = {0};

#define	WPS_AUTH_TYPE_OPEN		(0x0001)
#define	WPS_AUTH_TYPE_WPA_PERSONAL	(0x0002)
#define	WPS_AUTH_TYPE_WPA_ENTERPRISE	(0x0008)
#define	WPS_AUTH_TYPE_WPA2_PERSONAL	(0x0010)
#define	WPS_AUTH_TYPE_WPA2_ENTERPRISE	(0x0020)

#define 	SCAN_BUFFER_LENGTH	(4096)


#ifdef CONFIG_WPS
#if CONFIG_ENABLE_WPS
xqueue_handle_t queue_for_credential;
char wps_pin_code[32];
u16 config_method;
u8 wps_password_id;

void wps_check_and_show_connection_info(void)
{
	rtw_wifi_setting_t setting;	
#if CONFIG_LWIP_LAYER 
	/* Start DHCP Client */
	LwIP_DHCP(0, DHCP_START);		
#endif	
	wifi_get_setting(WLAN0_NAME, &setting);
	wifi_show_setting(WLAN0_NAME, &setting);
}

static void wps_config_wifi_setting(rtw_network_info_t *wifi, struct dev_credential *dev_cred)
{
	printf("\r\nwps_config_wifi_setting\n");
	//memcpy((void *)wifi->ssid, (void *)dev_cred->ssid, dev_cred->ssid_len); 
	strcpy((char*)wifi->ssid.val, (char*)&dev_cred->ssid[0]);
	printf("\r\nwps_wifi.ssid = %s\n", wifi->ssid.val);
	wifi->ssid.len = dev_cred->ssid_len;
	printf("\r\nwps_wifi.ssid_len = %d\n", wifi->ssid.len);

	switch(dev_cred->auth_type) {
	case WPS_AUTH_TYPE_OPEN :
		printf("\r\nsecurity_type = RTW_SECURITY_OPEN\n");
		wifi->security_type = RTW_SECURITY_OPEN;
		break;
	case WPS_AUTH_TYPE_WPA_PERSONAL : 
	case WPS_AUTH_TYPE_WPA_ENTERPRISE : 
		printf("\r\nsecurity_type = RTW_SECURITY_WPA_AES_PSK\n");
		wifi->security_type = RTW_SECURITY_WPA_AES_PSK;
		break;
	case WPS_AUTH_TYPE_WPA2_PERSONAL : 
	case WPS_AUTH_TYPE_WPA2_ENTERPRISE : 
		printf("\r\nsecurity_type = RTW_SECURITY_WPA2_AES_PSK\n");
		wifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
		break;
	}

	printf("\r\nwps_wifi.security_type = %d\n", wifi->security_type);

	//memcpy(wifi->password, dev_cred->key, dev_cred->key_len);
	wifi->password = dev_cred->key;
	printf("\r\nwps_wifi.password = %s\n", wifi->password);
	wifi->password_len = dev_cred->key_len;
	printf("\r\nwps_wifi.password_len = %d", wifi->password_len);
	//xSemaphoreGive(wps_reconnect_semaphore);
	//printf("\r\nrelease wps_reconnect_semaphore");			
}

static void wps_connect_to_AP_by_certificate(rtw_network_info_t *wifi)
{
#define RETRY_COUNT		3
	int retry_count = RETRY_COUNT, ret;

	printf("\r\n=============== wifi_certificate_info ===============\n");
	printf("\r\nwps_wifi.ssid = %s\n", wifi->ssid.val);
	printf("\r\nsecurity_type = %d\n", wifi->security_type);
	printf("\r\nwps_wifi.password = %s\n", wifi->password);
	printf("\r\nssid_len = %d\n", wifi->ssid.len);
	printf("\r\npassword_len = %d\n", wifi->password_len);
	while (1) {
		ret = wifi_connect((char*)wifi->ssid.val,
						 wifi->security_type,
						 (char*)wifi->password,
						 wifi->ssid.len,
						 wifi->password_len,
						 wifi->key_id,
						 NULL);
		if (ret == RTW_SUCCESS) {
			if(retry_count == RETRY_COUNT)
				rtw_msleep_os(1000);  //When start wps with OPEN AP, AP will send a disassociate frame after STA connected, need reconnect here.
			if(RTW_SUCCESS == wifi_is_connected_to_ap( )){
				//printf("\r\n[WPS]Ready to tranceive!!\n");
				wps_check_and_show_connection_info();
				break;
			}
		}
		if (retry_count == 0) {
			printf("\r\n[WPS]Join bss failed\n");
			break;
		}
		retry_count --;
	}
}

static int wps_connect_to_AP_by_open_system(char *target_ssid)
{
	int retry_count = 3, ret;
	
	if (target_ssid != NULL) {
		rtw_msleep_os(500);	//wait scan complete.
		while (1) {
			ret = wifi_connect(target_ssid,
							 RTW_SECURITY_OPEN,
							 NULL,
							 strlen(target_ssid),
							 0,
							 0,
							 NULL);
			if (ret == RTW_SUCCESS) {
			  	//wps_check_and_show_connection_info();
				break;
			}
			if (retry_count == 0) {
				printf("\r\n[WPS]Join bss failed\n");
				return -1;
			}
			retry_count --;
		}
		//
	} else {
		printf("\r\n[WPS]Target SSID is NULL\n");
	}

	return 0;
}

static void process_wps_scan_result( rtw_scan_result_t* record, void * user_data )
{
	internal_wps_scan_handler_arg_t *wps_arg = (internal_wps_scan_handler_arg_t *)user_data;

	if (record->wps_type != 0xff) {
		if (wps_arg->config_method == WPS_CONFIG_PUSHBUTTON) {
			if (record->wps_type == 0x04) {
				wps_password_id = record->wps_type;
				if (++wps_arg->isoverlap == 0) {
					memcpy(&wps_arg->target_ssid[0], record->SSID.val, record->SSID.len);
					wps_arg->target_ssid[record->SSID.len] = '\0';
					printf("\r\n[pbc]Record first triger wps AP = %s\n", wps_arg->target_ssid);
				}
			}
		} else if (wps_arg->config_method == WPS_CONFIG_DISPLAY) {
			if (record->wps_type == 0x00) {
				wps_arg->isoverlap = 0;
				wps_password_id = record->wps_type;
				memcpy(&wps_arg->target_ssid[0], record->SSID.val, record->SSID.len);
				wps_arg->target_ssid[record->SSID.len] = '\0';
				printf("\r\n[pin]find out first triger wps AP = %s\n", wps_arg->target_ssid);
			}
		}
	}
}

static rtw_result_t wps_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	internal_wps_scan_handler_arg_t *wps_arg = (internal_wps_scan_handler_arg_t *)malloced_scan_result->user_data;
	if (malloced_scan_result->scan_complete != RTW_TRUE)
	{
		rtw_scan_result_t* record = &malloced_scan_result->ap_details;
		record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

		process_wps_scan_result(record, malloced_scan_result->user_data);
	}
	else
	{
		printf("\r\nWPS scan done!\r\n");
		rtw_up_sema(&wps_arg->scan_sema);
	}
	return RTW_SUCCESS;
}


static int wps_find_out_triger_wps_AP(char *target_ssid, u16 config_method)
{
	internal_wps_scan_handler_arg_t wps_arg = {0};

	wps_password_id = 0xFF;

	wps_arg.isoverlap = -1;
	wps_arg.config_method = config_method;
	wps_arg.target_ssid = target_ssid;
	rtw_init_sema(&wps_arg.scan_sema, 0);
	if(wps_arg.scan_sema == NULL) return RTW_ERROR;

	if(wifi_scan_networks(wps_scan_result_handler, &wps_arg ) != RTW_SUCCESS){
		printf("\n\rERROR: wifi scan failed");
		goto exit;
	}
	if(rtw_down_timeout_sema(&wps_arg.scan_sema, SCAN_LONGEST_WAIT_TIME) == RTW_FALSE){
		printf("\r\nWPS scan done early!\r\n");
	}

exit:
	rtw_free_sema(&wps_arg.scan_sema);
	
	return wps_arg.isoverlap;
}
extern void wpas_wps_notify_wps_finish_hdl(char *buf, int buf_len, int flags, void *userdata);
extern void wpas_wsc_eapol_recvd_hdl(char *buf, int buf_len, int flags, void* handler_user_data);
int wps_start(u16 wps_config, char *pin, u8 channel, char *ssid)
{
	struct dev_credential dev_cred;
	rtw_network_info_t wifi = {0}; 
	char target_ssid[64];
	int is_overlap = -1;
	u32 start_time = rtw_get_current_time();
	int ret = 0;

	memset(&dev_cred, 0, sizeof(struct dev_credential));
	memset(target_ssid, 0, 64);
	if((wps_config != WPS_CONFIG_PUSHBUTTON) 
		&& (wps_config != WPS_CONFIG_DISPLAY)
		&& (wps_config != WPS_CONFIG_KEYPAD)){
		printf("\n\rWPS: Wps method(%d) is wrong. Not triger WPS.\n", wps_config);
		return -1;
	}
	config_method = wps_config;
	
	if(wps_config == WPS_CONFIG_DISPLAY
		|| wps_config == WPS_CONFIG_KEYPAD) {
		if(pin)
			strcpy(wps_pin_code, pin);
		else{
			printf("\n\rWPS: PIN is NULL. Not triger WPS.\n");
			return -1;
		}
	}
	
	if(!ssid)	{	
		while (1) {
			unsigned int current_time = rtw_get_current_time();
			if (rtw_systime_to_sec(current_time - start_time) < 120) { 	
				is_overlap = wps_find_out_triger_wps_AP(&target_ssid[0], wps_config);
				if ((is_overlap == 0) || (is_overlap > 0)) 
					break;
			} else {
				printf("\r\nWPS: WPS Walking Time Out\n");
				return 0;
			}
		}

		if (is_overlap > 0) {
			printf("\r\nWPS: WPS session overlap. Not triger WPS.\n");
			return 0;
		}
	}else{
		rtw_memcpy(target_ssid, ssid, strlen(ssid));
	}
	
	if (queue_for_credential != NULL) {
		os_xqueue_delete(queue_for_credential);
		queue_for_credential = NULL;
	}
	queue_for_credential = os_xqueue_create(1, sizeof(struct dev_credential));
	if(!queue_for_credential)
		return -1;
	
	wifi_reg_event_handler(WIFI_EVENT_WPS_FINISH, wpas_wps_notify_wps_finish_hdl, NULL);
	wifi_reg_event_handler(WIFI_EVENT_EAPOL_RECVD, wpas_wsc_eapol_recvd_hdl, NULL);

	wifi_set_wps_phase(ENABLE);
	ret = wps_connect_to_AP_by_open_system(target_ssid);
	if(ret < 0){
		printf("\n\rWPS: WPS Fail!!\n");
		goto exit;
	}
	os_xqueue_receive(queue_for_credential, &dev_cred, 120);
	if (dev_cred.ssid[0] != 0 && dev_cred.ssid_len <= 32) {
		wps_config_wifi_setting(&wifi, &dev_cred);
		wifi_set_wps_phase(DISABLE);
		wps_connect_to_AP_by_certificate(&wifi);
		goto exit1;
	} else {
		printf("\n\rWPS: WPS FAIL!!!\n");
		printf("\n\rWPS: WPS FAIL!!!\n");
		printf("\n\rWPS: WPS FAIL!!!\n");
	}
exit:
	wifi_set_wps_phase(DISABLE);
exit1:
	if (queue_for_credential != NULL) {
		os_xqueue_delete(queue_for_credential);
		queue_for_credential = NULL;
	}

	wifi_unreg_event_handler(WIFI_EVENT_WPS_FINISH, wpas_wps_notify_wps_finish_hdl);
	wifi_unreg_event_handler(WIFI_EVENT_EAPOL_RECVD, wpas_wsc_eapol_recvd_hdl);
	
	return 0;
}

#ifdef CONFIG_WPS_AP
int ap_wps_start(u16 wps_config, char *pin)
{
	u8 authorized_mac[ETH_ALEN];
	int ret = 0;
	u32 pin_val = 0;

	printf("\n\rWPS-AP: wps_config(%x).\n", wps_config);
	if((wps_config != WPS_CONFIG_PUSHBUTTON) 
		&& (wps_config != WPS_CONFIG_DISPLAY)
		&& (wps_config != WPS_CONFIG_KEYPAD)){
		printf("\n\rWPS-AP: Wps method(%d) is wrong. Not triger WPS.\n", wps_config);
		return -1;
	}
	config_method = wps_config;
	if(wps_config == WPS_CONFIG_DISPLAY
		|| wps_config == WPS_CONFIG_KEYPAD) {
		if(pin)
			strcpy(wps_pin_code, pin);
		else{
			printf("\n\rWPS-AP: PIN is NULL. Not triger WPS.\n");
			return -1;
		}
	}

	if (queue_for_credential != NULL) {
		os_xqueue_delete(queue_for_credential);
		queue_for_credential = NULL;
	}

	queue_for_credential = os_xqueue_create(1, sizeof(authorized_mac));	
	if(!queue_for_credential)
		return -1;
	
	wifi_set_wps_phase(1);
	
	if(wps_config == WPS_CONFIG_KEYPAD)
	{
		pin_val = atoi(pin);
		if (!wps_pin_valid(pin_val)) {
			printf("\n\rWPS-AP: Enter pin code is unvalid.");
			goto exit;
		}
		ret = wpas_wps_registrar_add_pin((unsigned char const*)pin, strlen(pin));
	}
	else if(wps_config == WPS_CONFIG_DISPLAY)
		ret = wpas_wps_registrar_add_pin((unsigned char const*)pin, strlen(pin));
	else
		ret = wpas_wps_registrar_button_pushed();

	if(ret<0)
		goto exit;

	printf("\n\rWPS-AP: wait for STA connect!\n");
	os_xqueue_receive(queue_for_credential, authorized_mac, 120); //max wait 2min

	if(!wpas_wps_registrar_check_done())
	{
		ret = -1;
		wpas_wps_registrar_wps_cancel();
	}

exit:
	wifi_set_wps_phase(0);
	os_xqueue_delete(queue_for_credential);
	queue_for_credential = NULL;		

	return ret;
}
#endif //CONFIG_WPS_AP

void wps_judge_staion_disconnect(void) 
{
	int mode = 0;
	unsigned char ssid[33];

	wext_get_mode(WLAN0_NAME, &mode);

	switch(mode) {
	case IW_MODE_MASTER:		//In AP mode
		rltk_wlan_deinit();
		rltk_wlan_init(0,RTW_MODE_STA);
		rltk_wlan_start(0);
		break;
	case IW_MODE_INFRA:		//In STA mode
		if(wext_get_ssid(WLAN0_NAME, ssid) > 0)
			wifi_disconnect();
	}	
}

void cmd_wps(int argc, char **argv)
{
	wps_judge_staion_disconnect();
	
	if((argc == 2 || argc == 3 ) && (argv[1] != NULL)){
		if(strcmp(argv[1],"pin") == 0){
			unsigned int pin_val = 0;
			/* start pin */
			if(argc == 2){
				char device_pin[10];
				pin_val = wps_generate_pin();
				sprintf(device_pin, "%08d", pin_val);
				/* Display PIN 3 times to prevent to be overwritten by logs from other tasks */
				printf("\n\rWPS: Start WPS PIN Display. PIN: [%s]\n\r", device_pin);
				printf("\n\rWPS: Start WPS PIN Display. PIN: [%s]\n\r", device_pin);
				printf("\n\rWPS: Start WPS PIN Display. PIN: [%s]\n\r", device_pin);
				wps_start(WPS_CONFIG_DISPLAY, (char*)device_pin, 0, NULL);
			}else{
	 			pin_val = atoi(argv[2]);
				if (!wps_pin_valid(pin_val)) {
					printf("\n\rWPS: Device pin code is invalid. Not triger WPS.\n");
					return;
				}
				printf("\n\rWPS: Start WPS PIN Keypad.\n\r");
				wps_start(WPS_CONFIG_KEYPAD, argv[2], 0, NULL);
			}
		}else if(strcmp(argv[1],"pbc") == 0){
			/* start pbc */
			printf("\n\rWPS: Start WPS PBC.\n\r");
			wps_start(WPS_CONFIG_PUSHBUTTON, NULL, 0, NULL);
		}else{
			printf("\n\rWPS: Wps Method is wrong. Not triger WPS.\n");
			return;
		}
	}
}

#ifdef CONFIG_WPS_AP
/*
cmd_ap_wps for AP WSC setting. command style:
cmd_ap_wps pbc or cmd_ap_wps pin 12345678
*/
void cmd_ap_wps(int argc, char **argv)
{
	if(rltk_wlan_running(WLAN1_IDX)){
		printf("\n\rNot support con-current softAP WSC!\n\r");
		return;
	}

	if((argc == 2 || argc == 3) && (argv[1] != NULL)) {
		if (strcmp(argv[1],"pin") == 0 ) {
			unsigned int pin_val = 0;
			if(argc == 3){
				pin_val = atoi(argv[2]);
				if (!wps_pin_valid(pin_val)) {
					printf("\n\rWPS-AP: Device pin code is invalid. Not trigger WPS.\n\r");
					return;
				}
				printf("\n\rWPS-AP: Start AP WPS PIN Keypad.\n");
				ap_wps_start(WPS_CONFIG_KEYPAD, argv[2]);
			}else{
				char device_pin[10];
				pin_val = wps_generate_pin();
				sprintf(device_pin, "%08d", pin_val);
				printf("\n\rWPS: Start WPS PIN Display. PIN: %s\n\r", device_pin);
				ap_wps_start(WPS_CONFIG_DISPLAY, (char*)device_pin);
			}
		}else if (strcmp(argv[1],"pbc") == 0) {
			printf("\n\rWPS-AP: Start AP WPS PBC\n");
			ap_wps_start(WPS_CONFIG_PUSHBUTTON, NULL);
		}else{
			printf("\n\rWPS-AP Usage:\"wifi_ap_wps pin [pin_code]\" or \"wifi_ap_wps pbc\"\n");
			return;
		}
	} else {
		printf("\n\rWPS-AP Usage:\"wifi_ap_wps pin [pin_code]\" or \"wifi_ap_wps pbc\"\n");
	}
	return;
}
#endif //CONFIG_WPS_AP
#endif //CONFIG_ENABLE_WPS
#endif	//#ifdef CONFIG_WPS
