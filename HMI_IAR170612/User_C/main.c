/**************************************************************************************************
Project: TTK_ACPR
Filename: main.c
Description: time interruput functions for air compressor.
Complier: Keil uVision5.
***************************************************************************************************
Copyright (c) 2014 TTK.
All rights reserved.
***************************************************************************************************
DESCRIPTION:
Revision History:
Original Author:JesonsSun
First edition Time:20140601
Last Review Time:20140701 11:11;V1.0
**************************************************************************************************/
#ifndef __MAIN_C
#define __MAIN_C

#include "stm32f10x.h"
#include "string.h"
#include "define.h"
#include "delay.h"
#include "Menu.h"
#include "IIC.h"
#include "24CXX.h"
#include "timer.h"
#include "LCD_display.h"
#include "SystemInit.h"
#include "usart.h"
#include "usart3.h"
#include "crc.h"
#include "SysPr.h"
#include "rtc.h"
#include "Interrupt_Init.h"
#include "key.h"
#include "ziku.h"
#include "main.h"
#include "led.h"
#include "buzz.h"
#include "stdio.h"
#include "Typedef.h"
#include "struct_comm.h"

//#include "struct_comm.h"
//extern u16 modbus_check[MBS_BufSize];
//extern u8 fact_temp;//实际温度
//extern u8 fact_liquid_height;//实际液位
//extern volatile FUNC_CODE_I  Code_I;
u8 flag_err_handshake = 0; 
void ini_data(void)
{
	main_cycle_1s_timer =300;// 500;
        //flag_1s_wr = 0;
        //flag_wr_more = 0;
	flag_fault_display = 1;
	_if_save_data = 0;
	LED_RUN = 1;
        spin_time_interval = 0;
	LED_STOP = 0;
	back_light_timer = 30;
	_set_mode_symbol = 0;//进入设定模式标志
	auto_run_mode = 1;//为标准运行模式  
	_key_pro_ok_2 = 1;
	_key_long = 0;
        run_frequency = 0;//运行频率
        acce_time = 0;//加数时间
        dece_time = 0;//减速时间

	//fact_temp = 12;
	//fact_liquid_height = 10;
//	_enable_dece_time = 0;azalea__
}

int main(void)
{
	SystemInit();			//系统时钟等初始化
	delay_init(72);	    	//延时初始化 72M主频                                                                                                            
	GPIOInit();  	//IO初始化
	init_LCD();
	init_sap1024();
	//MenuIndex = 1000;//菜单标签号
	lcd_wrstring(0,0,disresetmenu);
	AT24CXX_Init();			//初始化IIC接口
	SysPrInit();			//系统参数初始化在IIC初始化之后
	usart_config(9600);		//串口初始化波特率为9600，停止位1

	sendtobuff(0,0,disresetmenu);			//菜单的静态内容
	lcd_wrstring(0,0,DisBuff);
	Timerx_Init(10,7200);//10Khz的计数频率，计数到10为1ms
	NVIC_Configuration();	

	memset(sendBuf,0,sizeof(sendBuf));
	memset(receBuf,0,sizeof(receBuf));	
	
	//把需要下发的数据填满到：MBS_HoldRegs[] 	
	MBS_HoldRegs[1] = 0x5a5a;//握手信号地址@

	flag_writing_0x06 = 1;

	while(flag_writing_0x06 == 1)
	{
		if(flag_2s_WrSysPr == 1)
		{
		       counter_wr_handshake ++;
                       flag_2s_WrSysPr = 0;
			usart_WriteRegister(1,MBS_HoldRegs[1]);//发送握手信号
			delay_ms(1000);
		}
		if(counter_wr_handshake >= 200)
		{
			counter_wr_handshake = 0;
			flag_err_handshake = 1;
			break;
		}
		modbus_dataUnpack_host();
	}
	if(flag_err_handshake == 1)
	{
		handshake_pro();
	}

	ini_data();
	flag_InitFinish = 1;//置一个初始化完成标志位
	while(1)
	{
             Lcd_KEYPro();		
             LCDDisplay();
           //  sub_buzz();
            // fault_dis_pro();   //故障扫描和处理
             Auto_WashPro();		 //自动洗衣程序
             if(flag_InitFinish == 1)//初始化完成后，才开始读写串口
             {
                 commu_pro();                 
             }
             sub_InputStates();  
        }
}

void commu_pro(void)
{
    if(flag_InitFinish == 1)
    {  	
//        if(flag_500ms_rd == 1) 
//        {
//            flag_500ms_rd = 0;
//            modbus_dataUnpack_host();
//            if((flag_writing_0x06 == 0)&&(flag_writing_0x10 == 0))
//            {
//                    
//                if(flag_reading_0x03 == 0)
//                {				
//                    if(readINV_handshake)
//                    {	
//                        usart_ReadRegister(58,6);
//                        flag_reading_0x03 = 1;
//                    }
//                    else
//                    {
//                        usart_ReadRegister(55,8);
//                        flag_reading_0x03 = 1;
//                    }
//                        
//                }
//                else
//                {
//                    if(readINV_handshake)
//                    {
//                        if(flag_rw_elc_err == 1)
//                        {
//                            usart_ReadRegister(58,6);
//                            counter_rd ++;
//                        }
//                    }
//                    else
//                    {
//                        if(flag_rw_elc_err == 1)
//                        {
//                            usart_ReadRegister(55,8);
//                            counter_rd ++;
//                        }
//                    }	
//                }
//                
//                if(counter_rd >= 5)
//                {
//                        flag_error_usart = 1;
//                        counter_rd =0;
//                }
//            }
//        }

        if(flag_1s_wr == 1)
        {
            flag_1s_wr = 0;
            modbus_dataUnpack_host();
            if((flag_reading_0x03 == 0) && (flag_writing_0x10 == 0))
            {
                if(usart3_flag_fault_clear == 1 )
                {
                    if(flag_writing_0x06 == 0)
                    {
                        flag_writing_0x06 = 1;
                        usart_WriteRegister(6,7);
                    }
                    else
                    {
                        if(flag_rw_elc_err == 1 )
                        {
                            usart_WriteRegister(6,7);
                            counter_wr++;
                        }
                        
                        if(counter_wr >= 5)
                        {
                            flag_error_usart = 1;
                            counter_wr =0;
                        }
                    }
                }
                else
                {
                    putCommSend();
                    if(CommSend_All_old !=structCommSend.all  )
                    {
                        if(flag_writing_0x06 == 0)
                        {
                            flag_writing_0x06 = 1;
                            usart_WriteRegister(5,structCommSend.all);
                        }
                        else
                        {
                            if(flag_rw_elc_err == 1 )
                            {
                                usart_WriteRegister(5,structCommSend.all);
                                counter_wr++;
                            }
                            if(counter_wr >= 5)
                            {
                                flag_error_usart = 1;
                                counter_wr =0;
                            }
                        }
                                    
                    }
                }	
            }
        }				
//				
        if(flag_wr_more == 1)
        {
            flag_wr_more = 0;
            modbus_dataUnpack_host();
            if((flag_reading_0x03 == 0)&&(flag_writing_0x06 == 0))
            {
                if(_enable_commu_Inventer == 1)
                {
                    //_enable_commu_Inventer = 0;
                    if(flag_writing_0x10 == 0)// 01 10 00 02 00 03 06 00 22 01 33 0F A0 4B 36
                    {
                        flag_writing_0x10 = 1;
                        MBS_HoldRegs[2] = acce_time;
                        MBS_HoldRegs[3] = dece_time;
                        MBS_HoldRegs[4] = (run_frequency * 100)/ 200 ;
                        usart_WriteMultipleRegister(2,3);	
                    }
                    else
                    {
                        if(flag_rw_elc_err == 1)
                        {
                            MBS_HoldRegs[2] = acce_time;
                            MBS_HoldRegs[3] = dece_time;
                            MBS_HoldRegs[4] = (run_frequency* 100 )/ 200 ;
                            usart_WriteMultipleRegister(2,3);
                            counter_wr10 ++;

                        }
                    }
                    if(counter_wr10 >= 5)
                    {
                        flag_error_usart = 1;
                        counter_wr10 = 0;
                    }
                
                }		
            }
        }			
                    
    }		

}

#endif
