///==========================ヘッダファイル================================
#include <reg51.h>
#include <stdio.h>

#define led P0//セグメント�x�k
#define uchar unsigned char
#define uint unsigned int 


//==========================18B20�l�咼廛蹈哀薀�===========================

#include <intrins.h>
#include <holder.h>//貧にあるヘッダファイル
#define nops(); {_nop_(); _nop_(); _nop_(); _nop_();} 
sbit DQ = P2 ^ 3;//18Ｂ20のデ�`タポ�`ト
bit flag = 0;//梁業の憲催��0はプラス��1はマイナス


void delay11us(uint a)
{
	while (a--);
}
//==========================18B20のリセット�v方プログラム=================
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
//============================1バイトを��く================================
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
//============================1バイトを�iむ================================
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
		DQ = 1;//バスをリリ��スする
		nops();   //4us
		if (DQ)
			dat |= 0x80;
		delay11us(6);
	}
	DQ = 1;
	return(dat);
}
//============================梁業デ�`タを�iむ==============================
uint read_temperature()
{
	uchar a, b;
	uint temp;
	rst_18b20();//18Ｂ20をリセットする
	write_byte(0xcc);//ROMに��秘するのを閲ける
	write_byte(0x44);//梁業方�､瑠��Qを兵める
	delay11us(90);//�s1?ぐらいディレイする
	rst_18b20();
	write_byte(0xcc);
	write_byte(0xbe);//RAMにあるデ�`タを�iむ
	a = read_byte();//詰了のバイトを�iむ
	b = read_byte();//互了のバイトを�iむ
	temp = (b << 8) | a;
	if (0 == (temp & 0x8000))
	{
		flag = 0;//梁業が0≧參貧だ
	}
	else if (1 == (temp & 0x8000))
	{
		flag = 1;//梁業がマイナスだ
		temp = ~temp + 1;

	}
	temp = temp*0.0625 * 10 + 0.5;//方�､亮唸ぃ�弌方を匯蓐隠藻して,膨�領緘襪靴藤�噴蔚にして��弌方泣を紗える��
	return(temp);
}


//=======================メインプログラム===================================

#include <holder.h>
//=======================ポ�`トの蛍塘=======================================
sbit s1 = P1 ^ 0; sbit s2 = P1 ^ 1; sbit s3 = P1 ^ 2; sbit s4 = P1 ^ 3;//??,+ ,-,
sbit red = P2 ^ 1; sbit green = P2 ^ 0;////貧�沺∀堆泙琳己勝▲▲薊`ム
sbit wei4 = P2 ^ 7; sbit wei3 = P2 ^ 6; sbit wei2 = P2 ^ 5; sbit wei1 = P2 ^ 4;//LEDを�x�kする com4---com1
sbit alarm = P2 ^ 2; ビ�`プ咄
uchar code display[13] = { 0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0x7f }; //LED�l��
uchar buf[4] = { 10,10,10,10 };
uint dat;
uchar num = 0;//貧�泙範堆泙鰡x�kする佚催��1は貧�泯�2は和��
uint down = 5, up = 25;//貧和�泙粒�豚��5≧----25≧
extern bit flag;//梁業の憲催��0はプラス��1はマイナス

void delay1m(uint x);

void convert(uint dat);

void smg_scan();

void alarm_function();

uint read_temperature();

int main(void)
{
	uchar i;
	down *= 10; up *= 10;
	dat = read_temperature();//梁業方�､鰌iむ
	delay1m(600);//梁業方�､����Qをまつ
	while (1)
	{
		dat = read_temperature();
		convert(dat);//�iんだ梁業方�､鰛��Qする
		smg_scan();//燕す
		alarm_function();//アラ�`ム
		if (s1 == 0)
		{
			delay1m(10);//ボタンのため、�W决する
			if (s1 == 0)
			{
				num++; alarm = 1;
				while (!s1);//ボタンが指甠靴燭泙粘�つ
			}
		}
		if (num == 1)//貧�泙鰓O崔する
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
						while (!s2);// ボタンが指甠靴燭泙粘�つ
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
						while (!s3);// ボタンが指甠靴燭泙粘�つ
						convert(up);
					}
				}
			} while (s4);//�_�J
			alarm = 0; delay1m(500); alarm = 1; red = 1;
		}
		if (num == 2)//和�泙鰓O崔する
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
						while (!s2);// ボタンが指甠靴燭泙粘�つ
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
						while (!s3);// ボタンが指甠靴燭泙粘�つ
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
//=============燕すの�v方プログラム===================================
void smg_scan()
{
	wei4 = wei3 = wei2 = 1; wei1 = 0;//恷詰了
	led = display[buf[3]];
	delay1m(3); led = 0xff;
	wei4 = wei3 = wei1 = 1; wei2 = 0;//�糞洋�
	led = display[buf[2]] & 0x7f;//弌方泣
	delay1m(3); led = 0xff;
	wei4 = wei1 = wei2 = 1; wei3 = 0;//�文瀘�
	led = display[buf[1]];
	delay1m(3); led = 0xff;
	wei2 = wei3 = wei1 = 1; wei4 = 0;//恷互了
	led = display[buf[0]];
	delay1m(2); led = 0xff;
}
//========================�W决�v方プログラム==========================
//邦唱尅�嘛咾琳棆�方を12.000?とする
void delay1m(uint x)
{
	uint i, j;
	for (i = 0; i<x; i++); // x ms
	for (j = 0; j<120; j++);//��埃1 ms 
}
//===================デ�`タを���Qする�v方プログラム====================
void convert(uint dat)
{
	buf[0] = dat / 1000;
	buf[1] = dat / 100 % 10;
	buf[2] = dat / 10 % 10;
	buf[3] = dat % 10;
	if (flag == 1)//マイナスであれば
	{
		buf[0] = 10;
		if (buf[1] == 0) { buf[1] = 11; }
	}
	else if (flag == 0)//プラスであれば
	{
		if (buf[0] == 0) { buf[0] = 11; }
		if ((buf[0] == 11) && (buf[1] == 0)) { buf[1] = 11; }
	}
}
//========================アラ�`ム�v方プログラム=======================
void alarm_function()
{
	if (dat>up) { alarm = 0; red = 0; }//貧�泙魍�えれば��ビ�`プを�Qらす
	else if (dat<down) { alarm = 0; green = 0; }//和�泙魍�えれば��ビ�`プを�Qらす
	else { alarm = 1; red = green = 1; }//なければ、ビ�`プもLEDもオ�`フする
}
