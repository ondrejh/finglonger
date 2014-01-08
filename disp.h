#ifndef _DISP_H_
#define _DISP_H_

#include <stdbool.h>
#include <inttypes.h>

//************definitions for TM1637*********************
#define ADDR_AUTO  0x40
#define ADDR_FIXED 0x44

#define STARTADDR  0xc0
/**** definitions for the clock point of the digit tube *******/
#define POINT_ON   1
#define POINT_OFF  0
/**************definitions for brightness***********************/
#define  BRIGHT_DARKEST 0
#define  BRIGHT_TYPICAL 2
#define  BRIGHTEST      7

uint8_t Cmd_DispCtrl;
bool _PointFlag;     //_PointFlag=1:the clock point on

void init_disp(void);
void disp_writeByte(int8_t wr_data);
void disp_start(void);
void disp_stop(void);
void disp_displayAll(int8_t DispData[4]);
void disp_displayOne(uint8_t BitAddr,int8_t DispData);
void disp_clearDisplay(void);
void disp_brightness(uint8_t brightness);
void disp_point(bool PointFlag);
int8_t disp_coding(int8_t DispData);

#endif
