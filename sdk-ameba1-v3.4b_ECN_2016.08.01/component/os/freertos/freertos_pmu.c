#include "FreeRTOS.h"

#include "freertos_pmu.h"

#include <platform_opts.h>

#include "platform_autoconf.h"
#include "sys_api.h"
#include "sleep_ex_api.h"
#include "gpio_api.h"

#define FREERTOS_PMU_DISABLE_LOGUART_IN_TICKLESS (1)

static uint32_t wakelock     = DEFAULT_WAKELOCK;
static uint32_t wakeup_event = DEFAULT_WAKEUP_EVENT;

void (*pre_sleep_callback)(unsigned int expected_idle_time) = NULL;
void (*post_sleep_callback)(unsigned int expected_idle_time) = NULL;

#if (configGENERATE_RUN_TIME_STATS == 1)
static u8 last_wakelock_state[32] = {
    DEFAULT_WAKELOCK & 0x01, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
static u32 last_acquire_wakelock_time[32] = {0};
static u32 hold_wakelock_time[32] = {0};
static u32 base_sys_time = 0;
#endif

/* ++++++++ FreeRTOS macro implementation ++++++++ */

/*
 *  It is called in idle task.
 *
 *  @return  true  : System is ready to check conditions that if it can enter sleep.
 *           false : System keep awake.
 **/
int freertos_ready_to_sleep() {
    return wakelock == 0;
}

/*
 *  It is called when freertos is going to sleep.
 *  At this moment, all sleep conditons are satisfied. All freertos' sleep pre-processing are done.
 *
 *  @param  expected_idle_time : The time that FreeRTOS expect to sleep.
 *                               If we set this value to 0 then FreeRTOS will do nothing in its sleep function.
 **/
void freertos_pre_sleep_processing(unsigned int *expected_idle_time) {

#ifdef CONFIG_SOC_PS_MODULE

    uint32_t stime;
    uint32_t tick_before_sleep;
    uint32_t tick_after_sleep;
    uint32_t tick_passed;

    /* To disable freertos sleep function and use our sleep function, 
     * we can set original expected idle time to 0. */
    stime = *expected_idle_time;
    *expected_idle_time = 0;

    // handle user level pre sleep processing
    if (pre_sleep_callback != NULL) {
        pre_sleep_callback(stime);
    }

    // Store gtimer timestamp before sleep
    tick_before_sleep = us_ticker_read();

#if (FREERTOS_PMU_DISABLE_LOGUART_IN_TICKLESS)
    // config gpio on log uart tx for pull ctrl
    HAL_GPIO_PIN gpio_log_uart_tx;
    gpio_log_uart_tx.pin_name = gpio_set(PB_0);
    gpio_log_uart_tx.pin_mode = DOUT_PUSH_PULL;
    HAL_GPIO_Init(&gpio_log_uart_tx);
    GpioFunctionChk(PB_0, ENABLE);

    sys_log_uart_off();
    HAL_GPIO_WritePin(&gpio_log_uart_tx, 1); // pull up log uart tx to avoid power lekage
#endif

    // sleep
#if defined(FREERTOS_PMU_TICKLESS_PLL_RESERVED) && (FREERTOS_PMU_TICKLESS_PLL_RESERVED==1)
    sleep_ex_selective(wakeup_event, stime, 1, 1);
#else
    sleep_ex_selective(wakeup_event, stime, 0, 1);
#endif

#if (FREERTOS_PMU_DISABLE_LOGUART_IN_TICKLESS)
    sys_log_uart_off();
    sys_log_uart_on();
#endif

    // update kernel tick by calculating passed tick from gtimer
    portENTER_CRITICAL();
    {
        // get current gtimer timestamp
        tick_after_sleep = us_ticker_read();

        // calculated passed time
        if (tick_after_sleep > tick_before_sleep) {
            tick_passed = tick_after_sleep - tick_before_sleep;
        } else {
            // overflow
            tick_passed = (0xffffffff - tick_before_sleep) + tick_after_sleep;
        }

        if (tick_passed > stime * 1000) {
            tick_passed = stime * 1000;
        }

        // update kernel tick
        vTaskStepTick( tick_passed/1000 );
    }
    portEXIT_CRITICAL();

    // handle user level post sleep processing
    if (post_sleep_callback != NULL) {
        post_sleep_callback(stime);
    }

#else
    // If PS is not enabled, then use freertos sleep function
#endif
}

/* -------- FreeRTOS macro implementation -------- */

void acquire_wakelock(uint32_t lock_id) {

    wakelock |= lock_id;

#if (configGENERATE_RUN_TIME_STATS == 1)
    u32 i;
    u32 current_timestamp = osKernelSysTick();
    for (i=0; i<32; i++) {
        if ( (1<<i & lock_id) && (last_wakelock_state[i] == 0) ) {
            last_acquire_wakelock_time[i] = current_timestamp;
            last_wakelock_state[i] = 1;            
        }
    }
#endif

}

void release_wakelock(uint32_t lock_id) {
    wakelock &= ~lock_id;

#if (configGENERATE_RUN_TIME_STATS == 1)
    u32 i;
    u32 current_timestamp = osKernelSysTick();
    for (i=0; i<32; i++) {
        if ( (1<<i & lock_id) && (last_wakelock_state[i] == 1) ) {
            hold_wakelock_time[i] += current_timestamp - last_acquire_wakelock_time[i];
            last_wakelock_state[i] = 0;
        }
    }
#endif

}

uint32_t get_wakelock_status() {
    return wakelock;
}

#if (configGENERATE_RUN_TIME_STATS == 1)
void get_wakelock_hold_stats( char *pcWriteBuffer ) {
    u32 i;
    u32 current_timestamp = osKernelSysTick();

    *pcWriteBuffer = 0x00;

    // print header
    sprintf(pcWriteBuffer, "wakelock_id\tholdtime\r\n");
    pcWriteBuffer += strlen( pcWriteBuffer );

    for (i=0; i<32; i++) {
        if (last_wakelock_state[i] == 1) {
            sprintf(pcWriteBuffer, "%x\t\t%d\r\n", i, hold_wakelock_time[i] + (current_timestamp - last_acquire_wakelock_time[i]));
        } else {
            if (hold_wakelock_time[i] > 0) {
                sprintf(pcWriteBuffer, "%x\t\t%d\r\n", i, hold_wakelock_time[i]);
            }
        }
        pcWriteBuffer += strlen( pcWriteBuffer );
    }
    sprintf(pcWriteBuffer, "time passed: %d\r\n", current_timestamp - base_sys_time);
}

void clean_wakelock_stat() {
    u32 i;
    base_sys_time = osKernelSysTick();
    for (i=0; i<32; i++) {
        hold_wakelock_time[i] = 0;
        if (last_wakelock_state[i] == 1) {
            last_acquire_wakelock_time[i] = base_sys_time;
        }
    }
}
#endif

void add_wakeup_event(uint32_t event) {
    wakeup_event |= event;
}

void del_wakeup_event(uint32_t event) {
    wakeup_event &= ~event;
    // To fulfill tickless design, system timer is required to be wakeup event
    wakeup_event |= SLEEP_WAKEUP_BY_STIMER;
}

void register_pre_sleep_callback( void (*pre_sleep_cb)(unsigned int) ) {
    pre_sleep_callback = pre_sleep_cb;
}

void register_post_sleep_callback( void (*post_sleep_cb)(unsigned int) ) {
    post_sleep_callback = post_sleep_cb;
}
