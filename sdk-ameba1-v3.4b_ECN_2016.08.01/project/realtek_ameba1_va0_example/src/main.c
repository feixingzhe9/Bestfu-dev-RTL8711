
 //original

#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>
#include "wifi_constants.h"

#include "semphr.h"

#include "myIIC.h"



extern void console_init(void);
extern xTaskHandle g_client_task;  
extern xTaskHandle g_server_task;

rtw_mode_t gWifiWorkStatus = RTW_MODE_AP;
unsigned char gWifiConnectStatus = 0;//WiFi连接情况： 0：未连接到路由器       1：连接到路由器，但未连接到云    2：连接到云
unsigned char gDevIP[4] = {0};
unsigned char gDevGW[4] = {0};
unsigned char gDevMac[6] = {0};




xTaskHandle gTimeDelayTestTask = NULL;
xTaskHandle gSHT20TestTask = NULL;
xTaskHandle gWaterLevelTestTask = NULL;

xTaskHandle gauto_watering_task = NULL;
xTaskHandle gget_temp_humidity_task = NULL;

xSemaphoreHandle sht20_handle = NULL;

struct watering_t watering_param;            
struct watering_t *pwatering_param = &watering_param;


extern xTaskHandle manual_watering_handle;

#if  1 


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
        uint16_t shtc1_id;
        
        
         
        pwatering_param->watering_period = 100;
        pwatering_param->watering_interval = 100;
        pwatering_param->pwm_period = 1.0/2000;
        pwatering_param->pwm_pulsewidth = pwatering_param->pwm_period/2;
        pwatering_param->cnt = 8;
        
        printf("\n main function");

        printf("\n test for j-link");

        printf("\n Version 1.23");
        
        sht20_handle = xSemaphoreCreateMutex();//创建SHT20 温湿度 的信号量
                  
            
	if ( rtl_cryptoEngine_init() != 0 ) {
		DiagPrintf("crypto engine init failed\r\n");
	}
        
        
        
	/* Initialize log uart and at command service */
	console_init();	

	/* pre-processor of application example */
	pre_example_entry();

	/* wlan intialization */
#if defined(CONFIG_WIFI_NORMAL) && defined(CONFIG_NETWORK)
	wlan_network();
#endif

	/* Execute application example */
	example_entry();
        

#define BSD_STACK_SIZE_TEST		512        
#if 0    //xjx-test 
            
        
#if 0          //作为Client，主要
        printf("\n creating TcpClientHandler_test task ");
        if(xTaskCreate(TcpClientHandler_test, "tcp_client", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, &g_client_task) != pdPASS)
		printf("\n\rTCP ERROR: Create tcp client task failed.");
#endif 
        
#if 0           //作为Server
        if(xTaskCreate(TcpServerHandler_test, "tcp_server", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, &g_server_task) != pdPASS)
			printf("\n\rTCP ERROR: Create tcp server task failed.");
#endif 
        
#if 0           //UDP模式 ，接收SSID和Password    
        if(xTaskCreate(Udpserverhandler_test, "Udpserverhandler_test", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, &g_client_task) != pdPASS)
		printf("\n\rTCP ERROR: Create udp server task failed.");   
#endif   
        
        
#endif
        
#if  0           //iic获取与设置时间测试
        if(xTaskCreate(Example_BL5372_test, "Example_BL5372_test", BSD_STACK_SIZE_TEST, NULL, tskIDLE_PRIORITY + 2 + PRIORITIE_OFFSET, &gTimeDelayTestTask) != pdPASS)
		printf("\n\rTCP ERROR: Create tcp Example_BL5372_test task failed.");

#endif        
        
#if 0           //shtc20
        if(xTaskCreate(example_shtc20_test, ((const char*)"example_shtc20_test"), 512, NULL, tskIDLE_PRIORITY + 3, &gSHT20TestTask) != pdPASS)
		printf("\n\r%s xTaskCreate(example_shtc20_test) failed", __FUNCTION__);
#endif

#if 0           //EEPROM
        if(xTaskCreate(example_EEPROM_test, ((const char*)"example_EEPROM_test"), 512, NULL, tskIDLE_PRIORITY + 4, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(example_EEPROM_test) failed", __FUNCTION__);
#endif   
      
#if 1
        if(xTaskCreate(ExampleWaterTest, ((const char*)"ExampleWaterTest"), 512, NULL, tskIDLE_PRIORITY + 5, &gWaterLevelTestTask) != pdPASS)
		printf("\n\r%s xTaskCreate(ExampleWaterTest) failed", __FUNCTION__);
#endif
        
#if 1           
            printf("\n starting create manual_watering_task\n");
            if(xTaskCreate(manual_watering_task, ((const char*)"manual_watering_task"), 1000, (void*)pwatering_param, tskIDLE_PRIORITY + 5, &manual_watering_handle) != pdPASS)
                printf("\n\r%s xTaskCreate(manual_watering_task) failed", __FUNCTION__);
            printf("\n create manual_watering_task is done \n");  

#endif  
#if 1     
            
            printf("\n starting create auto_watering_task\n");
            if(xTaskCreate(auto_watering_task, ((const char*)"auto_watering_task"), 512, (void*)pwatering_param, tskIDLE_PRIORITY + 5, &manual_watering_handle) != pdPASS)
                printf("\n\r%s auto_watering_task(manual_watering_task) failed", __FUNCTION__);
            printf("\n create manual_watering_task is done \n");  

#endif 
            
#if 1     
            
            printf("\n starting create get_temp_humidity_task\n");
            if(xTaskCreate(get_temp_humidity_task, ((const char*)"get_temp_humidity_task"), 512, (void*)pwatering_param, tskIDLE_PRIORITY + 5, &gget_temp_humidity_task) != pdPASS)
                printf("\n\r%s auto_watering_task(get_temp_humidity_task) failed", __FUNCTION__);
            printf("\n create manual_watering_task is done \n");  

#endif 
    	/*Enable Schedule, Start Kernel*/
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
	#ifdef PLATFORM_FREERTOS
	vTaskStartScheduler();
	#endif
#else
	RtlConsolTaskRom(NULL);
#endif
        
       
}
#endif








