#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stdio.h"
#include "string.h"
#include "rtc.h"
#include "task.h"
#include "semphr.h"
void rtc_config(void);

void rtc_init(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    RTC_TimeTypeDef  RTC_TimeStructure;
    
    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
    {  
        rtc_config();
        //RTC_TimeShow();
        //RTC_AlarmShow();
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
        printf("%02d:%02d:%02d\n",RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    }
    else
    {       
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        PWR_BackupAccessCmd(ENABLE); // allow RTC access
        RTC_WaitForSynchro();

        // clear the alarm and int
        RTC_ClearFlag(RTC_FLAG_ALRAF);
        EXTI_ClearITPendingBit(EXTI_Line17);

        //RTC_TimeShow();
        //RTC_AlarmShow();
    }

    // setup alarm
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStruct.EXTI_Line = EXTI_Line17;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    NVIC_InitStruct.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void rtc_config(void)
{
    RTC_DateTypeDef RTC_DateStructure;
    RTC_TimeTypeDef  RTC_TimeStructure;
    RTC_InitTypeDef  RTC_InitStructure;
    RTC_AlarmTypeDef RTC_AlarmStructure;
  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

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
  
  // Set the alarm 01h:02min:03s
  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x01;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x02;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x03;
  RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x01;
  RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
  RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
  
  RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
  
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
  RTC_ClearFlag(RTC_FLAG_ALRAF);
  
  /* Set the date: Friday January 11th 2013 */
  /*RTC_DateStructure.RTC_Year = 0x13;
  RTC_DateStructure.RTC_Month = RTC_Month_January;
  RTC_DateStructure.RTC_Date = 0x11;
  RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Saturday;
  RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
  */
  /* Set the time to 05h 20mn 00s AM */
  /*RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
  RTC_TimeStructure.RTC_Hours   = 0x05;
  RTC_TimeStructure.RTC_Minutes = 0x20;
  RTC_TimeStructure.RTC_Seconds = 0x00; 
  
  RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);   
  */
  
  // RTC Confirmed
  RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}
