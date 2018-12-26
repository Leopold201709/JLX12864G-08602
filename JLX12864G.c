
#include <reg52.h>
#include <intrins.h>

sbit lcd_sclk = P3^2; 
sbit lcd_sid = P3^1;
sbit lcd_rs = P3^0;
sbit lcd_reset = P1^0;
sbit lcd_cs1 = P1^1;



#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

uchar code bmp1[];

uchar code jiong1[]={};
uchar code lei1[] = {};

void delay(int n_ms)
{
	int j, k;
}

void delay_us(int n_us)
{
	
}

void waitkey()
{
	
}

//Write Command
void transfer_command_lcd(int data1)
{
	char i;
	lcd_cs1 = 0;
	lcd_rs = 0;
	for(i = 0; i < 8; i++)
	{
		lcd_sclk = 0;
		//delay_us(10);
		if(data1 & 0x80) lcd_sid = 1;
		else lcd_sid = 0;
		lcd_sclk = 1;
		//delay_us(10);
		data1 = data1 << 1;
	}
	lcd_cs1 = 1;
}
//Write Data
void transfer_data_lcd(int data1)
{
	char i;
	lcd_cs1 = 0;
	lcd_rs = 1;
	for(i = 0; i < 8; i++)
	{
		lcd_sclk = 0;
		//delay_us(10);
		if(data1 & 0x80) lcd_sid = 1;
		else lcd_sid = 0;
		lcd_sclk = 1;
		//delay_us(10);
		data1 = data1 << 1;
	}
	lcd_cs1 = 1;
}

//LCD Inital
void inital_lcd()
{
	lcd_reset = 0;			//Reset
	delay(100);
	lcd_reset = 1;
	delay(100);
	transfer_command_lcd(0xE2);			//软复位
	dalay(5);
	transfer_command_lcd(0x2C);			//升压步骤1
	delay(50);
	transfer_command_lcd(0x2E);			//升压步骤2
	dalay(50);
	transfer_command_lcd(0x2F);			//升压步骤3
	dalay(5);
	transfer_command_lcd(0x23);			//粗调对比度， 可设置范围0x20~0x27
	transfer_command_lcd(0x81);			//微调对比度， 
	transfer_command_lcd(0x28);			//微调对比度， 可设置范围0x00~0x3F
	transfer_command_lcd(0xA2);			// 1/9偏压比
	transfer_command_lcd(0xC8);			//行扫描顺序，从上到下
	transfer_command_lcd(0xA0);			//列扫描顺序， 从左到右
	transfer_command_lcd(0x40);			//起始行：第一行显示
	transfer_command_lcd(0xAF);			//开始显示
}

void lcd_address(uint page, uint column)
{
	column = column - 0x01;
	transfer_command_lcd(0xB0 + page - 1); 			//设置页地址，每8行为一页， 全屏共64行，即8页
	transfer_command_lcd(0x10 + (column >> 4 & 0x0F));	//设置列地址的高4位
	transfer_command_lcd(column & 0x0F); 				//设置列地址的低4位
}

void clear_screen()
{
	unsigned char i, j;
	for(i = 0; i < 9; i++)
	{
		transfer_command_lcd(0xB0 + i);
		transfer_command_lcd(0x10);
		transfer_command_lcd(0x00);
		for(j = 0; j < 132; j++)
		{
			transfer_data_lcd(0x00);
		}
	}
}

void display_graphic_16x16(uchar page, uchar column, unchar *dp)
{
	uint i, j;
	for(j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for(i = 0; i < 16; i++)
		{
			transfer_data_lcd(*dp);
			dp++;
		}
	}
}

void display_graphic_8x16(uchar page, uchar column, unchar *dp)
{
	uint i, j;
	for(j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for(i = 0; i < 8; i++)
		{
			transfer_data_lcd(*dp);
			dp++;
		}
	}
}

void display_graphic_5x8(uchar page, uchar column, unchar *dp)
{
	lcd_address(page, column);
	for(i = 0; i < 6; i++)
	{
		transfer_data_lcd(*dp);
		dp++;
	}
}

void send_command_to_ROM(uchar data)
{
	uchar i;
	for(i = 0; i < 8; i++)
	{
		Rom_SCK = 0;
		delay_us(10);
		if(data & 0x80) Rom_IN = 1;
		else Rom_IN = 0;
		data = data << 1;
		Rom_SCK = 1;
		delay_us(10);
	}
}

static uchar get_data_from_ROM()
{
	uchar i;
	uchar ret_data = 0;
	for(i = 0; i < 8; i++)
	{
		Rom_OUT = 1;
		Rom_SCK = 0;
		//delay_us(1);
		ret_data = ret_data << 1;
		if(Rom_OUT) ret_data = ret_data + 1;
		else ret_data = ret_data + 0;
		Rom_SCK = 1;
		//delay_us(1);
	}
	return ret_data;
}
//从指定地址读出数据写到LCD指定位置，16x16
void get_and_write_16x16(ulong fontaddr, uchar page, uchar column)
{
	uchar i, j, disp_data;
	Rom_CS = 0;
	send_command_to_ROM(0x03);
	send_command_to_ROM((fontaddr & 0xFF0000) >> 16);	//地址的高8位， 共24位
	send_command_to_ROM((fontaddr & 0xFF00) >> 8);
	send_command_to_ROM(fontaddr & 0xFF);
	for(j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for(i = 0; i < 16; i++)
		{
			disp_data = get_data_from_ROM();
			transfer_data_lcd(disp_data);				//
		}
	}
	Rom_CS = 1;
}
//从指定地址读出数据写到LCD指定位置，8x16
void get_and_write_8x16(ulong fontaddr, uchar page, uchar column)
{
	uchar i, j, disp_data;
	Rom_CS = 0;
	send_command_to_ROM(0x03);
	send_command_to_ROM((fontaddr & 0xFF0000) >> 16);	//地址的高8位， 共24位
	send_command_to_ROM((fontaddr & 0xFF00) >> 8);
	send_command_to_ROM(fontaddr & 0xFF);
	for(j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for(i = 0; i < 8; i++)
		{
			disp_data = get_data_from_ROM();
			transfer_data_lcd(disp_data);				//
		}
	}
	Rom_CS = 1;
}
//从指定地址读出数据写到LCD指定位置，5x8
void get_and_write_5x8(ulong fontaddr, uchar page, uchar column)
{
	uchar i, j, disp_data;
	Rom_CS = 0;
	send_command_to_ROM(0x03);
	send_command_to_ROM((fontaddr & 0xFF0000) >> 16);	//地址的高8位， 共24位
	send_command_to_ROM((fontaddr & 0xFF00) >> 8);
	send_command_to_ROM(fontaddr & 0xFF);
	
	lcd_address(page, column);
	for(i = 0; i < 5; i++)
	{
		disp_data = get_data_from_ROM();
		transfer_data_lcd(disp_data);				//
	}
	Rom_CS = 1;
}

ulong fontaddr = 0;

void display_GB2312_string(uchar page, uchar column, uchar *text)
{
	uchar i = 0;
	ulong fontaddr = 0;
	while(text[i] > 0x00)
	{
		if(((text[i] >= 0xB0) && (text[i] <= 0xF7)) && (text[i + 1] >= 0xA1))
		{
			fontaddr = (text[i] - 0xB0) * 94;
			fontaddr += (text[i + 1] - 0xA1) + 846;
			fontaddr = (ulong)(fontaddr * 32);
			
			get_and_write_16x16(fontaddr, page, column);
			i = i + 2；
			column = column + 16;
		}
		else if(((text[i] >= 0xA1) && (text[i] <= 0xA3)) && (text[i + 1] >= 0xA1))
		{
			fontaddr = (text[i] - 0xA1) * 94;
			fontaddr += (text[i + 1] - 0xA1);
			fontaddr = (ulong)(fontaddr * 32);
			
			get_and_write_16x16(fontaddr, page, column);
			i = i + 2；
			column = column + 16;
		}
		else if((text[i] >= 0x20) && (text[i] <= 0x7E))
		{
			fontaddr = (text[i] - 0x20);
			fontaddr = (unsigned long) (fontaddr * 16);
			fontaddr = (unsigned long) (fontaddr + 0x3CF80);
			
			get_and_write_8x16(fontaddr, page, column);
			i = i + 1；
			column = column + 8;
		}
		else i++;
	}
}

void display_string_5x8(uchar page, uchar column, uchar *text)
{
	unsigned char i = 0;
	ulong fontaddr = 0;
	while(text[i] > 0x00)
	{
		if((text[i] >= 0x20) && (text[i] <= 0x7E))
		{
			fontaddr = (text[i] - 0x20);
			fontaddr = (unsigned long) (fontaddr * 8);
			fontaddr = (unsigned long) (fontaddr + 0x3BFC0);
			
			get_and_write_5x8(fontaddr, page, column);
			i = i + 1;
			column = column + 6;
		}
	}
}

void main(void)
{
	inital_lcd();
	clear_screen();
	while(1)
	{
		display_GB2312_string(1, 1, "12864, 中文字库");
		display_GB2312_string(3, 1, "16x16, 中文字库");
		display_GB2312_string(5, 1, "8x16, 中文字库");
		delay(4000);
		clear_screen();
		display_string_5x8(1, 1, "<!@#$%^&*()_+/");
	}
}