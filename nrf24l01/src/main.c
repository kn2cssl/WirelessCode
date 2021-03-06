#include <asf.h>
#include <stdio.h>
#define F_CPU 32000000UL
#include <util/delay.h>

#include "initialize.h"
#include "nrf24l01_L.h"
#include "nrf24l01_R.h"
#include "Menu.h"
#include "lcd.h"
#include "transmitter.h"

char Buf_Tx_R[Max_Robot][_Buffer_Size];
char Buf_Rx_R[Max_Robot][_Buffer_Size];
//char Buf_Tx_Comp[122] = {0xa5,0x5a}; //2+10*12
int tmprid;
int RobotID=0;

//char Buf_Tx_L[_Buffer_Size] ;

char Address[_Address_Width];	
char str[40];
uint8_t count;
uint8_t flg,wireless_flg=0,Address_flg=0;
float P_temp,I_temp,D_temp,P,I,D,a=0,ki=0.65,kp=0.15,kd=0.05,M1,M1_temp;//ki=1.34,kp=1,kd=0.02,;

uint16_t pck_timeout[Max_Robot];	

int main (void)
{
	Address[0] = 0x00;
	Address[1] = 0x22;
	Address[2] = 0x33;
	Address[3] = 0x44;
	Address[4] = 0x55;
	
    En_RC32M();
    PORT_init();
	//LCDInit();
    TimerD0_init();
    PMIC_CTRL |=PMIC_LOLVLEN_bm|PMIC_MEDLVLEN_bm;

    wdt_enable();

    USART_R_init();
    USART_L_init();
    NRF24L01_L_CE_LOW;       //disable transceiver modes
    NRF24L01_R_CE_LOW;
    ///////////////////////////////////////////////////////////////////////////////////////////spi se

    spi_xmega_set_baud_div(&NRF24L01_L_SPI,8000000UL,F_CPU);
    spi_enable_master_mode(&NRF24L01_L_SPI);
    spi_enable(&NRF24L01_L_SPI);

    //spi_xmega_set_baud_div(&NRF24L01_R_SPI,8000000UL,F_CPU);
    //spi_enable_master_mode(&NRF24L01_R_SPI);
    //spi_enable(&NRF24L01_R_SPI);
		//LED_White_R_PORT.OUTSET = LED_White_R_PIN_bm;
		//LED_Green_R_PORT.OUTCLR = LED_Green_R_PIN_bm;
		//LED_White_L_PORT.OUTSET = LED_White_L_PIN_bm;
		//LED_Green_L_PORT.OUTCLR = LED_Green_L_PIN_bm;
    sei();
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    _delay_us(10);
    _delay_ms(100);      //power on reset delay needs 100ms
    NRF24L01_L_Clear_Interrupts();
    //NRF24L01_R_Clear_Interrupts();
    NRF24L01_L_Flush_TX();
    //NRF24L01_R_Flush_TX();
    NRF24L01_L_Flush_RX();
    //NRF24L01_R_Flush_RX();

    NRF24L01_L_CE_LOW;
    NRF24L01_L_Init(_TX_MODE, _CH_L, _2Mbps, Address, _Address_Width, _Buffer_Size, RF_PWR_MAX);
    NRF24L01_L_CE_HIGH;


    //NRF24L01_R_CE_LOW;
    //NRF24L01_R_Init(_TX_MODE, _CH_R, _2Mbps, Address, _Address_Width, _Buffer_Size, RF_PWR_MAX);
    //NRF24L01_R_CE_HIGH;

    _delay_us(130);

    for (uint8_t i=0;i<Max_Robot;i++)
    {
        Robot_D_tmp[i].RID=12;
    }
	
	
	
	
    while (1)
    {		
        for(uint8_t i=0;i<12;i++)
        {
	        Buf_Tx_R[i][11] = Menu_Num;
            Buf_Tx_R[i][12] = (int)(kp*100);
            Buf_Tx_R[i][13] = (int)(ki*100);
            Buf_Tx_R[i][14] = (int)(kd*100);
        }

        if(Menu_PORT.IN & Menu_Side_Select_PIN_bm)
        {
	        count = sprintf(str,"%d,%d\r",
	        ((Buf_Rx_R[0][1+(Menu_Num*2)]<<8)&0x0ff00)|(Buf_Rx_R[0][0+(Menu_Num*2)]&0x00ff),
	        ((Buf_Rx_R[0][11+(Menu_Num*2)]<<8)&0x0ff00)|(Buf_Rx_R[0][10+(Menu_Num*2)]&0x00ff));
	        
	        for (uint8_t i=0;i<count;i++)
	        usart_putchar(&USARTE0,str[i]);
        }
		
        if(Menu_PORT.IN & Menu_Set_PIN_bm)
        {
            count = sprintf(str,"kp: %d      ki: %d      kd: %d\r",(int)(kp*100),(int)(ki*100),(int)(kd*100));

            for (uint8_t i=0;i<count;i++)
                usart_putchar(&USARTE0,str[i]);
        }

		
		for(Address_flg=0 ; Address_flg<=9 ; Address_flg++ )
		{
			Address[0]=Address_flg;	
			//LED_Green_L_PORT.OUTTGL = LED_Green_L_PIN_bm;
			NRF24L01_L_WriteRegBuf(W_REGISTER | (RX_ADDR_P0 ), Address, _Address_Width);
			NRF24L01_L_Set_TX_Address(Address, _Address_Width);
			NRF24L01_L_Write_TX_Buf(Buf_Tx_R[Address_flg], _Buffer_Size);
			NRF24L01_L_RF_TX();
			_delay_ms(3);	 
		}	
		
					   
    }
}

		
ISR(TCD0_OVF_vect)
{
    for (uint8_t i=0;i<Max_Robot;i++)
    {
	    pck_timeout[i]++;
	    if (pck_timeout[i]>=300)
	    {
		    if(pck_timeout[i]<=600)
		    
		    {
			    Buf_Tx_R[i][1] = 0;
			    Buf_Tx_R[i][2] = 1;
			    Buf_Tx_R[i][3] = 0;
			    Buf_Tx_R[i][4] = 1;
			    Buf_Tx_R[i][5] = 0;
			    Buf_Tx_R[i][6] = 1;
			    Buf_Tx_R[i][7] = 0;
			    Buf_Tx_R[i][8] = 1;
			    Buf_Tx_R[i][9] = 0;
			    Buf_Tx_R[i][10] = 0;
		    }
		    else
		    {
			    Buf_Tx_R[i][1] = 0;
			    Buf_Tx_R[i][2] = 0;
			    Buf_Tx_R[i][3] = 0;
			    Buf_Tx_R[i][4] = 0;
			    Buf_Tx_R[i][5] = 0;
			    Buf_Tx_R[i][6] = 0;
			    Buf_Tx_R[i][7] = 0;
			    Buf_Tx_R[i][8] = 0;
			    Buf_Tx_R[i][9] = 0;
			    Buf_Tx_R[i][10] = 0;
		    }
	    }
    }
    

    while(Menu_PORT.IN & Menu_Side_Switch_PIN_bm);
    wdt_reset();
}

ISR(PRX_R)
{
	//wdt_reset();
    //uint8_t status_R = NRF24L01_R_WriteReg(W_REGISTER|STATUSe,_TX_DS|_MAX_RT|_RX_DR);
    //if((status_R & _RX_DR) == _RX_DR)
    //{
    //}
    //if((status_R&_TX_DS) == _TX_DS)
    //{
		//LED_White_R_PORT.OUTTGL = LED_White_R_PIN_bm;
			//
    //}
    //if ((status_R&_MAX_RT) == _MAX_RT)
    //{
        //NRF24L01_R_Flush_TX();
		//LED_Green_R_PORT.OUTTGL = LED_Green_R_PIN_bm;
    //}
}

ISR(PTX_L)
{
	uint8_t status_L = NRF24L01_L_WriteReg(W_REGISTER | STATUSe, _TX_DS|_MAX_RT|_RX_DR);
	if((status_L & _RX_DR) == _RX_DR)
	{
	}
	if((status_L&_TX_DS) == _TX_DS)
	{
		LED_White_L_PORT.OUTTGL = LED_White_L_PIN_bm;
			
    }
    if ((status_L&_MAX_RT) == _MAX_RT)
    {
        LED_Green_L_PORT.OUTTGL = LED_Green_L_PIN_bm;	
        NRF24L01_L_Flush_TX();
    }
}

ISR(USART_R_RXC_vect) 
{
    GetNewData(USARTC0_DATA);
}

ISR(USART_R_DRE_vect) //Wireless_R_USART 
{

}

ISR(USART_L_RXC_vect)
{
    char data;
    data=USARTE0_DATA;


    switch (data)
    {
    case 'p':
        kp=kp+0.01;
        count = sprintf(str,"kp: %d\r",(int)(kp*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    case 'o':
        ki=ki+0.05;
        count = sprintf(str,"ki: %d\r",(int)(ki*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    case 'i':
        kd=kd+0.01;
        count = sprintf(str,"kd: %d\r",(int)(kd*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    case 'l':
        kp=kp-0.01;
        count = sprintf(str,"kp: %d\r",(int)(kp*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    case 'k':
        ki=ki-0.05;
        count = sprintf(str,"ki: %d\r",(int)(ki*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    case 'j':
        kd=kd-0.01;
        count = sprintf(str,"kd: %d\r",(int)(kd*100));
        for (uint8_t i=0;i<count;i++)
            usart_putchar(&USARTE0,str[i]);
        break;
    };

}
ISR(USART_L_DRE_vect) //Wireless_R_USART
{

}
