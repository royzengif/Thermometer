///==========================�إå��ե�����================================
#include <reg51.h>
#include <stdio.h>

#define led P0//���������x�k
#define uchar unsigned char
#define uint unsigned int 


//==========================18B20�l�ӥץ����===========================

#include <intrins.h>
#include <holder.h>//�Ϥˤ���إå��ե�����
#define nops(); {_nop_(); _nop_(); _nop_(); _nop_();} 
sbit DQ = P2 ^ 3;//18��20�Υǩ`���ݩ`��
bit flag = 0;//�¶Ȥη��ţ�0�ϥץ饹��1�ϥޥ��ʥ�


void delay11us(uint a)
{
	while (a--);
}
//==========================18B20�Υꥻ�å��v���ץ����=================
void rst_18b20()
{
	bit flag = 1;
	while (flag)
	{
		while (flag)
		{
			DQ = 1;
			delay11us(1);
			DQ = 0;
			delay11us(50);
			DQ = 1;
			delay11us(6);
			flag = DQ;
		}
		delay11us(40);
		flag = ~DQ;
	}
	DQ = 1;
}
//============================1�Х��Ȥ����================================
void write_byte(uchar dat)
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		DQ = 1;
		_nop_();
		DQ = 0;
		nops(); //4us
		DQ = dat & 0x01;
		delay11us(6);
		dat >>= 1;
	}
	DQ = 1;
}
//============================1�Х��Ȥ��i��================================
uchar read_byte()
{
	uchar dat, i;
	for (i = 0; i < 8; i++)
	{
		DQ = 1;
		_nop_();
		dat >>= 1;
		DQ = 0;
		nops();   //4us
		DQ = 1;//�Х����꣭������
		nops();   //4us
		if (DQ)
			dat |= 0x80;
		delay11us(6);
	}
	DQ = 1;
	return(dat);
}
//============================�¶ȥǩ`�����i��==============================
uint read_temperature()
{
	uchar a, b;
	uint temp;
	rst_18b20();//18��20��ꥻ�åȤ���
	write_byte(0xcc);//ROM��ӛ�뤹��Τ�ܤ���
	write_byte(0x44);//�¶�������ܞ�Q��ʼ���
	delay11us(90);//�s1?���餤�ǥ��쥤����
	rst_18b20();
	write_byte(0xcc);
	write_byte(0xbe);//RAM�ˤ���ǩ`�����i��
	a = read_byte();//��λ�ΥХ��Ȥ��i��
	b = read_byte();//��λ�ΥХ��Ȥ��i��
	temp = (b << 8) | a;
	if (0 == (temp & 0x8000))
	{
		flag = 0;//�¶Ȥ�0�����Ϥ�
	}
	else if (1 == (temp & 0x8000))
	{
		flag = 1;//�¶Ȥ��ޥ��ʥ���
		temp = ~temp + 1;

	}
	temp = temp*0.0625 * 10 + 0.5;//�����μӹ���С����һ�챣������,�Ē����뤷�ƣ�ʮ���ˤ��ƣ�С�����Ӥ��룩
	return(temp);
}


//=======================�ᥤ��ץ����===================================

#include <holder.h>
//=======================�ݩ`�Ȥη���=======================================
sbit s1 = P1 ^ 0; sbit s2 = P1 ^ 1; sbit s3 = P1 ^ 2; sbit s4 = P1 ^ 3;//??,+ ,-,
sbit red = P2 ^ 1; sbit green = P2 ^ 0;////���ޡ����ޤ�ָʾ������`��
sbit wei4 = P2 ^ 7; sbit wei3 = P2 ^ 6; sbit wei2 = P2 ^ 5; sbit wei1 = P2 ^ 4;//LED���x�k���� com4---com1
sbit alarm = P2 ^ 2; �ө`����
uchar code display[13] = { 0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0x7f }; //LED�l��
uchar buf[4] = { 10,10,10,10 };
uint dat;
uchar num = 0;//���ޤ����ޤ��x�k�����źţ�1�����ޣ�2������
uint down = 5, up = 25;//�����ޤγ��ڂ�5��----25��
extern bit flag;//�¶Ȥη��ţ�0�ϥץ饹��1�ϥޥ��ʥ�

void delay1m(uint x);

void convert(uint dat);

void smg_scan();

void alarm_function();

uint read_temperature();

int main(void)
{
	uchar i;
	down *= 10; up *= 10;
	dat = read_temperature();//�¶��������i��
	delay1m(600);//�¶������Ή�Q��ޤ�
	while (1)
	{
		dat = read_temperature();
		convert(dat);//�i����¶�������ܞ�Q����
		smg_scan();//��
		alarm_function();//����`��
		if (s1 == 0)
		{
			delay1m(10);//�ܥ���Τ��ᡢ�W�Ӥ���
			if (s1 == 0)
			{
				num++; alarm = 1;
				while (!s1);//�ܥ��󤬻؏ͤ����ޤǴ���
			}
		}
		if (num == 1)//���ޤ��O�ä���
		{
			convert(up); num = 2; red = 0; green = 1;
			do
			{
				smg_scan();
				if (s2 == 0)
				{
					delay1m(10);
					if (s2 == 0)
					{
						up++;
						if (up>1200) { up = 250; }
						while (!s2);// �ܥ��󤬻؏ͤ����ޤǴ���
						convert(up);
					}
				}
				else if (s3 == 0)//-
				{
					delay1m(10);
					if (s3 == 0)
					{
						up--;
						if (up <= down) { up = 250; }
						while (!s3);// �ܥ��󤬻؏ͤ����ޤǴ���
						convert(up);
					}
				}
			} while (s4);//�_�J
			alarm = 0; delay1m(500); alarm = 1; red = 1;
		}
		if (num == 2)//���ޤ��O�ä���
		{
			convert(down); num = 0; green = 0; red = 1;
			do
			{
				smg_scan();
				if (s2 == 0)//+
				{
					delay1m(10);
					if (s2 == 0)
					{
						down++;
						if (down >= up) { down = 50; }
						while (!s2);// �ܥ��󤬻؏ͤ����ޤǴ���
						convert(down);
					}
				}
				else if (s3 == 0)//-
				{
					delay1m(10);
					if (s3 == 0)
					{
						down--;
						if (down<10) { down = 50; }
						while (!s3);// �ܥ��󤬻؏ͤ����ޤǴ���
						convert(down);
					}
				}
			} while (s4);
			for (i = 0; i<2; i++)
			{
				alarm = 0; delay1m(200); alarm = 1; delay1m(200);
			}
			green = 1;
		}


	}
}
//=============�����v���ץ����===================================
void smg_scan()
{
	wei4 = wei3 = wei2 = 1; wei1 = 0;//���λ
	led = display[buf[3]];
	delay1m(3); led = 0xff;
	wei4 = wei3 = wei1 = 1; wei2 = 0;//�ʵ�λ
	led = display[buf[2]] & 0x7f;//С����
	delay1m(3); led = 0xff;
	wei4 = wei1 = wei2 = 1; wei3 = 0;//�ʸ�λ
	led = display[buf[1]];
	delay1m(3); led = 0xff;
	wei2 = wei3 = wei1 = 1; wei4 = 0;//���λ
	led = display[buf[0]];
	delay1m(2); led = 0xff;
}
//========================�W���v���ץ����==========================
//ˮ������Ӥ��ܲ�����12.000?�Ȥ���
void delay1m(uint x)
{
	uint i, j;
	for (i = 0; i<x; i++); // x ms
	for (j = 0; j<120; j++);//��Լ1 ms 
}
//===================�ǩ`����ܞ�Q�����v���ץ����====================
void convert(uint dat)
{
	buf[0] = dat / 1000;
	buf[1] = dat / 100 % 10;
	buf[2] = dat / 10 % 10;
	buf[3] = dat % 10;
	if (flag == 1)//�ޥ��ʥ��Ǥ����
	{
		buf[0] = 10;
		if (buf[1] == 0) { buf[1] = 11; }
	}
	else if (flag == 0)//�ץ饹�Ǥ����
	{
		if (buf[0] == 0) { buf[0] = 11; }
		if ((buf[0] == 11) && (buf[1] == 0)) { buf[1] = 11; }
	}
}
//========================����`���v���ץ����=======================
void alarm_function()
{
	if (dat>up) { alarm = 0; red = 0; }//���ޤ򳬤���У��ө`�פ��Q�餹
	else if (dat<down) { alarm = 0; green = 0; }//���ޤ򳬤���У��ө`�פ��Q�餹
	else { alarm = 1; red = green = 1; }//�ʤ���С��ө`�פ�LED�⥪�`�դ���
}
