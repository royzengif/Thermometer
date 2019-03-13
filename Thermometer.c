///==========================ヘッダファイル================================
#include <reg51.h>
#include <stdio.h>

#define led P0//セグメント選択
#define uchar unsigned char
#define uint unsigned int 


//==========================18B20駆動プログラム===========================

#include <intrins.h>
#include <holder.h>//上にあるヘッダファイル
#define nops(); {_nop_(); _nop_(); _nop_(); _nop_();} 
sbit DQ = P2 ^ 3;//18Ｂ20のデータポート
bit flag = 0;//温度の符号，0はプラス，1はマイナス


void delay11us(uint a)
{
	while (a--);
}
//==========================18B20のリセット関数プログラム=================
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
//============================1バイトを書く================================
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
//============================1バイトを読む================================
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
		DQ = 1;//バスをリリ－スする
		nops();   //4us
		if (DQ)
			dat |= 0x80;
		delay11us(6);
	}
	DQ = 1;
	return(dat);
}
//============================温度データを読む==============================
uint read_temperature()
{
	uchar a, b;
	uint temp;
	rst_18b20();//18Ｂ20をリセットする
	write_byte(0xcc);//ROMに記入するのを避ける
	write_byte(0x44);//温度数値の転換を始める
	delay11us(90);//約1?ぐらいディレイする
	rst_18b20();
	write_byte(0xcc);
	write_byte(0xbe);//RAMにあるデータを読む
	a = read_byte();//低位のバイトを読む
	b = read_byte();//高位のバイトを読む
	temp = (b << 8) | a;
	if (0 == (temp & 0x8000))
	{
		flag = 0;//温度が0℃以上だ
	}
	else if (1 == (temp & 0x8000))
	{
		flag = 1;//温度がマイナスだ
		temp = ~temp + 1;

	}
	temp = temp*0.0625 * 10 + 0.5;//数値の加工（小数を一桁保留して,四捨五入して，十倍にして，小数点を加える）
	return(temp);
}


//=======================メインプログラム===================================

#include <holder.h>
//=======================ポートの分配=======================================
sbit s1 = P1 ^ 0; sbit s2 = P1 ^ 1; sbit s3 = P1 ^ 2; sbit s4 = P1 ^ 3;//??,+ ,-,
sbit red = P2 ^ 1; sbit green = P2 ^ 0;////上限、下限の指示、アラーム
sbit wei4 = P2 ^ 7; sbit wei3 = P2 ^ 6; sbit wei2 = P2 ^ 5; sbit wei1 = P2 ^ 4;//LEDを選択する com4---com1
sbit alarm = P2 ^ 2; ビープ音
uchar code display[13] = { 0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0x7f }; //LED駆動
uchar buf[4] = { 10,10,10,10 };
uint dat;
uchar num = 0;//上限と下限を選択する信号，1は上限，2は下限
uint down = 5, up = 25;//上下限の初期値5℃----25℃
extern bit flag;//温度の符号，0はプラス，1はマイナス

void delay1m(uint x);

void convert(uint dat);

void smg_scan();

void alarm_function();

uint read_temperature();

int main(void)
{
	uchar i;
	down *= 10; up *= 10;
	dat = read_temperature();//温度数値を読む
	delay1m(600);//温度数値の変換をまつ
	while (1)
	{
		dat = read_temperature();
		convert(dat);//読んだ温度数値を転換する
		smg_scan();//表す
		alarm_function();//アラーム
		if (s1 == 0)
		{
			delay1m(10);//ボタンのため、遅延する
			if (s1 == 0)
			{
				num++; alarm = 1;
				while (!s1);//ボタンが回復したまで待つ
			}
		}
		if (num == 1)//上限を設置する
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
						while (!s2);// ボタンが回復したまで待つ
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
						while (!s3);// ボタンが回復したまで待つ
						convert(up);
					}
				}
			} while (s4);//確認
			alarm = 0; delay1m(500); alarm = 1; red = 1;
		}
		if (num == 2)//下限を設置する
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
						while (!s2);// ボタンが回復したまで待つ
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
						while (!s3);// ボタンが回復したまで待つ
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
//=============表すの関数プログラム===================================
void smg_scan()
{
	wei4 = wei3 = wei2 = 1; wei1 = 0;//最低位
	led = display[buf[3]];
	delay1m(3); led = 0xff;
	wei4 = wei3 = wei1 = 1; wei2 = 0;//準低位
	led = display[buf[2]] & 0x7f;//小数点
	delay1m(3); led = 0xff;
	wei4 = wei1 = wei2 = 1; wei3 = 0;//準高位
	led = display[buf[1]];
	delay1m(3); led = 0xff;
	wei2 = wei3 = wei1 = 1; wei4 = 0;//最高位
	led = display[buf[0]];
	delay1m(2); led = 0xff;
}
//========================遅延関数プログラム==========================
//水晶振動子の周波数を12.000?とする
void delay1m(uint x)
{
	uint i, j;
	for (i = 0; i<x; i++); // x ms
	for (j = 0; j<120; j++);//，约1 ms 
}
//===================データを転換する関数プログラム====================
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
//========================アラーム関数プログラム=======================
void alarm_function()
{
	if (dat>up) { alarm = 0; red = 0; }//上限を超えれば，ビープを鳴らす
	else if (dat<down) { alarm = 0; green = 0; }//下限を超えれば，ビープを鳴らす
	else { alarm = 1; red = green = 1; }//なければ、ビープもLEDもオーフする
}
