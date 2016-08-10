#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "udp.h"
#include <sockets.h>
#include <lwip_netconf.h>
#include <osdep_service.h>
#include "platform_stdlib.h"
#include "wifi_simple_config_parser.h"



#define STACKSIZE     512


#if CONFIG_WLAN
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
#include "wifi/wifi_conf.h"
int is_promisc_callback_unlock = 0;
static int is_fixed_channel;
int fixed_channel_num;
unsigned char g_ssid[32];
int g_ssid_len;

extern int promisc_get_fixed_channel( void *, u8 *, int* );
struct pattern_test_ops;
struct rtk_test_sc;

typedef int (*sc_test_check_pattern_call_back)(struct pattern_test_ops *pp, struct rtk_test_sc *pSc);
typedef int (*sc_test_get_cipher_info_call_back)(struct pattern_test_ops *pp, struct rtk_test_sc *pSc);
typedef int (*sc_test_generate_key_call_back)(struct pattern_test_ops *pp, struct rtk_test_sc *pSc);
typedef int (*sc_test_decode_profile_call_back)(struct pattern_test_ops *pp, struct rtk_test_sc *pSc);
typedef int (*sc_test_get_tlv_info_call_back)(struct pattern_test_ops *pp, struct rtk_test_sc *pSc);

struct pattern_test_ops {
	unsigned int index;
	unsigned flag;				
	unsigned char name[64]; 		
	sc_test_check_pattern_call_back check_pattern;	
	sc_test_get_cipher_info_call_back get_cipher_info;
	sc_test_generate_key_call_back generate_key;
	sc_test_decode_profile_call_back decode_profile;
	sc_test_get_tlv_info_call_back get_tlv_info;
};


struct rtk_test_sc {
	/* API exposed to user */
	unsigned char		ssid[32];
	unsigned char		password[65];	
	unsigned int		ip_addr;
};

enum sc_result {
	SC_ERROR = -1,	/* default error code*/
	SC_NO_CONTROLLER_FOUND = 1, /* cannot get sta(controller) in the air which starts a simple config session */
	SC_CONTROLLER_INFO_PARSE_FAIL, /* cannot parse the sta's info  */
	SC_TARGET_CHANNEL_SCAN_FAIL, /* cannot scan the target channel */
	SC_JOIN_BSS_FAIL, /* fail to connect to target ap */
	SC_DHCP_FAIL, /* fail to get ip address from target ap */
	 /* fail to create udp socket to send info to controller. note that client isolation
		must be turned off in ap. we cannot know if ap has configured this */
	SC_UDP_SOCKET_CREATE_FAIL,
	SC_SUCCESS,	/* default success code */

};

#ifdef PACK_STRUCT_USE_INCLUDES
#include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct ack_msg {
	PACK_STRUCT_FIELD(u8_t flag);
	PACK_STRUCT_FIELD(u16_t length);
	PACK_STRUCT_FIELD(u8_t smac[6]);
	PACK_STRUCT_FIELD(u8_t status);
	PACK_STRUCT_FIELD(u16_t device_type);
	PACK_STRUCT_FIELD(u32_t device_ip);
	PACK_STRUCT_FIELD(u8_t device_name[64]);
};PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#include "arch/epstruct.h"
#endif



#define 	MULCAST_PORT		(8864)

#define 	SCAN_BUFFER_LENGTH	(1024)

#ifndef WLAN0_NAME
  #define WLAN0_NAME		"wlan0"
#endif

int simple_config_result;
static struct ack_msg  *ack_content;
struct rtk_test_sc *backup_sc_ctx;
extern struct netif xnetif[NET_IF_NUM];
void SC_set_ack_content()
{
	memset(ack_content, 0, sizeof(struct ack_msg));
	ack_content->flag = 0x20;
	ack_content->length = htons(sizeof(struct ack_msg)-3);
	memcpy(ack_content->smac, xnetif[0].hwaddr, 6);
	ack_content->status = 0;
	ack_content->device_type = 0;
	ack_content->device_ip = xnetif[0].ip_addr.addr;
	memset(ack_content->device_name, 0, 64);
}

int SC_send_simple_config_ack()
{
#if CONFIG_LWIP_LAYER
	int ack_transmit_round, ack_num_each_sec;
	int ack_socket;
	//int sended_data = 0;
	struct sockaddr_in to_addr;

	SC_set_ack_content();
	
	ack_socket = socket(PF_INET, SOCK_DGRAM, IP_PROTO_UDP);
	if (ack_socket == -1) {
		return 	-1;
	}
	
	FD_ZERO(&to_addr);
	to_addr.sin_family = AF_INET;
	to_addr.sin_port = htons(8864);
	to_addr.sin_addr.s_addr = (backup_sc_ctx->ip_addr);

	for (ack_transmit_round = 0;ack_transmit_round < 10; ack_transmit_round++) {
		for (ack_num_each_sec = 0;ack_num_each_sec < 10; ack_num_each_sec++) {
			//sended_data = 
			sendto(ack_socket, (unsigned char *)ack_content, sizeof(struct ack_msg), 0, (struct sockaddr *) &to_addr, sizeof(struct sockaddr));
			//printf("\r\nAlready send %d bytes data\n", sended_data);
			vTaskDelay(100);	/* delay 100 ms */
		}
	}
	close(ack_socket);
#endif

#if CONFIG_INIC_CMD_RSP
	extern unsigned int inic_sc_ip_addr;
	inic_sc_ip_addr = backup_sc_ctx->ip_addr;
	inic_c2h_wifi_info("ATWQ", RTW_SUCCESS);
#endif

	return 0;
}



static enum sc_result SC_check_and_show_connection_info(void)
{
	rtw_wifi_setting_t setting;	
	int ret = -1;

#if CONFIG_LWIP_LAYER
	/* If not rise priority, LwIP DHCP may timeout */
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 3);	
	/* Start DHCP Client */
	ret = LwIP_DHCP(0, DHCP_START);
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 1);	
#endif	
	
	wifi_get_setting(WLAN0_NAME, &setting);
	wifi_show_setting(WLAN0_NAME, &setting);

#if CONFIG_LWIP_LAYER
	if (ret != DHCP_ADDRESS_ASSIGNED)
		return SC_DHCP_FAIL;
	else
#endif
		return SC_SUCCESS;
}

static void check_and_set_security_in_connection(rtw_security_t security_mode, rtw_network_info_t *wifi) 
{
  	
	if (security_mode == RTW_SECURITY_WPA2_AES_PSK) {					
		printf("\r\nwifi->security_type = RTW_SECURITY_WPA2_AES_PSK\n");
		wifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
	} else if (security_mode == RTW_SECURITY_WEP_PSK) {
		printf("\r\nwifi->security_type = RTW_SECURITY_WEP_PSK\n");
		wifi->security_type = RTW_SECURITY_WEP_PSK;
		wifi->key_id = 0;
	} else if (security_mode == RTW_SECURITY_WPA_AES_PSK) {
		printf("\r\nwifi->security_type = RTW_SECURITY_WPA_AES_PSK\n");
		wifi->security_type = RTW_SECURITY_WPA_AES_PSK;
	} else {
		printf("\r\nwifi->security_type = RTW_SECURITY_OPEN\n");
		wifi->security_type = RTW_SECURITY_OPEN;
	}
}

static int get_connection_info_from_profile(rtw_security_t security_mode, rtw_network_info_t *wifi)
{

	printf("\r\n======= Connection Information =======\n");
	check_and_set_security_in_connection(security_mode, wifi);

	wifi->password = backup_sc_ctx->password;
	wifi->password_len = (int)strlen((char const *)backup_sc_ctx->password);

	if ( g_ssid_len > 0 && g_ssid_len < 33){
		wifi->ssid.len = g_ssid_len;
		rtw_memcpy(wifi->ssid.val, g_ssid, wifi->ssid.len);
	}else if(strlen(backup_sc_ctx->ssid) > 0 && strlen(backup_sc_ctx->ssid) < 33){
		wifi->ssid.len = strlen(backup_sc_ctx->ssid);
		rtw_memcpy(wifi->ssid.val, backup_sc_ctx->ssid, wifi->ssid.len);
	}else{
		printf("\r\n SSID is NULL or SSID length is over 32!!");
		return -1;
	}
	
	if(wifi->security_type == RTW_SECURITY_WEP_PSK)
	{
		if(wifi->password_len == 10)
		{
			u32 p[5];
			u8 pwd[6], i = 0; 
			sscanf((const char*)backup_sc_ctx->password, "%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4]);
			for(i=0; i< 5; i++)
				pwd[i] = (u8)p[i];
			pwd[5] = '\0';
			memset(backup_sc_ctx->password, 0, 65);
			strcpy((char*)backup_sc_ctx->password, (char*)pwd);
			wifi->password_len = 5;
		}else if(wifi->password_len == 26){
			u32 p[13];
			u8 pwd[14], i = 0;
			sscanf((const char*)backup_sc_ctx->password, "%02x%02x%02x%02x%02x%02x%02x"\
				"%02x%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4],\
				&p[5], &p[6], &p[7], &p[8], &p[9], &p[10], &p[11], &p[12]);
			for(i=0; i< 13; i++)
				pwd[i] = (u8)p[i];
			pwd[13] = '\0';
			memset(backup_sc_ctx->password, 0, 64);
			strcpy((char*)backup_sc_ctx->password, (char*)pwd);
			wifi->password_len = 13;
		}
	}
	printf("\r\nwifi.password = %s\n", wifi->password);
	printf("\r\nwifi.password_len = %d\n", wifi->password_len);
	printf("\r\nwifi.ssid = %s\n", wifi->ssid.val);				
	printf("\r\nwifi.ssid_len = %d\n", wifi->ssid.len);
	printf("\r\nwifi.channel = %d\n", fixed_channel_num);
	printf("\r\n===== start to connect target AP =====\n");
	return 0;
}

enum sc_result SC_connect_to_AP(void)
{
	enum sc_result ret = SC_ERROR;
	u8 scan_channel;
	u8 pscan_config;
	int max_retry = 10, retry = 0;
	rtw_security_t security_mode;
	rtw_network_info_t wifi = {0};
	if(!(fixed_channel_num == 0)){
		scan_channel = fixed_channel_num;
	}
	pscan_config = PSCAN_ENABLE | PSCAN_SIMPLE_CONFIG;
	switch(g_security_mode){
		case RTW_ENCRYPTION_OPEN:
			security_mode = RTW_SECURITY_OPEN;
			break;
		case RTW_ENCRYPTION_WEP40:
		case RTW_ENCRYPTION_WEP104:
			security_mode = RTW_SECURITY_WEP_PSK;
			break;
		case RTW_ENCRYPTION_WPA_TKIP:
		case RTW_ENCRYPTION_WPA_AES:
		case RTW_ENCRYPTION_WPA2_TKIP:
		case RTW_ENCRYPTION_WPA2_AES:
		case RTW_ENCRYPTION_WPA2_MIXED:
			security_mode = RTW_SECURITY_WPA2_AES_PSK;
			break;
		case RTW_ENCRYPTION_UNKNOWN:
		case RTW_ENCRYPTION_UNDEF:
		default:
			printf("\r\n unknow security mode,connect fail!");
			return ret;
	}
	g_security_mode = 0xff;//clear it

	if (-1 == get_connection_info_from_profile(security_mode, &wifi)) {
		ret = SC_CONTROLLER_INFO_PARSE_FAIL;
		return ret;
	}

	while (1) {
		if(wifi_set_pscan_chan(&scan_channel, &pscan_config, 1) < 0){
			printf("\n\rERROR: wifi set partial scan channel fail");
			ret = SC_TARGET_CHANNEL_SCAN_FAIL;
			return ret;
		}
#if 0
				ret = wifi_connect((char*)wifi.ssid.val,
						wifi.security_type,
						(char*)wifi.password,
						wifi.ssid.len,
						wifi.password_len,
						wifi.key_id,
									  NULL);
#else
				ret = wifi_connect_bssid(g_bssid,
									  (char*)wifi.ssid.val,
									  wifi.security_type,
									  (char*)wifi.password,
									  6,
									  wifi.ssid.len,
									  wifi.password_len,
									  wifi.key_id,
									  NULL);
#endif
				if (ret == RTW_SUCCESS) {
					ret = SC_check_and_show_connection_info();
					break;
				}

		if (retry == max_retry) {
			ret = SC_JOIN_BSS_FAIL;
				break;
		}
		retry ++;
		if(retry > 3){
			vTaskDelay(5000);
		}
	}	

	return ret;
}
/* Make callback one by one to wlan rx when promiscuous mode */

void simple_config_callback(unsigned char *buf, unsigned int len, void* userdata)
{
	unsigned char * da = buf;
	unsigned char * sa = buf + ETH_ALEN;
	taskENTER_CRITICAL();
	if (is_promisc_callback_unlock == 1) {
	 	simple_config_result = rtk_start_parse_packet(da, sa, len, userdata, (void *)backup_sc_ctx);
		//printf("\r\nresult in callback function = %d\n",simple_config_result);
	} 
	taskEXIT_CRITICAL();

}

static unsigned int simple_config_cmd_start_time;
static unsigned int simple_config_cmd_current_time;
extern int simple_config_status;
extern void rtk_restart_simple_config(void);


extern void rtk_sc_deinit(void);

void init_simple_config_lib_config(struct simple_config_lib_config* config)
{
	config->free = rtw_mfree;
	config->malloc = rtw_malloc;
	config->memcmp = (simple_config_memcmp_fn)_memcmp;
	config->memcpy = _memcpy;
	config->memset = _memset;
	config->printf = printf;
	config->strcpy = strcpy;
	config->strlen = strlen;
	config->zmalloc = rtw_zmalloc;
	config->_ntohl = lwip_ntohl;

	config->is_promisc_callback_unlock = &is_promisc_callback_unlock;
}


int init_test_data(char *custom_pin_code)
{
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
	is_promisc_callback_unlock = 1;
	is_fixed_channel = 0;
	fixed_channel_num = 0;
	simple_config_result = 0;
	rtw_memset(g_ssid, 0, 32);
	g_ssid_len = 0;
	simple_config_cmd_start_time = xTaskGetTickCount();
	
	if (ack_content != NULL) {
		vPortFree(ack_content);
		ack_content = NULL;
	}
	ack_content = pvPortMalloc(sizeof(struct ack_msg));
	if (!ack_content) {
		printf("\n\rrtk_sc_init fail by allocate ack\n");
	}	
	memset(ack_content, 0, sizeof(struct ack_msg));

	

	backup_sc_ctx = pvPortMalloc(sizeof(struct rtk_test_sc));
	if (!backup_sc_ctx) {
		printf("\n\r[Mem]malloc SC context fail\n");
	} else {
		memset(backup_sc_ctx, 0, sizeof(struct rtk_test_sc));	
		struct simple_config_lib_config lib_config;
		init_simple_config_lib_config(&lib_config);
		//custom_pin_code can be null
		if (rtk_sc_init(custom_pin_code, &lib_config) < 0) {
			printf("\n\rRtk_sc_init fail\n");
		} else {
			return 0;
		}
	}	

#else
	printf("\n\rPlatform no include simple config now\n");
#endif	
	return -1;
}

void deinit_test_data(){
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
	rtk_sc_deinit();
	if (backup_sc_ctx != NULL) {
		vPortFree(backup_sc_ctx);
		backup_sc_ctx = NULL;
	}	
	if (ack_content != NULL) {
		vPortFree(ack_content);
		ack_content = NULL;
	}
#endif
}

enum sc_result simple_config_test(void)
{
	int channel = 1;
	enum sc_result ret = SC_SUCCESS;
	unsigned int start_time;
	int is_need_connect_to_AP = 0;
	int fix_channel = 0;
	int delta_time = 0;
	wifi_set_promisc(RTW_PROMISC_ENABLE, simple_config_callback, 1);
	start_time = xTaskGetTickCount();
	printf("\n\r");
	wifi_set_channel(channel);
	while (1) {
	  	vTaskDelay(50);	//delay 0.5s to release CPU usage
	  	simple_config_cmd_current_time = xTaskGetTickCount();
	  	if (simple_config_cmd_current_time - simple_config_cmd_start_time < ((120 + delta_time)*configTICK_RATE_HZ)) {
			unsigned int current_time = xTaskGetTickCount();
			if (((current_time - start_time)*1000 /configTICK_RATE_HZ < 100)
								|| (is_fixed_channel == 1)) { 	
				if((is_fixed_channel == 0)&&(!((fix_channel = promisc_get_fixed_channel(g_bssid,g_ssid,&g_ssid_len))== 0))){
					//printf("\r\n in simple_config_test fix channel = %d ",fix_channel);
					is_fixed_channel = 1;
					fixed_channel_num = fix_channel;
					wifi_set_channel(fix_channel);
					} 
				if (simple_config_result == 1) {  
					is_need_connect_to_AP = 1;
					is_fixed_channel = 0;	      
					break;
				} 
				if (simple_config_result == -1) {  
					printf("\r\nsimple_config_test restart for result = -1");
					delta_time = 60;
					wifi_set_channel(1);	
					is_need_connect_to_AP = 0;
					is_fixed_channel = 0;
	               		fixed_channel_num = 0;
					memset(g_ssid, 0, 32);
					g_ssid_len = 0;
					simple_config_result = 0;
					g_security_mode = 0xff;
					rtk_restart_simple_config();					
				} 
			} else {
					channel++;
					if ((1 <= channel) && (channel <= 13)) {
						if (wifi_set_channel(channel) == 0) {	
							start_time = xTaskGetTickCount();
							printf("\n\rSwitch to channel(%d)\n", channel);
						}	
					} else {
						channel = 1;
						if (wifi_set_channel(channel) == 0) {	
							start_time = xTaskGetTickCount();
							printf("\n\rSwitch to channel(%d)\n", channel);
						}	
					}	
					
			}
		} else {
			ret = SC_NO_CONTROLLER_FOUND;
			break;
		} 
	}
		wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);
	if (is_need_connect_to_AP == 1) {
		enum sc_result tmp_res = SC_connect_to_AP();
		if (SC_SUCCESS == tmp_res) {
			if(-1 == SC_send_simple_config_ack())
				ret = SC_UDP_SOCKET_CREATE_FAIL;
		} else {
			return tmp_res;
		}

	} else {
		ret = SC_NO_CONTROLLER_FOUND;
	}

	deinit_test_data();
	return ret;
}

//Filter packet da[] = {0x01, 0x00, 0x5e}
#define MASK_SIZE 3
void filter_add_enable(){
	u8 mask[MASK_SIZE]={0xFF,0xFF,0xFF};
	u8 pattern[MASK_SIZE]={0x01,0x00,0x5e};
	
	rtw_packet_filter_pattern_t packet_filter;
	rtw_packet_filter_rule_e rule;

	packet_filter.offset = 0;
	packet_filter.mask_size = 3;
	packet_filter.mask = mask;
	packet_filter.pattern = pattern;
	rule = RTW_POSITIVE_MATCHING;

	wifi_init_packet_filter();
	wifi_add_packet_filter(1, &packet_filter,rule);
	wifi_enable_packet_filter(1);
}
void remove_filter(){
	wifi_disable_packet_filter(1);
	wifi_remove_packet_filter(1);
}

static void print_simple_config_result(enum sc_result sc_code)
{
	printf("\r\n");
	switch (sc_code) {
	case SC_NO_CONTROLLER_FOUND:
		printf("Simple Config timeout!! Can't get Ap profile. Please try again\n"); 
	break;
	case SC_CONTROLLER_INFO_PARSE_FAIL:
		printf("Simple Config fail, cannot parse target ap info from controller\n");
	break;
	case SC_TARGET_CHANNEL_SCAN_FAIL:
		printf("Simple Config cannot scan the target channel\n");
	break;
	case SC_JOIN_BSS_FAIL:
		printf("Simple Config Join bss failed\n");
	break;
	case SC_DHCP_FAIL:
		printf("Simple Config fail, cannot get dhcp ip address\n");
	break;
	case SC_UDP_SOCKET_CREATE_FAIL:
		printf("Simple Config Ack socket create fail!!!\n");
	break;
	case SC_SUCCESS:
		printf("Simple Config success\n");
	break;

	case SC_ERROR:
	default:
		printf("unknown error when simple config!\n");

	}

}




#endif //CONFIG_INCLUDE_SIMPLE_CONFIG

void cmd_simple_config(int argc, char **argv){
#if CONFIG_INCLUDE_SIMPLE_CONFIG
	char *custom_pin_code = NULL;
	enum sc_result ret = SC_ERROR;

	if(argc > 2){
		printf("\n\rInput Error!");
	}

	if(argc == 2)
		custom_pin_code = (argv[1]);

	wifi_enter_promisc_mode();
	if(init_test_data(custom_pin_code) == 0){
		filter_add_enable();
		ret = simple_config_test();
		print_simple_config_result(ret);
		remove_filter();
	}
#if CONFIG_INIC_CMD_RSP
	if(ret != SC_SUCCESS)
		inic_c2h_wifi_info("ATWQ", RTW_ERROR); 
#endif
#endif	
}
#endif	//#if CONFIG_WLAN
