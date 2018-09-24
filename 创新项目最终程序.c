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
#define IAP_ADDR	0x0000    //单片机STC12C5A60S2的第一扇区起始地址
#define IAP_ADDR1   0x200     //第二扇区起始地址

/*****函数声明*****/

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

/*****字符数组定义并初始化*****/

uchar str_Temp[20]={0};			 		//暂存数组
uchar receive_number[20]={0};	        //中断数据接收数组
uchar str_initial[20]={"LJ1993$"};      //用于永久存储出厂密码  
                                 

uchar ser_receive;	 //串口接收到的数据
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
	  BEEP=~BEEP;       //输出频率1KHz
	  delay500();       //延时500us
	}

	for(j=200;j>0;j--)
	{
	  BEEP=~BEEP;       //输出频率500Hz
	  delay500();       //延时1ms
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
	  BEEP=~BEEP;       //输出频率500Hz
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


void IapIdle()		  //关Iap总线
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRH = 0x80;
	IAP_ADDRL = 0;
}

void  EraseSector(uint addr)	  //擦除扇区
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



void ISPWriteByte(uchar dat,uint addr)	   //向指定EEPROM地址写入一字节数据
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

void ISPWriteString(uint addr, uchar length, uchar *p)	  //向指定EEPROM地址写入字符串
{
    uchar i;
    for(i = 0; i < length; i++)
    {
        ISPWriteByte(*(p + i),(addr+i));
    }
}


uchar ISPReadByte(uint addr)	   //从指定EEPROM地址读取一字节数据
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

void ISPReadString_to_str_Temp(uint addr, uchar length)	 //从指定EEPROM地址读取字符串到数组str_Temp[20]
{
    uchar i;
	uchar *p=str_Temp;
    for(i = 0; i < length; i++)
    {
        *(p+i) = ISPReadByte(addr+i);
    }

}

void send_byte(uchar x)	  //发送一字节数据
{
	ES = 0;
	SBUF = x;
	while(!TI);
	TI = 0;
	ES = 1;
}


void Send_string(uchar *str)   //发送字符串
{
	
	while(*str) send_byte(*str++);
}

void UartInit(void)		//波特率设定：9600bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xDC;	    	//设定定时初值
	TH1 = 0xDC;		    //设定定时器重装值
	ET1 = 0;		    //禁止定时器1中断
	TR1 = 1;		    //启动定时器1


    EA=1;			    //开总中断
	REN=1;				//允许串口接收 
    ES=1;			    //开串行口中断
}

void  Clear_String(uchar *p)  //清除指定数组
{
      uint i;
      for(i=0;i<20;i++)
	   	{
		 *(p+i)=0;
		 }
}

void ser() interrupt 4	 //中断服务函数 
{
	if(RI==1)
	{
		RI=0;
		ser_receive=SBUF;
	
		
		if(come==1)
		{
			if(ser_receive!='\0'&&ser_receive!='&')	 //指令1以‘&’开头，以‘$’结束
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
		   	if(ser_receive!='\0'&&ser_receive!='*')	  //指令2以‘*’开头，以‘$’结束
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
		   	if(ser_receive!='\0'&&ser_receive!='@')	  //指令3以‘@’开头，以‘$’结束
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

		else if(ser_receive=='&')			//收到指令1
		{  	
			come=1;		
		}
		else if(ser_receive=='*')			//收到指令2
		{
		    come=2;
		}
		else if(ser_receive=='@')			//收到指令3
		{
		    come=3;
		} 
		
	}
		
}

void main()		         //主函数
{ 
  
  uint  m=0;			 //设置计数m初值为0
  uchar k;				 //用于存储原密码长度
  uchar t;				 //用于存储新密码长度
  UartInit();			 //定时器1初始化，并开串口和总中断
  P1=0xff;				 //P1端口初始化

  if(ISPReadByte(IAP_ADDR)==0xff)	 //判断是否是第一次执行。是，则把出厂密码写入EEPROM的IAP_ADDR地址
  {
  EraseSector(0x0200);
  ISPWriteByte(strlen(str_initial),0x0200);
  EraseSector(IAP_ADDR);
  ISPWriteString(IAP_ADDR,strlen(str_initial),str_initial);
  }
  else 								 //否则，不执行
   { ; }
  
  while(1)							 //等待中断
    {
	   if(ser_flag==1)							   //指令1：验证收到的密码
		{
	
        k=ISPReadByte(0x0200);
	    Clear_String(str_Temp);
  		ISPReadString_to_str_Temp(IAP_ADDR, k); 
       
		if(!(strcmp(receive_number,str_Temp)))	   //密码正确
	     
		{
		 tishi();
		 P1=0xff;
		 P1=0x00;
		 Delay500ms();
         P1=0xff;		
		 send_byte('S');							       

		}
		else 				   	                   //密码错误,发出提示
		{
		send_byte('F');

		if(m<4)				    
		{  m++;  }
		else
		{ 
		 m=0;
		 baojing();

		 }				//连续5次密码输入错误，用P1端口模拟蜂鸣器报警10秒（防止有人试密码）	
		
		}

		Clear_String(receive_number);
		ser_flag=0;
	}
	else if(ser_flag==2)					   //指令2：手机软件进入修改密码界面
		{
        k=ISPReadByte(0x0200);
	    Clear_String(str_Temp);
  		ISPReadString_to_str_Temp(IAP_ADDR, k); 
       
		if((strcmp(receive_number,str_Temp)))	//输入的原密码错误
	    {
		send_byte('E');						    //发出错误提示
		}
		else 									//输入的原密码正确
		{ 
		 send_byte('C');
		 }
		 										
		Clear_String(receive_number);
		ser_flag=0;
	}
   else if(ser_flag==3)					     	//指令3：修改密码（必须以指令2为前提）
       {
		  t=strlen(receive_number);
		  EraseSector(IAP_ADDR);
		  ISPWriteString(IAP_ADDR,t,receive_number); 
	      send_byte('T');					 //密码修改成功提示

		  EraseSector(0x0200);
		  ISPWriteByte(strlen(receive_number),0x0200);
		  Clear_String(receive_number);		 //接收数组清零：重要的一步，少了会出错
		  

		  ser_flag=0;
		
         }
    }
}

