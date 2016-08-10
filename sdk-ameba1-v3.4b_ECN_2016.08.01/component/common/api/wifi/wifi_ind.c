//----------------------------------------------------------------------------//
#include "wifi/wifi_ind.h"
#include "wifi/wifi_conf.h"
#include "osdep_service.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define WIFI_INDICATE_MSG	0
#define WIFI_MANAGER_STACKSIZE	128
#define WIFI_MANAGER_PRIORITY		(5)
#define WIFI_MANAGER_PRIO	2

#define INDICATE_USE_THREAD	0
#define WIFI_EVENT_MAX_ROW	3
/******************************************************
 *                    Globals
 ******************************************************/

static event_list_elem_t     event_callback_list[WIFI_EVENT_MAX][WIFI_EVENT_MAX_ROW];
#if INDICATE_USE_THREAD
static rtw_worker_thread_t          wifi_worker_thread;
#endif

//----------------------------------------------------------------------------//
#if INDICATE_USE_THREAD
static rtw_result_t rtw_send_event_to_worker(int event_cmd, char *buf, int buf_len, int flags)
{
	rtw_event_message_t message;
	int i;
	rtw_result_t ret = RTW_SUCCESS;
	
	if(event_cmd >= WIFI_EVENT_MAX)
		return RTW_BADARG;
	for(i = 0; i < WIFI_EVENT_MAX_ROW; i++){
		if(event_callback_list[event_cmd][i].handler == NULL)
			continue;

		message.function = (event_handler_t)event_callback_list[event_cmd][i].handler;
		message.buf = buf;
		message.buf_len = buf_len;
		message.flags = flags;
		message.user_data = event_callback_list[event_cmd][i].handler_user_data;

		ret = rtw_push_to_xqueue(&wifi_worker_thread->event_queue, &message, 0);
		if(ret != RTW_SUCCESS)
			break;
	}
	return ret;
}
#else
static rtw_result_t rtw_indicate_event_handle(int event_cmd, char *buf, int buf_len, int flags)
{
	rtw_event_handler_t handle = NULL;
	int i;
	
	if(event_cmd >= WIFI_EVENT_MAX)
		return RTW_BADARG;
	
	for(i = 0; i < WIFI_EVENT_MAX_ROW; i++){
		handle = event_callback_list[event_cmd][i].handler;
		if(handle == NULL)
			continue;
		handle(buf, buf_len, flags, event_callback_list[event_cmd][i].handler_user_data);
	}

	return RTW_SUCCESS;
}
#endif

void wifi_indication( WIFI_EVENT_INDICATE event, char *buf, int buf_len, int flags)
{
	//
	// If upper layer application triggers additional operations on receiving of wext_wlan_indicate,
	// 		please strictly check current stack size usage (by using uxTaskGetStackHighWaterMark() ) 
	//		, and tries not to share the same stack with wlan driver if remaining stack space is 
	//		not available for the following operations. 
	//		ex: using semaphore to notice another thread.
	switch(event)
	{
		case WIFI_EVENT_DISCONNECT:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r %s():Disconnection indication received", __FUNCTION__);
#endif
			break;
		case WIFI_EVENT_CONNECT:
			// For WPA/WPA2 mode, indication of connection does not mean data can be  
			// 		correctly transmitted or received. Data can be correctly transmitted or
			// 		received only when 4-way handshake is done. 
			// Please check WIFI_EVENT_FOURWAY_HANDSHAKE_DONE event
#if(WIFI_INDICATE_MSG==1)						
			// Sample: return mac address
			if(buf != NULL && buf_len == 6)
			{
				printf("\n\r%s():Connect indication received: %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, 
							buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
			}
#endif
			break;
		case WIFI_EVENT_FOURWAY_HANDSHAKE_DONE:
#if(WIFI_INDICATE_MSG==1)			
			if(buf != NULL)			
			{
				if(buf_len == strlen(IW_EXT_STR_FOURWAY_DONE))
					printf("\n\r%s():%s", __FUNCTION__, buf);
			}
#endif
			break;
		case WIFI_EVENT_SCAN_RESULT_REPORT:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_SCAN_RESULT_REPORT\n", __func__);
#endif
			break;
		case WIFI_EVENT_SCAN_DONE:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_SCAN_DONE\n", __func__);
#endif
			break;
		case WIFI_EVENT_RECONNECTION_FAIL:
#if(WIFI_INDICATE_MSG==1)			
			if(buf != NULL){
				if(buf_len == strlen(IW_EXT_STR_RECONNECTION_FAIL))
					printf("\n\r%s", buf);
			}
#endif
			break;
		case WIFI_EVENT_NO_NETWORK:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_NO_NETWORK\n", __func__);
#endif
			break;
#ifdef CONFIG_P2P_NEW
		case WIFI_EVENT_SEND_ACTION_DONE:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_SEND_ACTION_DONE\n", __func__);
#endif
			break;
		case WIFI_EVENT_RX_MGNT:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_RX_MGNT\n", __func__);
#endif
			break;
		case WIFI_EVENT_STA_ASSOC:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_STA_ASSOC\n", __func__);
#endif
			break;
		case WIFI_EVENT_STA_DISASSOC:
#if(WIFI_INDICATE_MSG==1)			
			printf("\n\r%s(): WIFI_EVENT_STA_DISASSOC\n", __func__);
#endif
			break;
#endif //CONFIG_P2P_NEW
#ifdef CONFIG_WPS
		case WIFI_EVENT_WPS_FINISH:
#if(WIFI_INDICATE_MSG==1)
			printf("\n\r%s(): WIFI_EVENT_WPS_FINISH\n", __func__);
#endif
			break;
		case WIFI_EVENT_EAPOL_RECVD:
#if(WIFI_INDICATE_MSG==1)
			printf("\n\r%s(): WIFI_EVENT_EAPOL_RECVD\n", __func__);
#endif
			break;
#endif
		case WIFI_EVENT_BEACON_AFTER_DHCP:
#if(WIFI_INDICATE_MSG==1)
			printf("\n\r%s(): WIFI_EVENT_BEACON_AFTER_DHCP\n", __func__);
#endif
			break;
	}

#if CONFIG_INIC_EN
	inic_indicate_event(event, buf, buf_len, flags);
#endif//CONFIG_INIC_EN

#if INDICATE_USE_THREAD
	rtw_send_event_to_worker(event, buf, buf_len, flags);
#else
	rtw_indicate_event_handle(event, buf, buf_len, flags);
#endif
}

void wifi_reg_event_handler(unsigned int event_cmds, rtw_event_handler_t handler_func, void *handler_user_data)
{
	int i = 0, j = 0;
	if(event_cmds < WIFI_EVENT_MAX){
		for(i=0; i < WIFI_EVENT_MAX_ROW; i++){
			if(event_callback_list[event_cmds][i].handler == NULL){
			    for(j=0; j<WIFI_EVENT_MAX_ROW; j++){
			        if(event_callback_list[event_cmds][j].handler == handler_func){
			            return;
			        }
			    }
				event_callback_list[event_cmds][i].handler = handler_func;
				event_callback_list[event_cmds][i].handler_user_data = handler_user_data;
				return;
			}
		}
	}
}

void wifi_unreg_event_handler(unsigned int event_cmds, rtw_event_handler_t handler_func)
{
	int i;
	if(event_cmds < WIFI_EVENT_MAX){
		for(i = 0; i < WIFI_EVENT_MAX_ROW; i++){
			if(event_callback_list[event_cmds][i].handler == handler_func){
				event_callback_list[event_cmds][i].handler = NULL;
				event_callback_list[event_cmds][i].handler_user_data = NULL;
				return;
			}
		}
	}
}

void init_event_callback_list(){
	memset(event_callback_list, 0, sizeof(event_callback_list));
}

int wifi_manager_init()
{
#if INDICATE_USE_THREAD
	rtw_create_worker_thread(&wifi_worker_thread, 
							WIFI_MANAGER_PRIORITY, 
							WIFI_MANAGER_STACKSIZE, 
							WIFI_MANAGER_PRIO);
#endif
	return 0;
}

void rtw_wifi_manager_deinit()
{
#if INDICATE_USE_THREAD
	rtw_delete_worker_thread(&wifi_worker_thread);
#endif
}

