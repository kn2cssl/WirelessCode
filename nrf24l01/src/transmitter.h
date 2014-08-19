/*
 * transmitter.h
 *
 * Created: 02/27/2012 05:52:57 ب.ظ
 *  Author: Milad
 
 
 
 
 */ 

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_
#include <lcd.h>
#include <stdio.h>

#define Header_Lenght 4
#define Data_Lenght 11
#define Max_Robot 12
#define Max_SendPacket_Lenght 10 //2*4+1+1
#define START_BYTE0 0xA5
#define START_BYTE1 0x5A
#define STOP_BYTE 0x80
uint8_t PCK_Num_R=0,PCK_Num_L=0;
uint8_t data_write_index=0,data_read_index=0,Packet_Next_Start=0;
uint16_t PCK_Speed_L,PCK_Reset_L=0,PCK_Speed_R=0,PCK_Reset_R=0;
extern char Buf_Tx_R[Max_Robot][_Buffer_Size],Buf_Tx_L[Max_Robot][_Buffer_Size];
extern uint16_t pck_timeout_L[Max_Robot],pck_timeout_R[Max_Robot];
struct PCK_Header
{
	uint8_t NOB;  //Counter for Packet Bytes
	uint8_t SIB;
	uint8_t CHK;
};

union Motor_Speed
{
	char Bytes[2];
	int16_t Speed;
};

struct Robot_Data
{
	uint8_t RID;
	union Motor_Speed M0;
	union Motor_Speed M1;
	union Motor_Speed M2;
	union Motor_Speed M3;
	uint8_t KCK;
	uint8_t CHP;
	uint8_t ASK;
	uint8_t P;
	uint8_t I;
	uint8_t D;
	
};

struct PCK_Send_Header
{
	uint8_t SIB;
	uint8_t CHK;
	uint8_t RID;	
};


struct Robot_Send_Data
{
	uint8_t PTP;
	uint8_t EN1;
	uint8_t EN2;
	uint8_t EN3;
	uint8_t EN4;
};

struct PCK_Header PCK_H_R;
struct Robot_Data Robot_D_R[Max_Robot], Robot_D_tmp_R[Max_Robot];

struct PCK_Send_Header PCK_S_H_R;
struct Robot_Send_Data Robot_S_D_R, Robot_S_D_tmp_R, Robot_S_D_tmp2_R;
uint8_t Robot_Send_PCK[11],Send_cnt=0;

static void GetNewData_R(uint8_t data)
{	
	if (PCK_Num_R<Header_Lenght)
	{
		switch(PCK_Num_R)
		{	
		case 0:
			if (data == START_BYTE0) 
			{
				PCK_Num_R++;// LED_White_R_PORT.OUTTGL = LED_White_R_PIN_bm;
			}
			break;
		case 1:
			if (data == START_BYTE1) 
				PCK_Num_R++;
			else
			{
				PCK_Num_R = 0;
			}			
			break;
		case 2:
			PCK_H_R.SIB = data;
			PCK_Num_R++;
			break;
		case 3:
			PCK_H_R.CHK = data;
			PCK_Num_R++;
			break;
		}
	}	
	else
	{
		if (PCK_Num_R < PCK_H_R.SIB-1)
		{
			
			switch((PCK_Num_R-Header_Lenght) % Data_Lenght) //0x07 is Size of Robot Data
			{
			case 0:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].RID=data;
				PCK_H_R.CHK -= data;
				break;
			case 1:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M0.Bytes[1]=data;
				PCK_H_R.CHK -= data;
				break;
			case 2:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M0.Bytes[0]=data;
				PCK_H_R.CHK -= data;
				break;
			case 3:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M1.Bytes[1]=data;
				PCK_H_R.CHK -= data;
				break;
			case 4:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M1.Bytes[0]=data;
				PCK_H_R.CHK -= data;
				break;
			case 5:
				Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M2.Bytes[1]=data;
				PCK_H_R.CHK -= data;
				break;
			case 6:
			Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M2.Bytes[0]=data;
			PCK_H_R.CHK -= data;
			break;
			case 7:
			Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M3.Bytes[1]=data;
			PCK_H_R.CHK -= data;
			break;
			case 8:
			Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].M3.Bytes[0]=data;
			PCK_H_R.CHK -= data;
			break;
			case 9:
			Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].KCK=data;
			PCK_H_R.CHK -= data;
			break;
			case 10:
			Robot_D_tmp_R[(PCK_Num_R-Header_Lenght)/Data_Lenght].CHP=data;
			PCK_H_R.CHK -= data;
			break;		
			}
			PCK_Num_R++;
		}		
		else
		{ 
			if (PCK_H_R.CHK == 0 && data == 0x80)
			{	
				PCK_Reset_R = 0;
				PCK_Speed_R++;
			
				for (uint8_t i=0;i<Max_Robot;i++)
				{
					
					if (Robot_D_tmp_R[i].RID!=12)
					{
						Robot_D_R[i] = Robot_D_tmp_R[i];
						Robot_D_tmp_R[i].RID=12;
						Robot_D_tmp_R[i].M0.Speed=0x7FFF;
						Robot_D_tmp_R[i].M1.Speed=0x7FFF;
						Robot_D_tmp_R[i].M2.Speed=0x7FFF;
						Robot_D_tmp_R[i].M3.Speed=0x7FFF;
						Robot_D_tmp_R[i].KCK=0;
						Robot_D_tmp_R[i].CHP=0;

						
						if (Robot_D_R[i].RID<3)
						{
							Buf_Tx_R[Robot_D_R[i].RID][0] = Robot_D_R[i].RID;
							Buf_Tx_R[Robot_D_R[i].RID][1] = Robot_D_R[i].M0.Bytes[1];
							Buf_Tx_R[Robot_D_R[i].RID][2] = Robot_D_R[i].M0.Bytes[0];
							Buf_Tx_R[Robot_D_R[i].RID][3] = Robot_D_R[i].M1.Bytes[1];
							Buf_Tx_R[Robot_D_R[i].RID][4] = Robot_D_R[i].M1.Bytes[0];
							Buf_Tx_R[Robot_D_R[i].RID][5] = Robot_D_R[i].M2.Bytes[1];
							Buf_Tx_R[Robot_D_R[i].RID][6] = Robot_D_R[i].M2.Bytes[0];
							Buf_Tx_R[Robot_D_R[i].RID][7] = Robot_D_R[i].M3.Bytes[1];
							Buf_Tx_R[Robot_D_R[i].RID][8] = Robot_D_R[i].M3.Bytes[0];
							Buf_Tx_R[Robot_D_R[i].RID][9] = Robot_D_R[i].KCK;
							Buf_Tx_R[Robot_D_R[i].RID][10] = Robot_D_R[i].CHP;

							pck_timeout_R[Robot_D_R[i].RID]=0;
						}	
									
					}
				}
				//LED_Green_R_PORT.OUTTGL = LED_Green_R_PIN_bm;
			}
			PCK_Num_R = 0;
		}					
	}
}
struct PCK_Header PCK_H_L;
struct Robot_Data Robot_D_L[Max_Robot], Robot_D_tmp_L[Max_Robot];

struct PCK_Send_Header PCK_S_H_L;
struct Robot_Send_Data Robot_S_D_L, Robot_S_D_tmp_L, Robot_S_D_tmp2_L;
uint8_t Robot_Send_PCK_L[11],Send_cnt_L=0;

static void GetNewData_L(uint8_t data)
{
	if (PCK_Num_L<Header_Lenght)
	{
		switch(PCK_Num_L)
		{
			case 0:
			if (data == START_BYTE0)
			{
				PCK_Num_L++;// LED_White_R_PORT.OUTTGL = LED_White_R_PIN_bm;
			}
			break;
			case 1:
			if (data == START_BYTE1)
			PCK_Num_L++;
			else
			{
				PCK_Num_L = 0;
			}
			break;
			case 2:
			PCK_H_L.SIB = data;
			PCK_Num_L++;
			break;
			case 3:
			PCK_H_L.CHK = data;
			PCK_Num_L++;
			break;
		}
	}
	else
	{
		if (PCK_Num_L < PCK_H_L.SIB-1)
		{
			
			switch((PCK_Num_L-Header_Lenght) % Data_Lenght) //0x07 is Size of Robot Data
			{
				case 0:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].RID=data;
				PCK_H_L.CHK -= data;
				break;
				case 1:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M0.Bytes[1]=data;
				PCK_H_L.CHK -= data;
				break;
				case 2:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M0.Bytes[0]=data;
				PCK_H_L.CHK -= data;
				break;
				case 3:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M1.Bytes[1]=data;
				PCK_H_L.CHK -= data;
				break;
				case 4:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M1.Bytes[0]=data;
				PCK_H_L.CHK -= data;
				break;
				case 5:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M2.Bytes[1]=data;
				PCK_H_L.CHK -= data;
				break;
				case 6:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M2.Bytes[0]=data;
				PCK_H_L.CHK -= data;
				break;
				case 7:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M3.Bytes[1]=data;
				PCK_H_L.CHK -= data;
				break;
				case 8:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].M3.Bytes[0]=data;
				PCK_H_L.CHK -= data;
				break;
				case 9:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].KCK=data;
				PCK_H_L.CHK -= data;
				break;
				case 10:
				Robot_D_tmp_L[(PCK_Num_L-Header_Lenght)/Data_Lenght].CHP=data;
				PCK_H_L.CHK -= data;
				break;
			}
			PCK_Num_L++;
		}
		else
		{
			if (PCK_H_L.CHK == 0 && data == 0x80)
			{
				PCK_Reset_L = 0;
				PCK_Speed_L++;
				
				for (uint8_t i=0;i<Max_Robot;i++)
				{
					
					if (Robot_D_tmp_L[i].RID!=12)
					{
						Robot_D_L[i] = Robot_D_tmp_L[i];
						Robot_D_tmp_L[i].RID=12;
						Robot_D_tmp_L[i].M0.Speed=0x7FFF;
						Robot_D_tmp_L[i].M1.Speed=0x7FFF;
						Robot_D_tmp_L[i].M2.Speed=0x7FFF;
						Robot_D_tmp_L[i].M3.Speed=0x7FFF;
						Robot_D_tmp_L[i].KCK=0;
						Robot_D_tmp_L[i].CHP=0;

						if (Robot_D_L[i].RID<Max_Robot && Robot_D_L[i].RID>2)
						{   
							Buf_Tx_L[Robot_D_L[i].RID][0] = Robot_D_L[i].RID;
							Buf_Tx_L[Robot_D_L[i].RID][1] = Robot_D_L[i].M0.Bytes[1];
							Buf_Tx_L[Robot_D_L[i].RID][2] = Robot_D_L[i].M0.Bytes[0];
							Buf_Tx_L[Robot_D_L[i].RID][3] = Robot_D_L[i].M1.Bytes[1];
							Buf_Tx_L[Robot_D_L[i].RID][4] = Robot_D_L[i].M1.Bytes[0];
							Buf_Tx_L[Robot_D_L[i].RID][5] = Robot_D_L[i].M2.Bytes[1];
							Buf_Tx_L[Robot_D_L[i].RID][6] = Robot_D_L[i].M2.Bytes[0];
							Buf_Tx_L[Robot_D_L[i].RID][7] = Robot_D_L[i].M3.Bytes[1];
							Buf_Tx_L[Robot_D_L[i].RID][8] = Robot_D_L[i].M3.Bytes[0];
							Buf_Tx_L[Robot_D_L[i].RID][9] = Robot_D_L[i].KCK;
							Buf_Tx_L[Robot_D_L[i].RID][10] = Robot_D_L[i].CHP;
                             //LED_White_L_PORT.OUTTGL = LED_White_L_PIN_bm;
							pck_timeout_L[Robot_D_L[i].RID]=0;
						}
					}
				}
				//LED_Green_R_PORT.OUTTGL = LED_Green_R_PIN_bm;
			}
			PCK_Num_L = 0;
		}
	}
}

//void SendNewData()
//{
	//Send_cnt = 0;
	//
	//Robot_Send_PCK[0] = START_BYTE0;
	//Robot_Send_PCK[1] = START_BYTE1;
	//Robot_Send_PCK[2] = PCK_S_H.SIB;
	//Robot_Send_PCK[3] = PCK_S_H.CHK;
	//Robot_Send_PCK[4] = PCK_S_H.RID;
	//Robot_Send_PCK[5] = Robot_S_D.PTP;
	//Robot_Send_PCK[6] = Robot_S_D.EN1;
	//Robot_Send_PCK[7] = Robot_S_D.EN2;
	//Robot_Send_PCK[8] = Robot_S_D.EN3;
	//Robot_Send_PCK[9] = Robot_S_D.EN4;
	//Robot_Send_PCK[10] = 0x80;
	//usart_set_dre_interrupt_level(&USARTE0,USART_INT_LVL_LO);
	//usart_put(&USARTE0,Robot_Send_PCK[Send_cnt]);
	//Send_cnt++;
	//
//}

//void SendData()
//{
	//Send_cnt = 0;
	//
	//Robot_Send_PCK[0] = START_BYTE0;
	//Robot_Send_PCK[1] = START_BYTE1;
	//Robot_Send_PCK[2] = PCK_S_H.SIB;
	//Robot_Send_PCK[3] = PCK_S_H.CHK;
	//Robot_Send_PCK[4] = PCK_S_H.RID;
	//Robot_Send_PCK[5] = Robot_S_D.PTP;
	//Robot_Send_PCK[6] = Robot_S_D.EN1;
	//Robot_Send_PCK[7] = Robot_S_D.EN2;
	//Robot_Send_PCK[8] = Robot_S_D.EN3;
	//Robot_Send_PCK[9] = Robot_S_D.EN4;
	//Robot_Send_PCK[10] = 0x80;
	//usart_set_dre_interrupt_level(&USARTE0,USART_INT_LVL_LO);
	//usart_put(&USARTE0,Robot_Send_PCK[Send_cnt]);
	//Send_cnt++;
	//
//}

#endif /* TRANSMITTER_H_ */




////////
////////SOP	// start of packet (2 bytes)
////////SIB	// size in bytes
////////CHK	// ckeck sum
////////-------------------------------------------
////////RID	// robot id (1 bytes)
////////M1	// motor 1
////////M2	// motor 2
////////M3	// motor 3
////////M4	// motor 4
////////KCK	// kick options
////////CHP	// CHIP & SPIN   #SCCCCCCC
////////-------------------------------------------
////////EOP	// end of packet
////////
////////
////////
////////SOP	// start of packet (2 bytes)
////////CHK	// ckeck sum
////////-------------------------------------------
////////EN1	Batt
////////EN2	Kick
////////EN3
////////EN4
////////-------------------------------------------
////////EOP	// end of packet



