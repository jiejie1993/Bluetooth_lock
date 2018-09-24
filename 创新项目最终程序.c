#include <reg52.h>
#include <string.h>
#include <intrins.h>

typedef unsigned int  uint;
typedef unsigned char uchar;

sfr IAP_DATA  = 0xc2;
sfr IAP_ADDRH = 0xc3;
sfr IAP_ADDRL = 0xc4;
sfr IAP_CMD   = 0xc5;
sfr IAP_TRIG  = 0xc6;
sfr IAP_CONTR = 0xc7;

sfr AUXR =0x8e;

#define CMD_IDLE	0	      
#define CMD_READ	1		  
#define CMD_PROG	2
#define CMD_ERAS	3

#define ENABLE_IAP	0x83
#define IAP_ADDR	0x0000    //��Ƭ��STC12C5A60S2�ĵ�һ������ʼ��ַ
#define IAP_ADDR1   0x200     //�ڶ�������ʼ��ַ

/*****��������*****/

void  IapIdle();
void  EraseSector(uint addr);
void  ISPWriteByte(uchar dat,uint addr);
void  ISPWriteString(uint addr, uchar length, uchar *p);
void  ISPReadString_to_str_Temp(uint addr, uchar length);
void  send_byte(uchar x);
void  Send_string(uchar *str);
void  Clear_String(uchar *p);

void   xunhuan();
void   tishi();
void   baojing();
void   delay500();
void   Delay500ms();
uchar j;

sbit  BEEP=P0^4;

uchar ISPReadByte(uint addr);

/*****�ַ����鶨�岢��ʼ��*****/

uchar str_Temp[20]={0};			 		//�ݴ�����
uchar receive_number[20]={0};	        //�ж����ݽ�������
uchar str_initial[20]={"LJ1993$"};      //�������ô洢��������  
                                 

uchar ser_receive;	 //���ڽ��յ�������
uint  ser_flag=0;	 
uint  i=0;
uint  come=0;


void delay500(void)
{
  uchar  i;
  for(i=230;i>0;i--);
}

void   xunhuan()
{ 
    for(j=200;j>0;j--)
	{
	  BEEP=~BEEP;       //���Ƶ��1KHz
	  delay500();       //��ʱ500us
	}

	for(j=200;j>0;j--)
	{
	  BEEP=~BEEP;       //���Ƶ��500Hz
	  delay500();       //��ʱ1ms
	  delay500();
	}
}

void baojing()
{
 uchar j;
 for(j=110;j>0;j--)
  {	
    xunhuan();
	xunhuan();
  }
}

void  tishi()
{ 
	for(j=200;j>0;j--)
	{
	  BEEP=~BEEP;       //���Ƶ��500Hz
	  delay500();       
	  delay500();
	  delay500();
	  delay500();
	  delay500();
	  delay500();
	  delay500();
	  delay500();
	}
}

void Delay500ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 22;
	j = 3;
	k = 227;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void IapIdle()		  //��Iap����
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0x80;
	IAP_ADDRL = 0;
}

void  EraseSector(uint addr)	  //��������
{
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_ERAS;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr>>8;
	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;

	_nop_();
	IapIdle();
}



void ISPWriteByte(uchar dat,uint addr)	   //��ָ��EEPROM��ַд��һ�ֽ�����
{
   	IAP_CONTR = ENABLE_IAP;
   	IAP_CMD = CMD_PROG;
   	IAP_ADDRL = addr;
   	IAP_ADDRH = addr>>8;
   	IAP_DATA = dat;

   	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	_nop_();
	IapIdle();
}

void ISPWriteString(uint addr, uchar length, uchar *p)	  //��ָ��EEPROM��ַд���ַ���
{
    uchar i;
    for(i = 0; i < length; i++)
    {
        ISPWriteByte(*(p + i),(addr+i));
    }
}


uchar ISPReadByte(uint addr)	   //��ָ��EEPROM��ַ��ȡһ�ֽ�����
{
	uchar dat;
	IAP_CONTR = ENABLE_IAP;
   	IAP_CMD = CMD_READ;
   	IAP_ADDRL = addr;
   	IAP_ADDRH = addr>>8;

	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	_nop_();

	dat = IAP_DATA;
	IapIdle();
	return dat;
}

void ISPReadString_to_str_Temp(uint addr, uchar length)	 //��ָ��EEPROM��ַ��ȡ�ַ���������str_Temp[20]
{
    uchar i;
	uchar *p=str_Temp;
    for(i = 0; i < length; i++)
    {
        *(p+i) = ISPReadByte(addr+i);
    }

}

void send_byte(uchar x)	  //����һ�ֽ�����
{
	ES = 0;
	SBUF = x;
	while(!TI);
	TI = 0;
	ES = 1;
}


void Send_string(uchar *str)   //�����ַ���
{
	
	while(*str) send_byte(*str++);
}

void UartInit(void)		//�������趨��9600bps@11.0592MHz
{
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xDC;	    	//�趨��ʱ��ֵ
	TH1 = 0xDC;		    //�趨��ʱ����װֵ
	ET1 = 0;		    //��ֹ��ʱ��1�ж�
	TR1 = 1;		    //������ʱ��1


    EA=1;			    //�����ж�
	REN=1;				//�����ڽ��� 
    ES=1;			    //�����п��ж�
}

void  Clear_String(uchar *p)  //���ָ������
{
      uint i;
      for(i=0;i<20;i++)
	   	{
		 *(p+i)=0;
		 }
}

void ser() interrupt 4	 //�жϷ����� 
{
	if(RI==1)
	{
		RI=0;
		ser_receive=SBUF;
	
		
		if(come==1)
		{
			if(ser_receive!='\0'&&ser_receive!='&')	 //ָ��1�ԡ�&����ͷ���ԡ�$������
			{
				receive_number[i]=ser_receive;
				i++;
				if(ser_receive=='$')
				{
					i=0;
					ser_flag=1;
					come=0;
				}	
	
			}

		}
		else if(come==2)
		 { 
		   	if(ser_receive!='\0'&&ser_receive!='*')	  //ָ��2�ԡ�*����ͷ���ԡ�$������
			{
				receive_number[i]=ser_receive;
				i++;
				if(ser_receive=='$')
				{
					i=0;
		            ser_flag=2;
		            come=0;
		        }
		     }
		 }

	    else if(come==3)
		 { 
		   	if(ser_receive!='\0'&&ser_receive!='@')	  //ָ��3�ԡ�@����ͷ���ԡ�$������
			{
				receive_number[i]=ser_receive;
				i++;
				if(ser_receive=='$')
				{
					i=0;
		            ser_flag=3;
		            come=0;
				}
			}
		 }

		else if(ser_receive=='&')			//�յ�ָ��1
		{  	
			come=1;		
		}
		else if(ser_receive=='*')			//�յ�ָ��2
		{
		    come=2;
		}
		else if(ser_receive=='@')			//�յ�ָ��3
		{
		    come=3;
		} 
		
	}
		
}

void main()		         //������
{ 
  
  uint  m=0;			 //���ü���m��ֵΪ0
  uchar k;				 //���ڴ洢ԭ���볤��
  uchar t;				 //���ڴ洢�����볤��
  UartInit();			 //��ʱ��1��ʼ�����������ں����ж�
  P1=0xff;				 //P1�˿ڳ�ʼ��

  if(ISPReadByte(IAP_ADDR)==0xff)	 //�ж��Ƿ��ǵ�һ��ִ�С��ǣ���ѳ�������д��EEPROM��IAP_ADDR��ַ
  {
  EraseSector(0x0200);
  ISPWriteByte(strlen(str_initial),0x0200);
  EraseSector(IAP_ADDR);
  ISPWriteString(IAP_ADDR,strlen(str_initial),str_initial);
  }
  else 								 //���򣬲�ִ��
   { ; }
  
  while(1)							 //�ȴ��ж�
    {
	   if(ser_flag==1)							   //ָ��1����֤�յ�������
		{
	
        k=ISPReadByte(0x0200);
	    Clear_String(str_Temp);
  		ISPReadString_to_str_Temp(IAP_ADDR, k); 
       
		if(!(strcmp(receive_number,str_Temp)))	   //������ȷ
	     
		{
		 tishi();
		 P1=0xff;
		 P1=0x00;
		 Delay500ms();
         P1=0xff;		
		 send_byte('S');							       

		}
		else 				   	                   //�������,������ʾ
		{
		send_byte('F');

		if(m<4)				    
		{  m++;  }
		else
		{ 
		 m=0;
		 baojing();

		 }				//����5���������������P1�˿�ģ�����������10�루��ֹ���������룩	
		
		}

		Clear_String(receive_number);
		ser_flag=0;
	}
	else if(ser_flag==2)					   //ָ��2���ֻ���������޸��������
		{
        k=ISPReadByte(0x0200);
	    Clear_String(str_Temp);
  		ISPReadString_to_str_Temp(IAP_ADDR, k); 
       
		if((strcmp(receive_number,str_Temp)))	//�����ԭ�������
	    {
		send_byte('E');						    //����������ʾ
		}
		else 									//�����ԭ������ȷ
		{ 
		 send_byte('C');
		 }
		 										
		Clear_String(receive_number);
		ser_flag=0;
	}
   else if(ser_flag==3)					     	//ָ��3���޸����루������ָ��2Ϊǰ�ᣩ
       {
		  t=strlen(receive_number);
		  EraseSector(IAP_ADDR);
		  ISPWriteString(IAP_ADDR,t,receive_number); 
	      send_byte('T');					 //�����޸ĳɹ���ʾ

		  EraseSector(0x0200);
		  ISPWriteByte(strlen(receive_number),0x0200);
		  Clear_String(receive_number);		 //�����������㣺��Ҫ��һ�������˻����
		  

		  ser_flag=0;
		
         }
    }
}

