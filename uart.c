/** uart module */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdbool.h>

#include "uart.h"
#include "servo.h"

#define TXBUFFLEN 64

uint8_t txbuffer[TXBUFFLEN];
uint8_t txbuf_inptr = 0;
uint8_t txbuf_outptr = 0;

/// hex char into int conversion
int8_t hexc2int(uint8_t c)
{
    if ((c>='0') && (c<='9')) return c-'0'; // 0 - 9
    if ((c>='A') && (c<='F')) return c-'A'+10; // A - F
    return -1; // error (no hex char)
}

/// int into hex char conversion
uint8_t int2hexc(int8_t i)
{
    if ((i>=0) && (i<=9)) return ('0'+i);
    if ((i>=0x0A) && (i<=0x0F)) return ('A'+i-10);
    return 0xFF;
}

/// dec char into int conversion
int8_t decc2int(char c)
{
    if ((c>='0') && (c<='9')) return c-'0'; // 0 - 9
    return -1; // error (no dec char)
}

/// print dec value
void printdec(int dec)
{
    bool neg = false;
    char s[7];
    int8_t ptr=0;

    if (dec<0) {neg=true; dec=-dec;}

    do
    {
        s[ptr++]=int2hexc(dec%10);
        dec/=10;
    }
    while ((dec!=0) && (ptr<6));

    if (neg) s[ptr++]='-';

    ptr--;

    do
    {
        uart_putchar(s[ptr--]);
    }
    while (ptr>=0);
}

/// uart putchar function
int8_t uart_putchar(uint8_t c)
{
    // calculate buffer pointer
    uint8_t next_bufptr = txbuf_inptr+1;
    if (next_bufptr>=TXBUFFLEN) next_bufptr=0;
    // test for risk of buffer overflow
    if (next_bufptr==txbuf_outptr)
        return -1; // buffer full error
    txbuffer[txbuf_inptr]=c;
    // test if its first char (if yes then start tx)
    if (txbuf_outptr==txbuf_inptr) UDR0 = c;
    // push buffer pointer
    txbuf_inptr=next_bufptr;
    // return OK
    return 0;
}

/// uart initialization
void init_uart(void)
{
    UCSR0A = (1<<U2X0);
    UCSR0B = (1<<RXCIE0) | (1<<TXCIE0) | (1<<TXEN0) | (1<<RXEN0);
    //UBRR0 = 3;  // 115.2kBaud / 7.3728MHz
    //UBRR0 = 7;  // 57.6kBaud / 7.3728MHz
    UBRR0 = 16; // 115.2kBaud / 16MHz
    //UBRR0 = 47; // 9.6kBaud / 7.3728MHz
}

/// receive complette interrupt handler
SIGNAL (USART_RX_vect)
//void usart_rx(void)
{
    uint8_t c;

    static uint8_t rxcnt=0;
    static char cmd='\0';
    static char cmdadd='\0';
    static int16_t value=0;

    bool cmdbreak = false;
    int8_t dec = 0;

    // test error flags
    if (UCSR0A&((1<<FE0)|(1<<DOR0)|(1<<UPE0)))
    {
        c = UDR0;
        cmdbreak = true;
        return; // input error
    }
    else
    {
        c = UDR0;
        switch (rxcnt)
        {
            case 0: // command
                if (c=='s')
                {
                    cmd = c;
                    rxcnt++;
                }
                break;
            case 1: // command add (?,-) or value
                dec = decc2int(c);

                if ((c=='?') || (c=='-'))
                {
                    cmdadd = c;
                    rxcnt++;
                }
                else if (dec>=0)
                {
                    value*=10;
                    value+=dec;
                    rxcnt++;
                }
                break;
            case 2: // value or enter
                dec = decc2int(c);

                if (dec>=0)
                {
                    value*=10;
                    value+=dec;
                }
                else
                {
                    cmdbreak = true;

                    if ((c=='\n') || (c=='\r') || (c=='\0'))
                    {
                        // do something
                        if (cmdadd=='-') value = -value;

                        if (cmdadd!='?')
                        {
                            if (cmd=='s') set_servo_position(value);
                        }
                        else
                        {
                            if (cmd=='s') printdec(get_servo_position());
                        }
                    }
                }
                break;
        }
        uart_putchar(c); // echo
        if (c=='\r') uart_putchar('\n');
    }

    if (cmdbreak==true)
    {
        rxcnt=0;
        cmd='\0';
        cmdadd='\0';
        value=0;
    }
}

/// transmit complette interrupt handler
SIGNAL (USART_TX_vect)
//void usart_tx(void)
{
    // calculate buffer pointer
    uint8_t next_bufptr = txbuf_outptr+1;
    if (next_bufptr>=TXBUFFLEN) next_bufptr=0;
    // push buffer pointer
    txbuf_outptr=next_bufptr;

    // test if something left in the buffer (if any then transmitt it)
    if (txbuf_outptr!=txbuf_inptr)
        UDR0=txbuffer[txbuf_outptr];
}
