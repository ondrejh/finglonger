/**
 * Driver module for Catalex 4-digit display with TM1637 chip.
 *
 *  Author:Fred.Chu
 *  Date:9 April,2013
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *      Modified record:
 *
 *  Author: ondrejh.ck@email.cz
 *  Date: January 2014
 *
 *  Module rewriten to C. Functions "writeByte","start","stop" hidden as local.
 *  "Raw Write" functions added. "Set" function simplified to "Set Brightness" only.
 *
 *  I've tried to use HW TWI bus but it doesn't worked. It'll be brobably nessesary
 *  to add some pullups or modify the module.
 *
 * HW:
 * Display is connected through the kind of I2C interface using 2 wires - clk and data.
 * The connection is set with DISP_PORT/PIN/DDR defining the port, and DISP_CLK_PIN resp.
 * DISP_DATA_PIN defining the port pins. Actually it's set to PORTC PIN4 - data,
 * PIN5 - clk, where the HW TWI bus is located (altrough it's not used).
 *
 *   arduino            display
 *  _________          _________
 * |         |  clk   |         |
 * |     PC5 |------->|  88:88  |
 * |     PC4 |<------>|         |
 * |_________|  data  |_________|
 *
 **/


#include <avr/io.h>
#include "disp.h"

#define DISP_PORT PORTC
#define DISP_PIN PINC
#define DISP_DDR DDRC
#define DISP_CLK_PIN PIN5
#define DISP_DATA_PIN PIN4

#define CLK_LOW() do{DISP_PORT&=~(1<<DISP_CLK_PIN);DISP_DDR|=(1<<DISP_CLK_PIN);}while(0)
#define CLK_HIGH() do{DISP_PORT|=(1<<DISP_CLK_PIN);DISP_DDR|=(1<<DISP_CLK_PIN);}while(0)
#define DATA_LOW() do{DISP_PORT&=~(1<<DISP_DATA_PIN);DISP_DDR|=(1<<DISP_DATA_PIN);}while(0)
#define DATA_HIGH() do{DISP_PORT|=(1<<DISP_DATA_PIN);DISP_DDR|=(1<<DISP_DATA_PIN);}while(0)
#define DATA_REL() do{DISP_DDR&=~(1<<DISP_DATA_PIN);DISP_PORT&=~(1<<DISP_DATA_PIN);}while(0)
#define DATA ((DISP_PIN&(1<<DISP_DATA_PIN))!=0)

// local function prototypes
void disp_writeByte(int8_t wr_data);
void disp_start(void);
void disp_stop(void);

// display coding table
static int8_t TubeTab[16] = {0x3f,0x06,0x5b,0x4f,
                             0x66,0x6d,0x7d,0x07,
                             0x7f,0x6f,0x77,0x7c,
                             0x39,0x5e,0x79,0x71};//0~9,A,b,C,d,E,F

//bus and display initialization
void init_disp(void)
{
    // init bus & IOs
    CLK_HIGH();
    DATA_HIGH();
    // set default brightness
    disp_brightness(BRIGHT_TYPICAL);
    // clear display
    disp_clearDisplay();
}

//send one byte over bus (local function)
void disp_writeByte(int8_t wr_data)
{
    uint8_t i,count1=0;
    for(i=0;i<8;i++)        //sent 8bit data
    {
        CLK_LOW();//digitalWrite(Clkpin,LOW);
        if (wr_data & 0x01) DATA_HIGH();//digitalWrite(Datapin,HIGH);//LSB first
        else DATA_LOW();//digitalWrite(Datapin,LOW);
        wr_data >>= 1;
        CLK_HIGH();//digitalWrite(Clkpin,HIGH);
    }
    CLK_LOW();//digitalWrite(Clkpin,LOW); //wait for the ACK
    DATA_HIGH();//digitalWrite(Datapin,HIGH);
    CLK_HIGH();//digitalWrite(Clkpin,HIGH);
    DATA_REL();//pinMode(Datapin,INPUT);
    while(DATA)//digitalRead(Datapin))
    {
        count1 +=1;
        if(count1 == 200)//
        {
            DATA_LOW();//pinMode(Datapin,OUTPUT);
            //digitalWrite(Datapin,LOW);
            count1 =0;
        }
        DATA_REL();//pinMode(Datapin,INPUT);
    }
    DATA_HIGH();//pinMode(Datapin,OUTPUT);
}

//send start signal to TM1637 (local function)
void disp_start(void)
{
    CLK_HIGH();//digitalWrite(Clkpin,HIGH);//send start signal to TM1637
    DATA_HIGH();//digitalWrite(Datapin,HIGH);
    DATA_LOW();//digitalWrite(Datapin,LOW);
    CLK_LOW();//digitalWrite(Clkpin,LOW);
}

//End of transmission (local function)
void disp_stop(void)
{
    CLK_LOW();//digitalWrite(Clkpin,LOW);
    DATA_LOW();//digitalWrite(Datapin,LOW);
    CLK_HIGH();//digitalWrite(Clkpin,HIGH);
    DATA_HIGH();//digitalWrite(Datapin,HIGH);
}

//Display function. Write to full-screen.
void disp_displayAll(int8_t DispData[4])
{
    int8_t SegData[4];
    uint8_t i;
    for(i = 0;i < 4;i ++)
    {
        SegData[i] = disp_coding(DispData[i]);
    }
    disp_displayAll_Raw(SegData);
}

//Display function. Write raw segments to full-screen.
void disp_displayAll_Raw(int8_t SegData[4])
{
  uint8_t i;
  //coding(SegData);
  disp_start();          //start signal sent to TM1637 from MCU
  disp_writeByte(ADDR_AUTO);//
  disp_stop();           //
  disp_start();          //
  disp_writeByte(STARTADDR);//disp_writeByte(Cmd_SetAddr);//
  for(i=0;i < 4;i ++)
  {
    disp_writeByte(SegData[i]);        //
  }
  disp_stop();           //
  disp_start();          //
  disp_writeByte(Cmd_DispCtrl);//
  disp_stop();           //
}

//Display function. Write one 7seg number.
void disp_displayOne(uint8_t SegAddr,int8_t DispData)
{
    int8_t SegData = disp_coding(DispData);
    disp_displayOne_Raw(SegAddr,SegData);
}

//Display function. Write segments to one char.
void disp_displayOne_Raw(uint8_t BitAddr,int8_t SegData)
{
  disp_start();          //start signal sent to TM1637 from MCU
  disp_writeByte(ADDR_FIXED);//
  disp_stop();           //
  disp_start();          //
  disp_writeByte(BitAddr|0xc0);//
  disp_writeByte(SegData);//
  disp_stop();            //
  disp_start();          //
  disp_writeByte(Cmd_DispCtrl);//
  disp_stop();           //
}

//Clear display
void disp_clearDisplay(void)
{
  disp_displayOne_Raw(0x00,0x00);
  disp_displayOne_Raw(0x01,0x00);
  disp_displayOne_Raw(0x02,0x00);
  disp_displayOne_Raw(0x03,0x00);
}

//Display brightness
//To take effect the next time it displays.
void disp_brightness(uint8_t brightness)
{
  Cmd_DispCtrl = 0x88 + brightness;//Set the brightness and it takes effect the next time it displays.
}

//Whether to light the clock point ":".
//To take effect the next time it displays.
void disp_point(bool PointFlag)
{
  _PointFlag = PointFlag;
}

//Code one byte (0..A - use table, 0x7f .. zeros)
int8_t disp_coding(int8_t DispData)
{
  uint8_t PointData;
  if(_PointFlag == POINT_ON)PointData = 0x80;
  else PointData = 0;
  if(DispData == 0x7f) DispData = 0x00 + PointData;//The bit digital tube off
  else DispData = TubeTab[DispData] + PointData;
  return DispData;
}
