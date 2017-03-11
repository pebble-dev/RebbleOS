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

// a buffer for the last captured time to avoid malloc
static struct tm time_now;

// for the interrupts
__IO uint32_t uwCaptureNumber = 0; 
__IO uint32_t uwPeriodValue = 0;
uint16_t tmpCC4[2] = {0, 0};
static uint32_t GetLSIFrequency(void);

void rtc_init(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    rtc_config();

    // setup calendar
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStruct.EXTI_Line = EXTI_Line22;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);



    /* Enable the RTC Wakeup Interrupt */
    NVIC_InitStruct.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(0x0);

    /* Enable the RTC Wakeup Interrupt */
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);
    
    /* Enable Wakeup Counter */
    RTC_WakeUpCmd(ENABLE);
}

void rtc_config(void)
{
    //RTC_DateTypeDef RTC_DateStructure;
    //RTC_TimeTypeDef  RTC_TimeStructure;
    RTC_InitTypeDef  RTC_InitStructure;
    //RTC_AlarmTypeDef RTC_AlarmStructure;
    __IO uint32_t uwLsiFreq = 0;

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
    
    // RTC Confirmed
//     RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
    
    /* Get the LSI frequency:  TIM5 is used to measure the LSI frequency */
    
    // TODO this doesn't seem to work
//     uwLsiFreq = GetLSIFrequency();

    /* Calendar Configuration */
    /* ck_spre(1Hz) = RTCCLK(LSI) /(AsynchPrediv + 1)*(SynchPrediv + 1)*/
//     RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
//     RTC_InitStructure.RTC_SynchPrediv	= (uwLsiFreq/128) - 1;
//     RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
//     RTC_Init(&RTC_InitStructure);

}

void hw_get_time_str(char *buf)
{
    RTC_TimeTypeDef  RTC_TimeStructure;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    sprintf(buf, "%02d:%02d:%02d\n",RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
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
    printf("TICK\n");
    if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        printf("TICK\n");
        EXTI_ClearITPendingBit(EXTI_Line22);
    } 
}


/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency
  */
static uint32_t GetLSIFrequency(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  RCC_ClocksTypeDef  RCC_ClockFreq;

  /* Enable the LSI oscillator ************************************************/
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}

  /* TIM5 configuration *******************************************************/ 
  /* Enable TIM5 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* Connect internally the TIM5_CH4 Input Capture to the LSI clock output */
  TIM_RemapConfig(TIM5, TIM5_LSI);

  /* Configure TIM5 presclaer */
  TIM_PrescalerConfig(TIM5, 0, TIM_PSCReloadMode_Immediate);
  
  /* TIM5 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM5 CH4
     The Rising edge is used as active edge,
     The TIM5 CCR4 is used to compute the frequency value 
  ------------------------------------------------------------ */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM5, &TIM_ICInitStructure);
  
  /* Enable TIM5 Interrupt channel */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable TIM5 counter */
  TIM_Cmd(TIM5, ENABLE);

  /* Reset the flags */
  TIM5->SR = 0;
    
  /* Enable the CC4 Interrupt Request */  
  TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);

  /* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in 
    stm32f4xx_it.c file) ******************************************************/
  while(uwCaptureNumber != 2)
  {
  }
  /* Deinitialize the TIM5 peripheral registers to their default reset values */
  TIM_DeInit(TIM5);

  /* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
  /* Get SYSCLK, HCLK and PCLKx frequency */
  RCC_GetClocksFreq(&RCC_ClockFreq);

  /* Get PCLK1 prescaler */
  if ((RCC->CFGR & RCC_CFGR_PPRE1) == 0)
  { 
    /* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
    return ((RCC_ClockFreq.PCLK1_Frequency / uwPeriodValue) * 8);
  }
  else
  { /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
    return (((2 * RCC_ClockFreq.PCLK1_Frequency) / uwPeriodValue) * 8) ;
  }
}



void TIM5_IRQHandler(void)
{
    printf("TICK\n");
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {    
    /* Get the Input Capture value */
    tmpCC4[uwCaptureNumber++] = TIM_GetCapture4(TIM5);
   
    /* Clear CC4 Interrupt pending bit */
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    if (uwCaptureNumber >= 2)
    {
      /* Compute the period length */
      uwPeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
  }
}
