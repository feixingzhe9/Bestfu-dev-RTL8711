#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log_service.h"
#include "atcmd_wifi.h"
#include <lwip_netconf.h>
#include "tcpip.h"
#include <dhcp/dhcps.h>
#include <wlan/wlan_test_inc.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>
#include "sockets.h"
#if SUPPORT_LOG_SERVICE
/******************************************************************************/
#define	_AT_WLAN_SET_SSID_          "ATW0"
#define	_AT_WLAN_SET_PASSPHRASE_    "ATW1"
#define	_AT_WLAN_SET_KEY_ID_        "ATW2"
#define	_AT_WLAN_AP_SET_SSID_       "ATW3"
#define	_AT_WLAN_AP_SET_SEC_KEY_    "ATW4"
#define	_AT_WLAN_AP_SET_CHANNEL_    "ATW5"
#define _AT_WLAN_SET_BSSID_         "ATW6"
#define	_AT_WLAN_AP_ACTIVATE_       "ATWA"
#define _AT_WLAN_AP_STA_ACTIVATE_   "ATWB"
#define	_AT_WLAN_JOIN_NET_          "ATWC"
#define	_AT_WLAN_DISC_NET_          "ATWD"
#define	_AT_WLAN_WEB_SERVER_        "ATWE"
#define _AT_WLAN_P2P_FIND_          "ATWF"
#define _AT_WLAN_P2P_START_         "ATWG"
#define _AT_WLAN_P2P_STOP_          "ATWH"
#define _AT_WLAN_PING_TEST_         "ATWI"
#define _AT_WLAN_P2P_CONNECT_       "ATWJ"
#define _AT_WLAN_P2P_DISCONNECT_    "ATWK"
#define _AT_WLAN_SSL_CLIENT_        "ATWL"
#define _AT_WLAN_PROMISC_           "ATWM"
#define _AT_WLAN_P2P_INFO_          "ATWN"
#define _AT_WLAN_OTA_UPDATE_        "ATWO"
#define	_AT_WLAN_POWER_             "ATWP"
#define	_AT_WLAN_SIMPLE_CONFIG_     "ATWQ"
#define	_AT_WLAN_GET_RSSI_          "ATWR"
#define	_AT_WLAN_SCAN_              "ATWS"
#define _AT_WLAN_SCAN_WITH_SSID_    "ATWs"
#define _AT_WLAN_TCP_TEST_          "ATWT"
#define _AT_WLAN_UDP_TEST_          "ATWU"
#define _AT_WLAN_WPS_               "ATWW"
#define _AT_WLAN_AP_WPS_            "ATWw"
#define _AT_WLAN_AIRKISS_           "ATWX"
#define _AT_WLAN_IWPRIV_            "ATWZ"
#define	_AT_WLAN_INFO_              "ATW?"

#define	_AT_WLAN_EXTEND_POWER_MODE_        "ATXP"

#ifndef CONFIG_SSL_CLIENT
#define CONFIG_SSL_CLIENT       0
#endif
#ifndef CONFIG_WEBSERVER
#define CONFIG_WEBSERVER        0
#endif
#ifndef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE       0
#endif
#ifndef CONFIG_BSD_TCP
#define CONFIG_BSD_TCP	        1
#endif
#ifndef CONFIG_ENABLE_P2P
#define CONFIG_ENABLE_P2P		0
#endif
#define SCAN_WITH_SSID		0
#if CONFIG_WEBSERVER
#define CONFIG_READ_FLASH	1
extern rtw_wifi_setting_t wifi_setting;
#endif
#if CONFIG_WLAN

extern void cmd_promisc(int argc, char **argv);
extern void cmd_update(int argc, char **argv);
extern void cmd_tcp(int argc, char **argv);
extern void cmd_udp(int argc, char **argv);
extern void cmd_simple_config(int argc, char **argv);
#if CONFIG_ENABLE_WPS
extern void cmd_wps(int argc, char **argv);
#endif
extern void cmd_ssl_client(int argc, char **argv);
#ifdef CONFIG_WPS_AP
extern void cmd_ap_wps(int argc, char **argv);
extern int wpas_wps_dev_config(u8 *dev_addr, u8 bregistrar);
#endif //CONFIG_WPS_AP
#if CONFIG_ENABLE_P2P
extern void cmd_wifi_p2p_start(int argc, char **argv);
extern void cmd_wifi_p2p_stop(int argc, char **argv);
extern void cmd_p2p_listen(int argc, char **argv);
extern void cmd_p2p_find(int argc, char **argv);
extern void cmd_p2p_peers(int argc, char **argv);
extern void cmd_p2p_info(int argc, char **argv);
extern void cmd_p2p_disconnect(int argc, char **argv);
extern void cmd_p2p_connect(int argc, char **argv);
#endif //CONFIG_ENABLE_P2P
#if CONFIG_AIRKISS
extern int airkiss_start();
#endif
#if CONFIG_LWIP_LAYER
extern struct netif xnetif[NET_IF_NUM]; 
#endif
#if CONFIG_INIC_CMD_RSP
extern void inic_c2h_wifi_info(const char *atcmd, char status);
extern void inic_c2h_msg(const char *atcmd, u8 status, char *msg, u16 msg_len);
#endif
static rtw_network_info_t wifi = {0};
static rtw_ap_info_t ap = {0};
static unsigned char password[65] = {0};

extern unsigned char gWifiConnectStatus;
static void init_wifi_struct(void)
{
	memset(wifi.ssid.val, 0, sizeof(wifi.ssid.val));
	memset(wifi.bssid.octet, 0, ETH_ALEN);	
	memset(password, 0, sizeof(password));
	wifi.ssid.len = 0;
	wifi.password = NULL;
	wifi.password_len = 0;
        wifi.key_id = -1;
	memset(ap.ssid.val, 0, sizeof(ap.ssid.val));
	ap.ssid.len = 0;
	ap.password = NULL;
	ap.password_len = 0;
	ap.channel = 1;
}

static void print_scan_result( rtw_scan_result_t* record )
{
    RTW_API_INFO( ( "%s\t ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra" ) );
    RTW_API_INFO( ( MAC_FMT, MAC_ARG(record->BSSID.octet) ) );
    RTW_API_INFO( ( " %d\t ", record->signal_strength ) );
    RTW_API_INFO( ( " %d\t  ", record->channel ) );
    RTW_API_INFO( ( " %d\t  ", record->wps_type ) );
    RTW_API_INFO( ( "%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
                                 ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
                                 ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
                                 ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
                                 ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
                                 ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
                                 ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" :
                                 "Unknown" ) );

    RTW_API_INFO( ( " %s ", record->SSID.val ) );
    RTW_API_INFO( ( "\r\n" ) );
}

static rtw_result_t app_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	static int ApNum = 0;

	if (malloced_scan_result->scan_complete != RTW_TRUE) {
		rtw_scan_result_t* record = &malloced_scan_result->ap_details;
		record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

		RTW_API_INFO( ( "%d\t ", ++ApNum ) );

		print_scan_result(record);
#if CONFIG_INIC_CMD_RSP
		if(malloced_scan_result->user_data)
			memcpy((void *)((char *)malloced_scan_result->user_data+(ApNum-1)*sizeof(rtw_scan_result_t)), (char *)record, sizeof(rtw_scan_result_t));
#endif
	} else{
#if CONFIG_INIC_CMD_RSP
		inic_c2h_msg("ATWS", RTW_SUCCESS, (char *)malloced_scan_result->user_data, ApNum*sizeof(rtw_scan_result_t));
		if(malloced_scan_result->user_data)
			free(malloced_scan_result->user_data);
		inic_c2h_msg("ATWS", RTW_SUCCESS, NULL, 0);
#endif
		ApNum = 0;
	}
	return RTW_SUCCESS;
}

void fATW0(void *arg){
	int ret = RTW_SUCCESS;
        if(!arg){
          printf("[ATW0]Usage: ATW0=SSID\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
	printf("[ATW0]: _AT_WLAN_SET_SSID_ [%s]\n\r", (char*)arg);
	strcpy((char *)wifi.ssid.val, (char*)arg);
	wifi.ssid.len = strlen((char*)arg);
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW0", ret, NULL, 0);
#endif
	return;
}

void fATW1(void *arg){
	int ret = RTW_SUCCESS;
        printf("[ATW1]: _AT_WLAN_SET_PASSPHRASE_ [%s]\n\r", (char*)arg); 
	strcpy((char *)password, (char*)arg);
	wifi.password = password;
	wifi.password_len = strlen((char*)arg);
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW1", ret, NULL, 0);
#endif
	return;	
}

void fATW2(void *arg){
	int ret = RTW_SUCCESS;
        printf("[ATW2]: _AT_WLAN_SET_KEY_ID_ [%s]\n\r", (char*)arg);
		if((strlen((const char *)arg) != 1 ) || (*(char*)arg <'0' ||*(char*)arg >'3')) {
			printf("\n\rWrong WEP key id. Must be one of 0,1,2, or 3.");
			ret = RTW_BADARG;
			goto exit;
		}
	wifi.key_id = atoi((const char *)(arg));
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW2", ret, NULL, 0);
#endif
	return;
}

void fATW3(void *arg){
	int ret = RTW_SUCCESS;
        if(!arg){
          printf("[ATW3]Usage: ATW3=SSID\n\r");
		ret = RTW_BADARG;
		goto exit;
        }

	ap.ssid.len = strlen((char*)arg);

	if(ap.ssid.len > 32){
          printf("[ATW3]Error: SSID length can't exceed 32\n\r");
		ret = RTW_BADARG;
		goto exit;
    }
	strcpy((char *)ap.ssid.val, (char*)arg);

	printf("[ATW3]: _AT_WLAN_AP_SET_SSID_ [%s]\n\r", ap.ssid.val); 
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW3", ret, NULL, 0);
#endif
	return;
}

void fATW4(void *arg){
	int ret = RTW_SUCCESS;
	strcpy((char *)password, (char*)arg);
	ap.password = password;
	ap.password_len = strlen((char*)arg);
	printf("[ATW4]: _AT_WLAN_AP_SET_SEC_KEY_ [%s]\n\r", ap.password); 
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW4", ret, NULL, 0);
#endif
	return;
}

void fATW5(void *arg){
	int ret = RTW_SUCCESS;
	ap.channel = (unsigned char) atoi((const char *)arg);
	printf("[ATW5]: _AT_WLAN_AP_SET_CHANNEL_ [channel %d]\n\r", ap.channel); 
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW5", ret, NULL, 0);
#endif
	return;
}

void fATW6(void *arg){
	u32		mac[ETH_ALEN];
	u32		i;
	int ret = RTW_SUCCESS;
	if(!arg){
		printf("[ATW6]Usage: ATW6=BSSID\n\r");
		ret =  RTW_BADARG;
		goto exit;
	}
	printf("[ATW6]: _AT_WLAN_SET_BSSID_ [%s]\n\r", (char*)arg);
	sscanf(arg, MAC_FMT, mac, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);
	for(i = 0; i < ETH_ALEN; i ++)
		wifi.bssid.octet[i] = (u8)mac[i] & 0xFF;
exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW6", ret, NULL, 0);
#endif
	return;
}

void fATWA(void *arg){
#if CONFIG_LWIP_LAYER
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif * pnetif = &xnetif[0];
#endif
	int timeout = 20;
	int ret = RTW_SUCCESS;
	printf("[ATWA]: _AT_WLAN_AP_ACTIVATE_\n\r"); 
	if(ap.ssid.val[0] == 0){
          printf("[ATWA]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto exit;
    }
	if(ap.password == NULL){
          ap.security_type = RTW_SECURITY_OPEN;
        }
        else{
          ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }

#if CONFIG_WEBSERVER
    //store into flash
    memset(wifi_setting.ssid, 0, sizeof(wifi_setting.ssid));;
    memcpy(wifi_setting.ssid, ap.ssid.val, strlen((char*)ap.ssid.val));
	wifi_setting.ssid[ap.ssid.len] = '\0';
    wifi_setting.security_type = ap.security_type;
    if(ap.security_type !=0)
        wifi_setting.security_type = 1;
    else
        wifi_setting.security_type = 0;
	if (ap.password)
    	memcpy(wifi_setting.password, ap.password, strlen((char*)ap.password));
	else
		memset(wifi_setting.password, 0, sizeof(wifi_setting.password));
    wifi_setting.channel = ap.channel;
#if CONFIG_READ_FLASH
    StoreApInfo();
#endif
#endif
	
#if CONFIG_LWIP_LAYER
	dhcps_deinit();
	IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
#ifdef CONFIG_DONT_CARE_TP
	pnetif->flags |= NETIF_FLAG_IPSWITCH;
#endif
#endif
	wifi_off();
	vTaskDelay(20);
	if (wifi_on(RTW_MODE_AP) < 0){
		printf("\n\rERROR: Wifi on failed!");
		ret = RTW_ERROR;
		goto exit;
	}
	printf("\n\rStarting AP ...");

#ifdef CONFIG_WPS_AP
	wpas_wps_dev_config(pnetif->hwaddr, 1);
#endif
	if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel) )< 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}

	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
				printf("\n\r%s started\n", ap.ssid.val);
				ret = RTW_SUCCESS;
				break;
			}
		}

		if(timeout == 0) {
			printf("\n\rERROR: Start AP timeout!");
			ret = RTW_TIMEOUT;		
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
	//LwIP_UseStaticIP(pnetif);
	dhcps_init(pnetif);
#endif

exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWA", ret);
#endif
	init_wifi_struct( );
}

#if CONFIG_INIC_EN
typedef struct {
	char *target_ssid;
	rtw_security_t *security_mode;
	unsigned char * channel;
	void * scan_sema;
	int scan_result;
} inic_scan_handler_arg_t;
static void _inic_process_scan_result( rtw_scan_result_t* record, void * user_data  )
{
	inic_scan_handler_arg_t *smartlink_arg = (inic_scan_handler_arg_t *)user_data;

	if (strlen(smartlink_arg->target_ssid) == record->SSID.len
	    && rtw_memcmp(smartlink_arg->target_ssid, record->SSID.val, record->SSID.len)) {
		smartlink_arg->scan_result = 1;

		if(record->security == RTW_SECURITY_WPA2_AES_PSK || record->security == RTW_SECURITY_WPA_AES_PSK 
			|| record->security == RTW_SECURITY_WPA2_MIXED_PSK )
			record->security = RTW_SECURITY_WPA2_AES_PSK;
		else if(record->security == RTW_SECURITY_WPA_TKIP_PSK || record->security == RTW_SECURITY_WPA2_TKIP_PSK)
			record->security = RTW_SECURITY_WPA_TKIP_PSK;
		
		*smartlink_arg->security_mode = record->security;
		*smartlink_arg->channel = record->channel;
		printf("Get AP's security mode = 0x%x\n", *smartlink_arg->security_mode);
	}
}

static rtw_result_t _inic_specificed_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	inic_scan_handler_arg_t *smartlink_arg = (inic_scan_handler_arg_t *)malloced_scan_result->user_data;

    if (malloced_scan_result->scan_complete != RTW_TRUE) {
        rtw_scan_result_t* record = &malloced_scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

        _inic_process_scan_result(record, malloced_scan_result->user_data);
    } else {
		rtw_up_sema(&smartlink_arg->scan_sema);
    }
    return RTW_SUCCESS;
}

static int _inic_get_AP_security_mode(char * ssid, rtw_security_t *security_mode, unsigned char * channel)
{
	inic_scan_handler_arg_t smartlink_arg = {0};

	smartlink_arg.scan_result = 0;
	smartlink_arg.security_mode = security_mode;
	smartlink_arg.target_ssid = ssid;
       smartlink_arg.channel = channel;
	rtw_init_sema(&smartlink_arg.scan_sema, 0);
	if(smartlink_arg.scan_sema == NULL) return 0;

	if(wifi_scan_networks(_inic_specificed_scan_result_handler, &smartlink_arg ) != RTW_SUCCESS){
		printf("Wifi scan failed!\n");
		goto exit;
	}
	rtw_down_timeout_sema(&smartlink_arg.scan_sema, SCAN_LONGEST_WAIT_TIME);
exit:
	rtw_free_sema(&smartlink_arg.scan_sema);

	return smartlink_arg.scan_result;
}
#endif


void fATWC(void *arg){
	int mode, ret;
	unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
	char empty_bssid[6] = {0}, assoc_by_bssid = 0;	
	
        gWifiConnectStatus = 1;         //xjx-test
        
	printf("[ATWC]: _AT_WLAN_JOIN_NET_\n\r"); 
	if(memcmp (wifi.bssid.octet, empty_bssid, 6))
		assoc_by_bssid = 1;
	else if(wifi.ssid.val[0] == 0){
		printf("[ATWC]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
                gWifiConnectStatus = 0;         //xjx-test       未连接到路由器
		goto EXIT;
	}
	if(wifi.password != NULL){
		if((wifi.key_id >= 0)&&(wifi.key_id <= 3)) {
			wifi.security_type = RTW_SECURITY_WEP_PSK;
		}
		else{
			wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
		}
	}
	else{
		wifi.security_type = RTW_SECURITY_OPEN;
	}
	//Check if in AP mode
	wext_get_mode(WLAN0_NAME, &mode);
	if(mode == IW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
		dhcps_deinit();
#endif
		wifi_off();
		vTaskDelay(20);
		if (wifi_on(RTW_MODE_STA) < 0){
			printf("\n\rERROR: Wifi on failed!");
                        gWifiConnectStatus = 0;         //xjx-test       未连接到路由器
                        ret = RTW_ERROR;
			goto EXIT;
		}
	}

#if CONFIG_INIC_EN //get security mode from scan list
	u8 connect_channel;
	u8 pscan_config;
	wifi.security_type = RTW_SECURITY_UNKNOWN;
	if(wifi.security_type == RTW_SECURITY_UNKNOWN)
	{
		int security_retry_count = 0;
		while (1) {
			if (_inic_get_AP_security_mode((char*)wifi.ssid.val, &wifi.security_type, &connect_channel))
				break;
			security_retry_count++;
			if(security_retry_count >= 3){
				printf("Can't get AP security mode and channel.\n");
				ret = RTW_NOTFOUND;
                                gWifiConnectStatus = 0;//xjx-test       未连接到路由器
				goto EXIT;
			}
		}
		if(wifi.security_type == RTW_SECURITY_WEP_PSK || wifi.security_type == RTW_SECURITY_WEP_SHARED)
			wifi.key_id = (wifi.key_id <0 || wifi.key_id >3)?0:wifi.key_id;
		//hex to ascii conversion
		if(wifi.security_type == RTW_SECURITY_WEP_PSK)
		{
			if(wifi.password_len == 10)
			{
				u32 p[5];
				u8 pwd[6], i = 0; 
				sscanf((const char*)wifi.password, "%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4]);
				for(i=0; i< 5; i++)
					pwd[i] = (u8)p[i];
				pwd[5] = '\0';
				memset(wifi.password, 0, 65);
				strcpy((char*)wifi.password, (char*)pwd);
				wifi.password_len = 5;
			}else if(wifi.password_len == 26){
				u32 p[13];
				u8 pwd[14], i = 0;
				sscanf((const char*)wifi.password, "%02x%02x%02x%02x%02x%02x%02x"\
				"%02x%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4],\
				&p[5], &p[6], &p[7], &p[8], &p[9], &p[10], &p[11], &p[12]);
				for(i=0; i< 13; i++)
					pwd[i] = (u8)p[i];
				pwd[13] = '\0';
				memset(wifi.password, 0, 65);
				strcpy((char*)wifi.password, (char*)pwd);
				wifi.password_len = 13;
			}
		}

	}
        pscan_config = PSCAN_ENABLE;
	if(connect_channel > 0 && connect_channel < 14)
		wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
#endif

	if(assoc_by_bssid){
		printf("\n\rJoining BSS by BSSID "MAC_FMT" ...\n\r", MAC_ARG(wifi.bssid.octet));
		ret = wifi_connect_bssid(wifi.bssid.octet, (char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, 
						ETH_ALEN, wifi.ssid.len, wifi.password_len, wifi.key_id, NULL);		
	} else {
		printf("\n\rJoining BSS by SSID %s...\n\r", (char*)wifi.ssid.val);
		ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len,
						wifi.password_len, wifi.key_id, NULL);
	}
	
	if(ret!= RTW_SUCCESS){
		printf("\n\rERROR: Can't connect to AP");
                gWifiConnectStatus = 0;         //xjx-test       未连接到路由器
		goto EXIT;
	}
	tick2 = xTaskGetTickCount();
	printf("\r\nConnected after %dms.\n", (tick2-tick1));
#if CONFIG_LWIP_LAYER
		/* Start DHCPClient */
		LwIP_DHCP(0, DHCP_START);
	tick3 = xTaskGetTickCount();
	printf("\r\n\nGot IP after %dms.\n", (tick3-tick1));
#endif
	printf("\n\r");
EXIT:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWC", ret);
#endif

	init_wifi_struct( );
}

void fATWD(void *arg){
	int timeout = 20;
	char essid[33];
	int ret = RTW_SUCCESS;
	printf("[ATWD]: _AT_WLAN_DISC_NET_\n\r"); 
	printf("\n\rDeassociating AP ...");

	if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
		printf("\n\rWIFI disconnected");
		return;
	}

	if((ret = wifi_disconnect()) < 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}

	while(1) {
		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
			printf("\n\rWIFI disconnected");
			break;
		}

		if(timeout == 0) {
			printf("\n\rERROR: Deassoc timeout!");
			ret = RTW_TIMEOUT;
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
        printf("\n\r");
exit:
#if CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWD", ret, NULL, 0);
#endif
        init_wifi_struct( );
}

void fATWS(void *arg){
        char buf[32] = {0};
        u8 *channel_list = NULL;
        u8 *pscan_config = NULL;
        int num_channel = 0;
        int i, argc = 0;
        char *argv[MAX_ARGC] = {0};        
	int ret = RTW_SUCCESS;
#if CONFIG_INIC_CMD_RSP
	u8 *inic_scan_buf = NULL;
#endif
        printf("[ATWS]: _AT_WLAN_SCAN_\n\r");
        if(arg){
          strcpy(buf, arg);
          argc = parse_param(buf, argv);
          if(argc < 2){
		ret = RTW_BADARG;
            goto exit;
	   }
          num_channel = atoi(argv[1]);
          channel_list = (u8*)malloc(num_channel);
          if(!channel_list){
                  printf("[ATWS]ERROR: Can't malloc memory for channel list\n\r");
                  ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
                  goto exit;
          }
          pscan_config = (u8*)malloc(num_channel);
	  	  if(!pscan_config){
				printf("[ATWS]ERROR: Can't malloc memory for pscan_config\n\r");
				goto exit;
		  }
          //parse command channel list
          for(i = 2; i <= argc -1 ; i++){
                  *(channel_list + i - 2) = (u8)atoi(argv[i]);
                  *(pscan_config + i - 2) = PSCAN_ENABLE;
          }
          
          if((ret = wifi_set_pscan_chan(channel_list, pscan_config, num_channel)) < 0){
              printf("[ATWS]ERROR: wifi set partial scan channel fail\n\r");
              goto exit;
          }
        }
#if CONFIG_INIC_CMD_RSP
	inic_scan_buf = malloc(65*sizeof(rtw_scan_result_t));
	if(inic_scan_buf == NULL){
		ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
		goto exit;
	}
        memset(inic_scan_buf, 0, 65*sizeof(rtw_scan_result_t));
	if((ret = wifi_scan_networks(app_scan_result_handler, inic_scan_buf)) != RTW_SUCCESS){
		printf("[ATWS]ERROR: wifi scan failed\n\r");
		goto exit;
	}	
#else
	if((ret = wifi_scan_networks(app_scan_result_handler, NULL )) != RTW_SUCCESS){
		printf("[ATWS]ERROR: wifi scan failed\n\r");
		goto exit;
	}
#endif
exit:
#if CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWS", ret, NULL, 0);
#endif
	if(arg && channel_list)
		free(channel_list);
}

#if SCAN_WITH_SSID
void fATWs(void *arg){
	char buf[32] = {0};
	u8 *channel_list = NULL;
	u8 *pscan_config = NULL;
	int scan_buf_len = 500;
	int num_channel = 0;
	int i, argc = 0;
	char *argv[MAX_ARGC] = {0};
	printf("[ATWs]: _AT_WLAN_SCAN_WITH_SSID_ [%s]\n\r",  (char*)wifi.ssid.val);
	if(arg){
		strcpy(buf, arg);
		argc = parse_param(buf, argv);
		if(argc == 2){
			scan_buf_len = atoi(argv[1]);  	
			if(scan_buf_len < 36){
				printf("[ATWs] BUFFER_LENGTH too short\n\r");
				goto exit;
			}
		}else if(argc > 2){
			num_channel = atoi(argv[1]);
			channel_list = (u8*)malloc(num_channel);
			if(!channel_list){
				printf("[ATWs]ERROR: Can't malloc memory for channel list\n\r");
				goto exit;
			}
			pscan_config = (u8*)malloc(num_channel);
			if(!pscan_config){
				printf("[ATWs]ERROR: Can't malloc memory for pscan_config\n\r");
				goto exit;
		  	}			
			//parse command channel list
			for(i = 2; i <= argc -1 ; i++){
				*(channel_list + i - 2) = (u8)atoi(argv[i]);
				*(pscan_config + i - 2) = PSCAN_ENABLE;	
			}

			if(wifi_set_pscan_chan(channel_list, pscan_config, num_channel) < 0){
				printf("[ATWs]ERROR: wifi set partial scan channel fail\n\r");
				goto exit;
			}
		}
	}else{
		printf("[ATWs]For Scan all channel Usage: ATWs=BUFFER_LENGTH\n\r");          
		printf("[ATWs]For Scan partial channel Usage: ATWs=num_channels[channel_num1, ...]\n\r");
		goto exit;
	}

	if(wifi_scan_networks_with_ssid(NULL, &scan_buf_len, (char*)wifi.ssid.val, wifi.ssid.len) != RTW_SUCCESS){
		printf("[ATWs]ERROR: wifi scan failed\n\r");
	}
exit:
	init_wifi_struct( );
	if(arg && channel_list)
		free(channel_list);
}
#endif

void fATWR(void *arg){
	int rssi = 0;
	printf("[ATWR]: _AT_WLAN_GET_RSSI_\n\r"); 
	wifi_get_rssi(&rssi);
	printf("\n\rwifi_get_rssi: rssi = %d", rssi);
	printf("\n\r");
}

void fATWP(void *arg){
	unsigned int parm = atoi((const char *)(arg));
	printf("[ATWP]: _AT_WLAN_POWER_[%s]\n\r", parm?"ON":"OFF");
	if(parm == 1){
		if(wifi_on(RTW_MODE_STA)<0){
			printf("\n\rERROR: Wifi on failed!\n");
		}
	}
	else if(parm == 0)
	{
#if CONFIG_WEBSERVER
		stop_web_server();
#endif
		wifi_off();		
	}
	else
		printf("[ATWP]Usage: ATWP=0/1\n\r");
}

#ifdef  CONFIG_CONCURRENT_MODE
void fATWB(void *arg)
{
	int timeout = 20;//, mode;
	int ret = RTW_SUCCESS;
#if CONFIG_LWIP_LAYER
	struct netif * pnetiff = (struct netif *)&xnetif[1];
#endif
	printf("[ATWB](_AT_WLAN_AP_STA_ACTIVATE_)\n\r"); 
	if(ap.ssid.val[0] == 0){
          printf("[ATWB]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
	if(ap.channel > 14){
		printf("[ATWB]Error:bad channel! channel is from 1 to 14\n\r");
		ret = RTW_BADARG;
		goto exit;
	}
	if(ap.password == NULL){
          ap.security_type = RTW_SECURITY_OPEN;
        }
        else{
          ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
        }

#if CONFIG_WEBSERVER
    //store into flash
    memset(wifi_setting.ssid, 0, sizeof(wifi_setting.ssid));;
    memcpy(wifi_setting.ssid, ap.ssid.val, strlen((char*)ap.ssid.val));
	wifi_setting.ssid[ap.ssid.len] = '\0';
    wifi_setting.security_type = ap.security_type;
    if(ap.security_type !=0)
        wifi_setting.security_type = 1;
    else
        wifi_setting.security_type = 0;
	if (ap.password)
    	memcpy(wifi_setting.password, ap.password, strlen((char*)ap.password));
	else
		memset(wifi_setting.password, 0, sizeof(wifi_setting.password));;
    wifi_setting.channel = ap.channel;
#if CONFIG_READ_FLASH
    StoreApInfo();
#endif
#endif

#if CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif
        
	wifi_off();
	vTaskDelay(20);
	if ((ret = wifi_on(RTW_MODE_STA_AP)) < 0){
		printf("\n\rERROR: Wifi on failed!");
		ret = RTW_ERROR;
		goto exit;
	}

	printf("\n\rStarting AP ...");
	if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel)) < 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}
	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN1_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
				printf("\n\r%s started\n", ap.ssid.val);
				ret = RTW_SUCCESS;
				break;
			}
		}

		if(timeout == 0) {
			printf("\n\rERROR: Start AP timeout!");
			ret = RTW_TIMEOUT;
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
	LwIP_UseStaticIP(&xnetif[1]);
#ifdef CONFIG_DONT_CARE_TP
	pnetiff->flags |= NETIF_FLAG_IPSWITCH;
#endif
	dhcps_init(pnetiff);
#endif

exit:
#if CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWB", ret);
#endif
	init_wifi_struct();
}
#endif

void fATWI(void *arg){
        int count, argc = 0;
        char buf[32] = {0};
        char *argv[MAX_ARGC] = {0};
        printf("[ATWI]: _AT_WLAN_PING_TEST_\n\r");
        if(!arg){
          printf("[ATWI]Usage: ATWI=xxxx.xxxx.xxxx.xxxx[y/loop]\n\r");
          return;
        }
        strcpy(buf, arg);
        argv[0] = "ping";
        argc = parse_param(buf, argv);
        printf("[ATWI]Target address: %s\n\r", argv[1]);
        if(argc == 2){
            printf("[ATWI]Repeat Count: 5\n\r");
            do_ping_call(argv[1], 0, 5);	//Not loop, count=5        
        }else{
          if(strcmp(argv[2], "loop") == 0){
            printf("[ATWI]Repeat Count: %s\n\r", "loop");
            do_ping_call(argv[1], 1, 0);	//loop, no count
          }
          else{
            count = atoi(argv[2]);
            printf("[ATWI]Repeat Count: %d\n\r", count);
            do_ping_call(argv[1], 0, count);	//Not loop, with count
          }
        }
}

#if CONFIG_BSD_TCP
void fATWT(void *arg){
	int argc;
	char *argv[MAX_ARGC] = {0};
        printf("[ATWT]: _AT_WLAN_TCP_TEST_\n\r");
        if(!arg){
          printf("[ATWT]Usage: ATWT=[-c/c,xxxx.xxxx.xxxx.xxxx,buf_len,count] or ATWT=[-s/s] or ATWT=[stop]\n\r");
          return;
        }
        argv[0] = "tcp";
        if((argc = parse_param(arg, argv)) > 1){
          cmd_tcp(argc, argv);
          } 
        else
          printf("[ATWT]Usage: ATWT=[-c/c,xxxx.xxxx.xxxx.xxxx,buf_len,count] or ATWT=[-s/s] or ATWT=[stop]\n\r");
}

void fATWU(void *arg){
	int argc;
	char *argv[MAX_ARGC] = {0};
        printf("[ATWU]: _AT_WLAN_UDP_TEST_\n\r");
        if(!arg){
	    printf("[ATWU]Usage: ATWU=[-c/c,xxxx.xxxx.xxxx.xxxx,buf_len,count] or ATWU=[-s/s] or ATWU=[stop]\n\r");
	    return;
	}        
        argv[0] = "ucp";
        if((argc = parse_param(arg, argv)) > 1){
	    cmd_udp(argc, argv);
        }
        else
	    printf("[ATWU]Usage: ATWU=[-c/c,xxxx.xxxx.xxxx.xxxx,buf_len,count] or ATWU=[-s/s] or ATWU=[stop]\n\r");
}
#endif

void fATWM(void *arg){ 
        int argc;
        char *argv[MAX_ARGC] = {0};
        argv[0] = "wifi_promisc";        
		printf("[ATWM]: _AT_WLAN_PROMISC_\n\r");
        if(!arg){
          printf("[ATWM]Usage: ATWM=DURATION_SECONDS[with_len]");
#if CONFIG_INIC_CMD_RSP
		inic_c2h_msg("ATWM", RTW_BADARG, NULL, 0);
#endif
          return;
        }
        if((argc = parse_param(arg, argv)) > 1){
          cmd_promisc(argc, argv);
        }        
}

void fATWL(void *arg){
	int argc;
	char *argv[MAX_ARGC] = {0};
        printf("[ATWL]: _AT_WLAN_SSL_CLIENT_\n\r"); 
        argv[0] = "ssl_client";
        if(!arg){
          printf("ATWL=SSL_SERVER_HOST[SRP_USER_NAME,SRP_PASSWORD]\n\r");
          return;
        }
        if((argc = parse_param(arg, argv)) > 1){
          cmd_ssl_client(argc, argv);
        }        
}


#if CONFIG_WEBSERVER
void fATWE(void *arg){
        printf("[ATWE]: _AT_WLAN_START_WEB_SERVER_\n\r"); 
        start_web_server();
}
#endif

#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
void fATWQ(void *arg){
	int argc=0;
	char *argv[2] = {0};
        printf("[ATWQ]: _AT_WLAN_SIMPLE_CONFIG_\n\r");
        argv[argc++] = "wifi_simple_config";
        if(arg){
          argv[argc++] = arg;
        }
        cmd_simple_config(argc, argv);
}
#endif

#if CONFIG_ENABLE_WPS
void fATWW(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWW]: _AT_WLAN_WPS_\n\r");
        if(!arg){
          printf("[ATWW]Usage: ATWW=pbc/pin\n\r");
          return;
        }
        argv[argc++] = "wifi_wps";
        argv[argc++] = arg;
        cmd_wps(argc, argv);
}
#endif
void fATWw(void *arg){
#ifdef CONFIG_WPS_AP
        int argc = 0;
        char *argv[4];
        printf("[ATWw]: _AT_WLAN_AP_WPS_\n\r");
        if(!arg){
          printf("[ATWw]Usage: ATWw=pbc/pin\n\r");
          return;
        }
        argv[argc++] = "wifi_ap_wps";
        argv[argc++] = arg;
        cmd_ap_wps(argc, argv);
#endif		
}

#if CONFIG_ENABLE_P2P
void fATWG(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWa]: _AT_WLAN_P2P_START_\n\r");
        argv[argc++] = "p2p_start";
        cmd_wifi_p2p_start(argc, argv);
}
void fATWH(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWb]: _AT_WLAN_P2P_STOP_\n\r");
        argv[argc++] = "p2p_stop";
        cmd_wifi_p2p_stop(argc, argv);
}
void fATWJ(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWc]: _AT_WLAN_P2P_CONNECT_\n\r");
        argv[0] = "p2p_connect";
        if(!arg){
		printf("ATWc=[DEST_MAC,pbc/pin]\n\r");
		return;
        }
        if((argc = parse_param(arg, argv)) > 1){
		cmd_p2p_connect(argc, argv);
        }        
}
void fATWK(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWd]: _AT_WLAN_P2P_DISCONNECT_\n\r");
        argv[argc++] = "p2p_disconnect";
        cmd_p2p_disconnect(argc, argv);
}
void fATWN(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWe]: _AT_WLAN_P2P_INFO_\n\r");
        argv[argc++] = "p2p_info";
        cmd_p2p_info(argc, argv);
}
void fATWF(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWf]: _AT_WLAN_P2P_FIND_\n\r");
        argv[argc++] = "p2p_find";
        cmd_p2p_find(argc, argv);
}
#endif
#if CONFIG_OTA_UPDATE
void fATWO(void *arg){
        int argc = 0;
        char *argv[MAX_ARGC] = {0};
        printf("[ATWO]: _AT_WLAN_OTA_UPDATE_\n\r");
        if(!arg){
          printf("[ATWO]Usage: ATWO=IP[PORT] or ATWO= REPOSITORY[FILE_PATH]\n\r");
          return;
        }
        argv[0] = "update";
        if((argc = parse_param(arg, argv)) != 3){
          printf("[ATWO]Usage: ATWO=IP[PORT] or ATWO= REPOSITORY[FILE_PATH]\n\r");
          return;
        }
        cmd_update(argc, argv);
}
#endif
void fATWx(void *arg){
	int i = 0;
#if CONFIG_LWIP_LAYER
	u8 *mac = LwIP_GetMAC(&xnetif[0]);
	u8 *ip = LwIP_GetIP(&xnetif[0]);
	u8 *gw = LwIP_GetGW(&xnetif[0]);
#endif
	u8 *ifname[2] = {WLAN0_NAME,WLAN1_NAME};
	rtw_wifi_setting_t setting;
	printf("[ATW?]: _AT_WLAN_INFO_\n\r"); 
#if CONFIG_INIC_CMD_RSP
	int ret = RTW_SUCCESS;
	int info_sz = 0;
	u8 *info = malloc(NET_IF_NUM*sizeof(rtw_wifi_setting_t)+3*sizeof(rtw_mac_t));
	if(info == NULL)
		ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
#endif
	for(i=0;i<NET_IF_NUM;i++){
		if(rltk_wlan_running(i)){
#if CONFIG_LWIP_LAYER
			mac = LwIP_GetMAC(&xnetif[i]);
			ip = LwIP_GetIP(&xnetif[i]);
			gw = LwIP_GetGW(&xnetif[i]);
#endif
			printf("\n\r\nWIFI %s Status: Running",  ifname[i]);
			printf("\n\r==============================");
			
			rltk_wlan_statistic(i);
			
			wifi_get_setting((const char*)ifname[i],&setting);
			wifi_show_setting((const char*)ifname[i],&setting);
#if CONFIG_INIC_CMD_RSP
			if(info){
				memcpy(info+info_sz, (void *)&setting, sizeof(rtw_wifi_setting_t));
				info_sz += sizeof(rtw_wifi_setting_t);
			}
#endif
			
#if CONFIG_LWIP_LAYER
			printf("\n\rInterface (%s)", ifname[i]);
			printf("\n\r==============================");
			printf("\n\r\tMAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
			printf("\n\r\tIP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			printf("\n\r\tGW  => %d.%d.%d.%d\n\r", gw[0], gw[1], gw[2], gw[3]);
#endif
			if(setting.mode == RTW_MODE_AP || i == 1)
			{
				int client_number;
				struct {
					int    count;
					rtw_mac_t mac_list[AP_STA_NUM];
				} client_info;

				client_info.count = AP_STA_NUM;
				wifi_get_associated_client_list(&client_info, sizeof(client_info));

				printf("\n\rAssociated Client List:");
				printf("\n\r==============================");

				if(client_info.count == 0)
					printf("\n\rClient Num: 0\n\r", client_info.count);
				else
				{
					printf("\n\rClient Num: %d", client_info.count);
					for( client_number=0; client_number < client_info.count; client_number++ )
					{
						printf("\n\rClient %d:", client_number + 1);
						printf("\n\r\tMAC => "MAC_FMT"",
										MAC_ARG(client_info.mac_list[client_number].octet));
#if CONFIG_INIC_CMD_RSP
						if(info){
							memcpy(info+info_sz, (void *)&client_info.mac_list[client_number], sizeof(rtw_mac_t));
							info_sz += sizeof(rtw_mac_t);
						}
#endif						
					}
					printf("\n\r");
				}
			}
		}		
	}

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
	{
		signed char pcWriteBuffer[1024];
		vTaskList((char*)pcWriteBuffer);
		printf("\n\rTask List: \n\r%s", pcWriteBuffer);
	}
#endif

#if CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATW?", ret, NULL, 0);
	else
		inic_c2h_msg("ATW?", RTW_SUCCESS, (char *)info, info_sz);
	
	if(info)
		free(info);
	info = NULL;
#endif
}


#if 1   //xjx-test
/*******************************************************************************
函 数 名：	GetWifiStatus
功能说明： 	获取WiFi的状态
参	  数： 	无
返 回 值：	WiFi的状态 
                1：WiFi无连接（未连接到路由器）
                2：连接到路由器但是没有连接到云服务器
                3：连接到了云服务器
                4：AP模式
注	  意：	
*******************************************************************************/
extern rtw_mode_t gWifiWorkStatus;

void GetWifiStatus(void *arg){
  
	int i = 0;
#if CONFIG_LWIP_LAYER
	u8 *mac = LwIP_GetMAC(&xnetif[0]);
	u8 *ip = LwIP_GetIP(&xnetif[0]);
	u8 *gw = LwIP_GetGW(&xnetif[0]);
#endif
	u8 *ifname[2] = {WLAN0_NAME,WLAN1_NAME};
	rtw_wifi_setting_t setting;
	printf("[ATW?]: _AT_WLAN_INFO_\n\r"); 
#if CONFIG_INIC_CMD_RSP
	int ret = RTW_SUCCESS;
	int info_sz = 0;
	u8 *info = malloc(NET_IF_NUM*sizeof(rtw_wifi_setting_t)+3*sizeof(rtw_mac_t));
	if(info == NULL)
		ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
#endif
	for(i=0;i<NET_IF_NUM;i++){
		if(rltk_wlan_running(i)){
#if CONFIG_LWIP_LAYER
			mac = LwIP_GetMAC(&xnetif[i]);
			ip = LwIP_GetIP(&xnetif[i]);
			gw = LwIP_GetGW(&xnetif[i]);
#endif
			printf("\n\r\nWIFI %s Status: Running",  ifname[i]);
			printf("\n\r==============================");
			
			rltk_wlan_statistic(i);
			
			wifi_get_setting((const char*)ifname[i],&setting);
                        
                        gWifiWorkStatus = setting.mode;          //       
                        
			wifi_show_setting((const char*)ifname[i],&setting);
#if CONFIG_INIC_CMD_RSP
			if(info){
				memcpy(info+info_sz, (void *)&setting, sizeof(rtw_wifi_setting_t));
				info_sz += sizeof(rtw_wifi_setting_t);
			}
#endif
			
#if CONFIG_LWIP_LAYER
			printf("\n\rInterface (%s)", ifname[i]);
			printf("\n\r==============================");
			printf("\n\r\tMAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
			printf("\n\r\tIP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			printf("\n\r\tGW  => %d.%d.%d.%d\n\r", gw[0], gw[1], gw[2], gw[3]);
#endif
			if(setting.mode == RTW_MODE_AP || i == 1)
			{
				int client_number;
				struct {
					int    count;
					rtw_mac_t mac_list[AP_STA_NUM];
				} client_info;

				client_info.count = AP_STA_NUM;
				wifi_get_associated_client_list(&client_info, sizeof(client_info));

				printf("\n\rAssociated Client List:");
				printf("\n\r==============================");

				if(client_info.count == 0)
					printf("\n\rClient Num: 0\n\r", client_info.count);
				else
				{
					printf("\n\rClient Num: %d", client_info.count);
					for( client_number=0; client_number < client_info.count; client_number++ )
					{
						printf("\n\rClient %d:", client_number + 1);
						printf("\n\r\tMAC => "MAC_FMT"",
										MAC_ARG(client_info.mac_list[client_number].octet));
#if CONFIG_INIC_CMD_RSP
						if(info){
							memcpy(info+info_sz, (void *)&client_info.mac_list[client_number], sizeof(rtw_mac_t));
							info_sz += sizeof(rtw_mac_t);
						}
#endif						
					}
					printf("\n\r");
				}
			}
		}		
	}

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
	{
		signed char pcWriteBuffer[1024];
		vTaskList((char*)pcWriteBuffer);
		printf("\n\rTask List: \n\r%s", pcWriteBuffer);
	}
#endif

#if CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATW?", ret, NULL, 0);
	else
		inic_c2h_msg("ATW?", RTW_SUCCESS, (char *)info, info_sz);
	
	if(info)
		free(info);
	info = NULL;
#endif
}
#endif



#if CONFIG_AIRKISS
void fATWX(void *arg)
{
	int ret = RTW_SUCCESS;
	ret = airkiss_start();
#if CONFIG_INIC_EN
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWX", RTW_ERROR, NULL, 0);
#endif	
}
#endif
void fATWZ(void *arg){
        char buf[32] = {0};
        char *copy = buf;
        int i = 0;
        int len = 0;
	int ret = RTW_SUCCESS;

        printf("[ATWZ]: _AT_WLAN_IWPRIV_\n\r");
        if(!arg){
          printf("[ATWZ]Usage: ATWZ=COMMAND[PARAMETERS]\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
        strcpy(copy, arg);
        len = strlen(copy);
        do{
          if((*(copy+i)=='['))
            *(copy+i)=' ';
          if((*(copy+i)==']')||(*(copy+i)=='\0')){
            *(copy+i)='\0';
            break;
          }
        }while((i++) < len);
        
        i = 0;
        do{
          if((*(copy+i)==',')) {
            *(copy+i)=' ';
            break;
          }
        }while((i++) < len);
        
#if CONFIG_INIC_CMD_RSP
	ret = wext_private_command_with_retval(WLAN0_NAME, copy, buf, 32);
	if(ret == RTW_SUCCESS)
		inic_c2h_msg("ATWZ", ret, buf, strlen(buf));
#else
        wext_private_command(WLAN0_NAME, copy, 1);
#endif
exit:
#if CONFIG_INIC_EN
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWZ", ret, NULL, 0);
#endif
	return;	// exit label cannot be last statement
}

#ifdef CONFIG_POWER_SAVING
void fATXP(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int ret = 0;
	int mode, dtim;
	int tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3;

	printf("[ATXP]: _AT_WLAN_POWER_MODE_\r\n");

	if (!arg) {
		printf("[ATXP] Usage: ATXP=lps/ips/dtim/tdma[mode]\r\n");
		return;
	} else {
		argc = parse_param(arg, argv);
		if (argc < 3) {
			printf("[ATXP] Usage: ATXP=lps/ips/dtim/tdma[mode]\r\n");
			return;
		}
	}

	if (strcmp(argv[1], "lps") == 0) {
		mode = atoi(argv[2]);
		if (mode >= 0 && mode < 0xFF) {
			printf("lps mode:%d\r\n", mode);
			wifi_set_power_mode(0xff, mode);
		}
	}

	if (strcmp(argv[1], "ips") == 0) {
		mode = atoi(argv[2]);
		if (mode >= 0 && mode < 0xFF) {
			printf("ips mode:%d\r\n", mode);
			wifi_set_power_mode(mode, 0xFF);
		}
	}

	if (strcmp(argv[1], "tdma") == 0) {
		if (argc >= 6) {
			tdma_slot_period = atoi(argv[2]);
			tdma_rfon_period_len_1 = atoi(argv[3]);
			tdma_rfon_period_len_2 = atoi(argv[4]);
			tdma_rfon_period_len_3 = atoi(argv[5]);
			printf("tdma param: %d %d %d %d\r\n", tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3);
			wifi_set_tdma_param(tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3);
		}
	}

	if (strcmp(argv[1], "dtim") == 0) {
		dtim = atoi(argv[2]);
		printf("dtim: %d\r\n", dtim);
		wifi_set_lps_dtim(dtim);
	}
}
#endif



void fATWSS(void *arg)
{
     
}
void print_wlan_help(void *arg){
	printf("\n\rWLAN AT COMMAND SET:");
	printf("\n\r==============================");
        printf("\n\r1. Wlan Scan for Network Access Point");
        printf("\n\r   # ATWS");
        printf("\n\r2. Connect to an AES AP");
        printf("\n\r   # ATW0=SSID");
        printf("\n\r   # ATW1=PASSPHRASE");
        printf("\n\r   # ATWC");
        printf("\n\r3. Create an AES AP");
        printf("\n\r   # ATW3=SSID");
        printf("\n\r   # ATW4=PASSPHRASE");
        printf("\n\r   # ATW5=CHANNEL");
        printf("\n\r   # ATWA");
        printf("\n\r4. Ping");
        printf("\n\r   # ATWI=xxx.xxx.xxx.xxx");
}

log_item_t at_wifi_items[ ] = {
	{"ATW0", fATW0,},
	{"ATW1", fATW1,},
	{"ATW2", fATW2,},
	{"ATW3", fATW3,},
	{"ATW4", fATW4,},
	{"ATW5", fATW5,},
	{"ATW6", fATW6,},	
	{"ATWA", fATWA,},
#ifdef  CONFIG_CONCURRENT_MODE
        {"ATWB", fATWB,},
#endif
	{"ATWC", fATWC,},
	{"ATWD", fATWD,},
	{"ATWP", fATWP,},
	{"ATWR", fATWR,},
	{"ATWS", fATWS,},
#if SCAN_WITH_SSID
	{"ATWs", fATWs,},
#endif
	{"ATWM", fATWM,},
        {"ATWZ", fATWZ,},
#if CONFIG_OTA_UPDATE
	{"ATWO", fATWO,},
#endif
#if CONFIG_WEBSERVER
	{"ATWE", fATWE,}, 
#endif
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)	
	{"ATWQ", fATWQ,},
#endif	
#ifdef CONFIG_WPS	
#if CONFIG_ENABLE_WPS
	{"ATWW", fATWW,},
#endif	
	{"ATWw", fATWw,}, //wps registrar for softap
#if CONFIG_ENABLE_P2P
	{"ATWG", fATWG,},  //p2p start
	{"ATWH", fATWH,},  //p2p stop
	{"ATWJ", fATWJ,},  //p2p connect
	{"ATWK", fATWK,},  //p2p disconnect
	{"ATWN", fATWN,},  //p2p info
	{"ATWF", fATWF,},  //p2p find
#endif
#endif
#if CONFIG_SSL_CLIENT
	{"ATWL", fATWL,},
#endif
#if CONFIG_LWIP_LAYER
	{"ATWI", fATWI,}, 
#if CONFIG_BSD_TCP
	{"ATWT", fATWT,},
	{"ATWU", fATWU,},
#endif
#endif
#if CONFIG_AIRKISS
	{"ATWX", fATWX,}, 
#endif
	{"ATW?", fATWx,},
	{"ATW+ABC", fATWx,},
#ifdef CONFIG_POWER_SAVING
	{"ATXP", fATXP,},
#endif
        
        {"ATWSS",fATWSS},//xjx，test
};

void at_wifi_init(void)
{
	init_wifi_struct();
	log_service_add_table(at_wifi_items, sizeof(at_wifi_items)/sizeof(at_wifi_items[0]));
}

log_module_init(at_wifi_init);

#endif // end of #if CONFIG_WLAN
#endif //end of #if SUPPORT_LOG_SERVICE
