/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stm32f4xx.h"
#include "stm32f4xx_rtc.h"
#include "stdio.h"
#include "string.h"
#include "snowy_rtc.h"
#include <stdlib.h>
#include <time.h>

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
    //RTC_DateTypeDef RTC_DateStructure;
    //RTC_TimeTypeDef  RTC_TimeStructure;
    RTC_InitTypeDef  RTC_InitStructure;
    //RTC_AlarmTypeDef RTC_AlarmStructure;
  
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
//   RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
//   RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x01;
//   RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x02;
//   RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x03;
//   RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x01;
//   RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
//   RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
//   
//   RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
//   
//   RTC_ITConfig(RTC_IT_ALRA, ENABLE);
//   RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
//   RTC_ClearFlag(RTC_FLAG_ALRAF);
  
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

void hw_get_time_str(char *buf)
{
    RTC_TimeTypeDef  RTC_TimeStructure;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    sprintf(buf, "%02d:%02d:%02d\n",RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
}

// void hw_get_time(time_t *time)
// {
//     RTC_TimeTypeDef RTC_TimeStructure;
//     RTC_DateTypeDef RTC_DateStructure;
//     struct tm timetmp;
// //    time = (time_t *)args;
// 
//     RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
//     RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
//     timetmp.tm_year = RTC_DateStructure.RTC_Year + 2000 - 1900; // since year 1990
//     timetmp.tm_mon = RTC_DateStructure.RTC_Month - 1;
//     timetmp.tm_mday = RTC_DateStructure.RTC_Date;
//     timetmp.tm_hour = RTC_TimeStructure.RTC_Hours;
//     timetmp.tm_min = RTC_TimeStructure.RTC_Minutes;
//     timetmp.tm_sec = RTC_TimeStructure.RTC_Seconds;
// 
//     time = mktime(&timetmp);
//     
//     return;
// }
