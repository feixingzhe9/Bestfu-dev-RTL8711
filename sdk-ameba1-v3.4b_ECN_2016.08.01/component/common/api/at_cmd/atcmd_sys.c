#include <platform_stdlib.h>
#include <platform_opts.h>
#include <hal_adc.h>
#include <gpio_api.h>   // mbed
#include <sys_api.h>
#include <rtl_lib.h>
#include <build_info.h>
#include "analogin_api.h"
#include "log_service.h"
#include "atcmd_sys.h"
#include "osdep_api.h"

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
#include "freertos_pmu.h"
#endif

extern u32 ConfigDebugErr;
extern u32 ConfigDebugInfo;
extern u32 ConfigDebugWarn;
extern u32 CmdDumpWord(IN u16 argc, IN u8 *argv[]);
extern u32 CmdWriteWord(IN u16 argc, IN u8 *argv[]);
#if SUPPORT_UART_YMODEM
extern int uart_ymodem(void);
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
static char cBuffer[512];
#endif

//-------- AT SYS commands ---------------------------------------------------------------

void fATSD(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	
	AT_DBG_MSG(AT_FLAG_DUMP, AT_DBG_ALWAYS, "[ATSD]: _AT_SYSTEM_DUMP_REGISTER_");
	if(!arg){
		AT_DBG_MSG(AT_FLAG_DUMP, AT_DBG_ALWAYS, "[ATSD] Usage: ATSD=REGISTER");
		return;
	}
	argc = parse_param(arg, argv);
	if(argc == 2 || argc == 3)
		CmdDumpWord(argc-1, (unsigned char**)(argv+1));
}

void fATSE(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	
	AT_DBG_MSG(AT_FLAG_EDIT, AT_DBG_ALWAYS, "[ATSE]: _AT_SYSTEM_EDIT_REGISTER_");
	if(!arg){
		AT_DBG_MSG(AT_FLAG_EDIT, AT_DBG_ALWAYS, "[ATSE] Usage: ATSE=REGISTER[VALUE]");
		return;
	}
	argc = parse_param(arg, argv);
	if(argc == 3)
		CmdWriteWord(argc-1, (unsigned char**)(argv+1));
}

#if SUPPORT_UART_YMODEM
void fATSY(void *arg)
{
	uart_ymodem();
}
#endif

#if SUPPORT_MP_MODE
void fATSA(void *arg)
{
	u32 tConfigDebugInfo = ConfigDebugInfo;
	int argc = 0, channel;
	char *argv[MAX_ARGC] = {0}, *ptmp;
	u16 offset, gain;
	
	AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA]: _AT_SYSTEM_ADC_TEST_");
	if(!arg){
		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] Usage: ATSA=CHANNEL(0~2)");
		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] Usage: ATSA=k_get");
		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] Usage: ATSA=k_set[offet(hex),gain(hex)]");
		return;
	}
	
	argc = parse_param(arg, argv);
	if(strcmp(argv[1], "k_get") == 0){
		sys_adc_calibration(0, &offset, &gain);
//		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] offset = 0x%04X, gain = 0x%04X", offset, gain);
	}else if(strcmp(argv[1], "k_set") == 0){
		if(argc != 4){
			AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] Usage: ATSA=k_set[offet(hex),gain(hex)]");
			return;
		}
		offset = strtoul(argv[2], &ptmp, 16);
		gain = strtoul(argv[3], &ptmp, 16);
		sys_adc_calibration(1, &offset, &gain);
//		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] offset = 0x%04X, gain = 0x%04X", offset, gain);
	}else{
		channel = atoi(argv[1]);
		if(channel < 0 || channel > 2){
			AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] Usage: ATSA=CHANNEL(0~2)");
			return;
		}
		analogin_t   adc;
		u16 adcdat;
		
		// Remove debug info massage
		ConfigDebugInfo = 0;
		if(channel == 0)
			analogin_init(&adc, AD_1);
		else if(channel == 1)
			analogin_init(&adc, AD_2);
		else
			analogin_init(&adc, AD_3);
		adcdat = analogin_read_u16(&adc)>>4;
		analogin_deinit(&adc);
		// Recover debug info massage
		ConfigDebugInfo = tConfigDebugInfo;
		
		AT_DBG_MSG(AT_FLAG_ADC, AT_DBG_ALWAYS, "[ATSA] A%d = 0x%04X", channel, adcdat);
	}
}

void fATSG(void *arg)
{
    gpio_t gpio_test;
    int argc = 0, val;
	char *argv[MAX_ARGC] = {0}, port, num;
	PinName pin = NC;
	u32 tConfigDebugInfo = ConfigDebugInfo;
    
	AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSG]: _AT_SYSTEM_GPIO_TEST_");
	if(!arg){
		AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSG] Usage: ATSG=PINNAME(ex:A0)");
		return;
	}else{
		argc = parse_param(arg, argv);
		if(argc != 2){
			AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSG] Usage: ATSG=PINNAME(ex:A0)");
			return;
		}
	}
	port = argv[1][0];
	num = argv[1][1];
	if(port >= 'a' && port <= 'z')
		port -= ('a' - 'A');
	if(num >= 'a' && num <= 'z')
		num -= ('a' - 'A');
	switch(port){
		case 'A':
			switch(num){
				case '0': pin = PA_0; break; case '1': pin = PA_1; break; case '2': pin = PA_2; break; case '3': pin = PA_3; break;
				case '4': pin = PA_4; break; case '5': pin = PA_5; break; case '6': pin = PA_6; break; case '7': pin = PA_7; break;
			}
			break;
		case 'B':
			switch(num){
				case '0': pin = PB_0; break; case '1': pin = PB_1; break; case '2': pin = PB_2; break; case '3': pin = PB_3; break;
				case '4': pin = PB_4; break; case '5': pin = PB_5; break; case '6': pin = PB_6; break; case '7': pin = PB_7; break;
			}
			break;
		case 'C':
			switch(num){
				case '0': pin = PC_0; break; case '1': pin = PC_1; break; case '2': pin = PC_2; break; case '3': pin = PC_3; break;
				case '4': pin = PC_4; break; case '5': pin = PC_5; break; case '6': pin = PC_6; break; case '7': pin = PC_7; break;
				case '8': pin = PC_8; break; case '9': pin = PC_9; break;
			}
			break;
		case 'D':
			switch(num){
				case '0': pin = PD_0; break; case '1': pin = PD_1; break; case '2': pin = PD_2; break; case '3': pin = PD_3; break;
				case '4': pin = PD_4; break; case '5': pin = PD_5; break; case '6': pin = PD_6; break; case '7': pin = PD_7; break;
				case '8': pin = PD_8; break; case '9': pin = PD_9; break;
			}
			break;
		case 'E':
			switch(num){
				case '0': pin = PE_0; break; case '1': pin = PE_1; break; case '2': pin = PE_2; break; case '3': pin = PE_3; break;
				case '4': pin = PE_4; break; case '5': pin = PE_5; break; case '6': pin = PE_6; break; case '7': pin = PE_7; break;
				case '8': pin = PE_8; break; case '9': pin = PE_9; break; case 'A': pin = PE_A; break;
			}
			break;
		case 'F':
			switch(num){
				case '0': pin = PF_0; break; case '1': pin = PF_1; break; case '2': pin = PF_2; break; case '3': pin = PF_3; break;
				case '4': pin = PF_4; break; case '5': pin = PF_5; break;
			}
			break;
		case 'G':
			switch(num){
				case '0': pin = PG_0; break; case '1': pin = PG_1; break; case '2': pin = PG_2; break; case '3': pin = PG_3; break;
				case '4': pin = PG_4; break; case '5': pin = PG_5; break; case '6': pin = PG_6; break; case '7': pin = PG_7; break;
			}
			break;
		case 'H':
			switch(num){
				case '0': pin = PH_0; break; case '1': pin = PH_1; break; case '2': pin = PH_2; break; case '3': pin = PH_3; break;
				case '4': pin = PH_4; break; case '5': pin = PH_5; break; case '6': pin = PH_6; break; case '7': pin = PH_7; break;
			}
			break;
		case 'I':
			switch(num){
				case '0': pin = PI_0; break; case '1': pin = PI_1; break; case '2': pin = PI_2; break; case '3': pin = PI_3; break;
				case '4': pin = PI_4; break; case '5': pin = PI_5; break; case '6': pin = PI_6; break; case '7': pin = PI_7; break;
			}
			break;
		case 'J':
			switch(num){
				case '0': pin = PJ_0; break; case '1': pin = PJ_1; break; case '2': pin = PJ_2; break; case '3': pin = PJ_3; break;
				case '4': pin = PJ_4; break; case '5': pin = PJ_5; break; case '6': pin = PJ_6; break;
			}
			break;
		case 'K':
			switch(num){
				case '0': pin = PK_0; break; case '1': pin = PK_1; break; case '2': pin = PK_2; break; case '3': pin = PK_3; break;
				case '4': pin = PK_4; break; case '5': pin = PK_5; break; case '6': pin = PK_6; break;
			}
			break;
	}
	if(pin == NC){
		AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSG]: Invalid Pin Name");
		return;
	}
	// Remove debug info massage
	ConfigDebugInfo = 0;
	// Initial input control pin
	gpio_init(&gpio_test, pin);
	gpio_dir(&gpio_test, PIN_INPUT);     // Direction: Input
	gpio_mode(&gpio_test, PullUp);       // Pull-High
	val = gpio_read(&gpio_test);
	// Recover debug info massage
	ConfigDebugInfo = tConfigDebugInfo;
	AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSG] %c%c = %d", port, num, val);
}

void fATSC(void *arg)
{	
	AT_DBG_MSG(AT_FLAG_OTA, AT_DBG_ALWAYS, "[ATSC]: _AT_SYSTEM_CLEAR_OTA_SIGNATURE_");
	sys_clear_ota_signature();
}

void fATSR(void *arg)
{
	AT_DBG_MSG(AT_FLAG_OTA, AT_DBG_ALWAYS, "[ATSR]: _AT_SYSTEM_RECOVER_OTA_SIGNATURE_");
	sys_recover_ota_signature();
}

void fATSP(void *arg)
{
	int   argc           = 0;
	char *argv[MAX_ARGC] = {0};

	unsigned long timeout; // ms
	unsigned long time_begin, time_current;

	gpio_t gpiob_1;
	int val_old, val_new;

	int expected_zerocount, zerocount;
	int test_result;

	// parameter check
	AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: _AT_SYSTEM_POWER_PIN_TEST_");
	if(!arg) {
		AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: Usage: ATSP=gpiob1[timeout,zerocount]");
	} else {
		argc = parse_param(arg, argv);
		if (argc < 2) {
			AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: Usage: ATSP=gpiob1[timeout,zerocount]");
			return;
		}
	}

	if ( strcmp(argv[1], "gpiob1" ) == 0 ) {
		if (argc < 4) {
			AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: Usage: ATSP=gpiob1[timeout,zerocount]");
			return;
		}

		// init gpiob1 test
		test_result         = 0;
		timeout             = strtoul(argv[2], NULL, 10);
		expected_zerocount  = atoi(argv[3]);
		zerocount           = 0;
		val_old             = 1;

		sys_log_uart_off();

		gpio_init(&gpiob_1, PB_1);
		gpio_dir(&gpiob_1, PIN_INPUT);
		gpio_mode(&gpiob_1, PullDown);

		// gpiob1 test ++
		time_begin = time_current = xTaskGetTickCount();
		while (time_current < time_begin + timeout) {
			val_new = gpio_read(&gpiob_1);

			if (val_new != val_old && val_new == 0) {

				zerocount ++;
				if (zerocount == expected_zerocount) {
					test_result = 1;
					break;
				}
			}

			val_old = val_new;
			time_current = xTaskGetTickCount();
		}
		// gpio test --

		sys_log_uart_on();

		if (test_result == 1) {
			AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: success");
		} else {
			AT_DBG_MSG(AT_FLAG_GPIO, AT_DBG_ALWAYS, "[ATSP]: fail, it only got %d zeros", zerocount);
		}
	}
}
#endif

#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
void fATSL(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	uint32_t lock_id;

	AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL]: _AT_SYS_WAKELOCK_TEST_");

	if (!arg) {
		AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] Usage ATSL=[a/r/?][bitmask]");
		return;
	} else {
		argc = parse_param(arg, argv);
		if (argc < 2) {
			AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] Usage ATSL=[a/r/?][bitmask]");
			return;
		}
	}

	switch(argv[1][0]) {
		case 'a': // acquire
		{
			if (argc == 3) {
				lock_id = strtoul(argv[2], NULL, 16);
				acquire_wakelock(lock_id);
			}
			AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] wakelock:0x%08x", get_wakelock_status());			
			break;
		}

		case 'r': // release
		{
			if (argc == 3) {
				lock_id = strtoul(argv[2], NULL, 16);
				release_wakelock(lock_id);
			}
			AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] wakelock:0x%08x", get_wakelock_status());
			break;
		}

		case '?': // get status
			AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] wakelock:0x%08x", get_wakelock_status());
#if (configGENERATE_RUN_TIME_STATS == 1)
            get_wakelock_hold_stats((char *)cBuffer);
            AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "%s", cBuffer);
#endif
			break;

#if (configGENERATE_RUN_TIME_STATS == 1)
        case 'c': // clean wakelock info (for recalculate wakelock hold time)
            AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] clean wakelock stat");
            clean_wakelock_stat();
            break;
#endif
		default:
			AT_DBG_MSG(AT_FLAG_OS, AT_DBG_ALWAYS, "[ATSL] Usage ATSL=[a/r/?][bitmask]");
			break;
	}
}
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
void fATSS(void *arg)	// Show CPU stats
{
	AT_PRINTK("[ATSS]: _AT_SYSTEM_CPU_STATS_");
	vTaskGetRunTimeStats((char *)cBuffer);
	AT_PRINTK("%s", cBuffer);
}
#endif

void fATSs(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	AT_PRINTK("[ATS@]: _AT_SYSTEM_DBG_SETTING_");
	if(!arg){
		AT_PRINTK("[ATS@] Usage: ATS@=[LEVLE,FLAG]");
	}else{
		argc = parse_param(arg, argv);
		if(argc == 3){
			char *ptmp;
			gDbgLevel = atoi(argv[1]);
			gDbgFlag = strtoul(argv[2], &ptmp, 16);
		}
	}
	AT_PRINTK("[ATS@] level = %d, flag = 0x%08X", gDbgLevel, gDbgFlag);
}

void fATSc(void *arg)
{
	int argc = 0, config = 0;
	char *argv[MAX_ARGC] = {0};

	AT_PRINTK("[ATS!]: _AT_SYSTEM_CONFIG_SETTING_");
	if(!arg){
		AT_PRINTK("[ATS!] Usage: ATS!=[CONFIG(0,1,2),FLAG]");
	}else{
		argc = parse_param(arg, argv);
		if(argc == 3){
			char *ptmp;
			config = atoi(argv[1]);
			if(config == 0)
				ConfigDebugErr = strtoul(argv[2], &ptmp, 16);
			if(config == 1)
				ConfigDebugInfo = strtoul(argv[2], &ptmp, 16);
			if(config == 2)
				ConfigDebugWarn = strtoul(argv[2], &ptmp, 16);
		}
	}
	AT_PRINTK("[ATS!] ConfigDebugErr  = 0x%08X", ConfigDebugErr);
	AT_PRINTK("[ATS!] ConfigDebugInfo = 0x%08X", ConfigDebugInfo);
	AT_PRINTK("[ATS!] ConfigDebugWarn = 0x%08X", ConfigDebugWarn);
}

void fATSt(void *arg)
{
	AT_PRINTK("[ATS#]: _AT_SYSTEM_TEST_");
}

void fATSx(void *arg)
{
	AT_PRINTK("[ATS?]: _AT_SYSTEM_HELP_");
	AT_PRINTK("[ATS?]: COMPILE TIME: %s", RTL8195AFW_COMPILE_TIME);
}

log_item_t at_sys_items[] = {
	{"ATSD", fATSD,},	// Dump register
	{"ATSE", fATSE,},	// Edit register
#if SUPPORT_UART_YMODEM
	{"ATSY", fATSY,},	// uart ymodem upgrade
#endif
#if SUPPORT_MP_MODE
	{"ATSA", fATSA,},	// MP ADC test
	{"ATSG", fATSG,},	// MP GPIO test
	{"ATSC", fATSC,},	// Clear OTA signature
	{"ATSR", fATSR,},	// Recover OTA signature
	{"ATSP", fATSP,},	// MP Power related test
#endif
#if defined(configUSE_WAKELOCK_PMU) && (configUSE_WAKELOCK_PMU == 1)
	{"ATSL", fATSL,},	 // wakelock test
#endif
#if (configGENERATE_RUN_TIME_STATS == 1)
	{"ATSS", fATSS,},	// Show CPU stats
#endif
	{"ATS@", fATSs,},	// Debug message setting
	{"ATS!", fATSc,},	// Debug config setting
	{"ATS#", fATSt,},	// test command
	{"ATS?", fATSx,}	// Help
};

void at_sys_init(void)
{
	log_service_add_table(at_sys_items, sizeof(at_sys_items)/sizeof(at_sys_items[0]));
}

log_module_init(at_sys_init);
