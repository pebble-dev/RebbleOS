/*
 * stm32_rtc.c
 * Implementation of generic STM32 button HAL.
 * RebbleOS
 *
 * Barry Carter <barry.carter@gmail.com>
 * Refactored by Thomas England (crc-32) <thomas.c.england@gmail.com>
 *
 */

#if defined(STM32F4XX)
#    include "stm32f4xx.h"
#elif defined(STM32F2XX)
#    include "stm32f2xx.h"
#    include "stm32f2xx_rtc.h"
#    include "stm32f2xx_rcc.h"
#    include "stm32f2xx_exti.h"
#    include "stm32f2xx_pwr.h"
#    include "misc.h"
#else
#    error "I have no idea what kind of stm32 this is; sorry"
#endif
#include "stdio.h"
#include "string.h"
#include "stm32_power.h"
#include "stm32_rtc.h"
#include "log.h"
#include <stdlib.h>
#include <time.h>

// a buffer for the last captured time to avoid malloc
static struct tm time_now;

// for the interrupts
__IO uint32_t uwCaptureNumber = 0;
__IO uint32_t uwPeriodValue = 0;
uint16_t tmpCC4[2] = {0, 0};

void rtc_init(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // Configure the RTC clocks
    rtc_config();

    stm32_power_request(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);

    // Setup the wakeup interrupt for later on when we do power management
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStruct.EXTI_Line = EXTI_Line22;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // Enable the RTC Wakeup Interrupt
    NVIC_InitStruct.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // Configure the RTC WakeUp Clock source: CK_SPRE (1Hz)
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(0x0);

    // Enable the RTC Wakeup Interrupt
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);

    // Enable Wakeup Counter
    RTC_WakeUpCmd(ENABLE);

    stm32_power_release(STM32_POWER_APB2, RCC_APB2Periph_SYSCFG);
}
void rtc_config(void)
{
    RTC_InitTypeDef  RTC_InitStructure;

    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_PWR);

    PWR_BackupAccessCmd(ENABLE); // allow RTC access

#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
    RCC_LSICmd(ENABLE); // enable internal clock

    // wait for it to come up
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {
    }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    uint32_t uwSynchPrediv = 0xFF;
    uint32_t uwAsynchPrediv = 0x7F;


// external oscilator
#elif defined (RTC_CLOCK_SOURCE_LSE)
    RCC_LSEConfig(RCC_LSE_ON);

    // wait for LSE
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    uint32_t uwSynchPrediv = 0xFF;
    uint32_t uwAsynchPrediv = 0x7F;

#else
    #error Please select the RTC Clock source inside the main.c file
#endif

    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();

    // configure clock prescaler
    RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line17);

    stm32_power_release(STM32_POWER_APB1, RCC_APB1Periph_PWR);
}

struct tm *hw_get_time(void)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
    time_now.tm_year = RTC_DateStructure.RTC_Year + 2000 - 1900; // since year 1990
    time_now.tm_mon = RTC_DateStructure.RTC_Month - 1;
    time_now.tm_mday = RTC_DateStructure.RTC_Date;
    time_now.tm_hour = RTC_TimeStructure.RTC_Hours;
    time_now.tm_min = RTC_TimeStructure.RTC_Minutes;
    time_now.tm_sec = RTC_TimeStructure.RTC_Seconds;

    return &time_now;
}

void hw_set_alarm(struct tm alarm)
{
//     RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
//     RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x01;
//     RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x02;
//     RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x03;
//     RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x01;
//     RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
//     RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
//
//     RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
//
//     RTC_ITConfig(RTC_IT_ALRA, ENABLE);
//     RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
//     RTC_ClearFlag(RTC_FLAG_ALRAF);
}

void hw_set_date_time(struct tm date_time)
{
//     RTC_DateStructure.RTC_Year = 0x13;
//     RTC_DateStructure.RTC_Month = RTC_Month_January;
//     RTC_DateStructure.RTC_Date = 0x11;
//     RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Saturday;
//     RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
//
//     RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
//     RTC_TimeStructure.RTC_Hours   = 0x05;
//     RTC_TimeStructure.RTC_Minutes = 0x20;
//     RTC_TimeStructure.RTC_Seconds = 0x00;
//
//     RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
}


void RTC_WKUP_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_WUT);
//         DRV_LOG("RTC", APP_LOG_LEVEL_DEBUG, "RTC WAKE IRQ");
        EXTI_ClearITPendingBit(EXTI_Line22);
    }
}
