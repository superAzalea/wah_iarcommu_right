/**************************************************************************************************
Project: TTK_ACPR
Filename: timer.c
Description: time interruput functions for air compressor.
Complier: Keil uVision5.
***************************************************************************************************
Copyright (c) 2014 TTK.
All rights reserved.
***************************************************************************************************
DESCRIPTION:
Revision History:
Original Author:Azalea
**************************************************************************************************/
#ifndef __TIMER_C
#define __TIMER_C

#include "stm32f10x.h"
#include "define.h"
#include "SystemInit.h"
#include "Typedef.h"
#include "key.h"
#include "SysPr.h"
#include "Menu.h"
#include "usart.h"
#include "adc.h"
#include "DataHandle.h"
#include "main.h"
#include "pid.h"
#include "SystemWord.h"
#include "LCD_display.h"
#include "timer.h"
#include "buzz.h"
#include "Typedef.h"
//#include "bit_define.h"

extern unsigned char _manual_mode; //1���ֶ�ģʽ
//extern unsigned char input_door_switch;
extern unsigned char _key_pro_ok_2;
extern u8 _enable_display;
extern unsigned int main_cycle_1s_timer;
extern unsigned int spin_time_interval;

extern unsigned char flag_fault_display;
extern unsigned char _enable_commu_Inventer;
extern unsigned char usart3_flag_fault_clear;
extern unsigned int acce_time;//����ʱ��
extern unsigned int dece_time;//����ʱ��
extern unsigned int run_frequency;
extern unsigned char auto_mode_s;
extern unsigned char auto_mode_m;
extern unsigned char set_num;
extern unsigned char set_num_timer;
extern unsigned char commun_interval;//ͨѶ���
extern unsigned char _1s_timer	;
extern unsigned char flag_read_Inv;
extern unsigned char buzzer_timer;
extern u16 counter_break_usart;
extern unsigned char HMICommuINV_OK;
extern unsigned char dehydration_provide_timer;        //��ˮ��ʼǰ��׼��ʱ��

extern unsigned char flag_1s_wr;
extern unsigned char flag_wr_more;
#define USART1_IRQn		37
#define TIM3_IRQn		29

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//IRQͨ����ʹ��	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���		

}

void Timerx_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	TIM_TimeBaseStructure.TIM_Period = (arr-1); //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler = (psc-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update  |  //TIM �ж�Դ
		TIM_IT_Trigger,   //TIM �����ж�Դ 
		ENABLE  //ʹ��
		);
			
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����							 
}

void TIM3_IRQHandler(void)                 //TIM3 1ms�ж� exe14us
{
	static u16 counter_rdwr = 0;
	static u16 counter_5s = 0;
	static u16 counter_2s = 0;
	static u16 counter_500ms = 0;
	static u16 counter_holding = 0;
	static u16 counter_reset = 0;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		if(flag_InitFinish == 1)//��ʼ����ɺ󣬲ſ�ʼ��д����
		{
			counter_rdwr ++;
			if(counter_rdwr == 500)
			{		
				flag_500ms_rd = 1;
			}
			
                        
			if(counter_rdwr == 700)//д����elc�Ĵ���
			{
				flag_1s_wr = 1;

			}
			if(counter_rdwr == 1200)//д���elc�Ĵ���
			{	
				flag_wr_more = 1;
			}			
			if(counter_rdwr > 1800)
			{
				counter_rdwr = 0;
			}
		}
		else
		{
			counter_2s ++;
			if(counter_2s == 2000)
			{
				counter_2s = 0;
				flag_2s_WrSysPr = 1;		
			}
		}	

		if(counter_500ms++ >= 2000)
		{
			counter_500ms = 0;
			flag_500ms = 1;
		}	
		

		 if(buzzer_timer)
		 {
			 buzzer_timer--;
			 if(buzzer && buzzer_timer == 0)
			 {
				 buzzer = 0;      
			 }
		  }	
		KeyScan();//�������ɨ�� exe time 7us
//////////////////////////////////////////////////////////////////////////////////////////////////
//USART ʵʱͨѶ���ּ��
//////////////////////////////////////////////////////////////////////////////////////////////////	
		if(flag_InitFinish == 1)//��ʼ����ɺ󣬲Ž�������
		{
			if(modbus_check[1] != MBS_HoldRegs[1])//�������ź�
			{
				if(flag_2s_WrSysPr == 1)//1s����һ��������Ϣ
				{
					flag_2s_WrSysPr = 0;
					usart_WriteRegister(1,MBS_HoldRegs[1]);//���������ź�0x5a5a
//					flag_writing = 1;
					flag_holding = 1;
				}
			}
			else
			{
				flag_holding = 0;
				modbus_check[1] = 0;
			}
			//modbus_dataUnpack_host();

		}
		
		if(flag_holding == 1)
		{
			counter_holding ++;
		}
		else
		{
			counter_holding = 0;
		}
		if(counter_holding >= 5000)
		{
			//glFaultWord.bit.usart = 1; //ͨѶ����		
		}
		else
		{	
			//glFaultWord.bit.usart = 0;
		}
		
		 sub_DataHandle();
		
	}
			
	if(main_cycle_1s_timer)//1000 2s
	{
		main_cycle_1s_timer--;
		
		 if(main_cycle_1s_timer == 0)
		{
			_enable_display = 1;
			_1s_timer = 1;
			if(flag_fault_display == 0)
			{
				flag_fault_display = 1;
			}
			else
			{
				flag_fault_display = 0;
			}
			//lcd_reset = 1;
			main_cycle_1s_timer = 300;//500;
	//~~~~~~~~~~~~1s~~~~~~~~~~~~~~~~~
			if(_manual_mode)
			{ 
				if(++auto_mode_s > 59)
				{          
					if(auto_mode_m < 99)
					{
					   auto_mode_s = 0;          
					   auto_mode_m++;
					 } 
					 else
					 {
						auto_mode_s--;  
					 }
				} 
                                
                                
			}
		//	_enable_display = 1;
		//	_1s_timer = 1;
			if(set_num != 0)
			{
				set_num_timer--;
				if(set_num_timer == 0)
				{
					set_num = 0;
				}
			}
			if(commun_interval)
			{
				commun_interval--;      
			}
                        
                       
	   }
	}

}

#endif












