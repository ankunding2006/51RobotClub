#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int

void uart_init(void);
void measure_distance(void);
void display_float(uint num);
void send_distance(uint distance);
void send_char(uchar ch);
void Delay_us(uint us);
void delay(uchar x);

int fm_flag=0;
uchar num;
uint disnum;
uint distance = 0;

//second1234



sbit dula=P2^6;
sbit wela=P2^7;
sbit fm=P2^3;
sbit Trig = P1^0;
sbit Echo = P1^1;

uchar code table_du[]={
0x3f,0x06,0x5b,0x4f,0x66,0x6d,
0x7d,0x07,0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

void main()
{
    TMOD = 0x21;
    TH0 = 0;
    TL0 = 0;
    
    uart_init();
    
    EA = 1;
    ET0 = 1;
    
    while(1)
    {
        measure_distance();
        display_float(distance);
        
        if(fm_flag == 1)
        {
            fm = 0;
        }
        else
        {
            fm = 1;
        }
        
        send_distance(distance);
        delay(100);
    }
}

void time0() interrupt 1
{
    TH0=(65536-50000)/256;
    TL0=(65536-50000)%256;
    num++;
}

void display_float(uint num)
{
    P0=table_du[num/1000];
    dula=1;
    dula=0;
    P0=0xfe;
    wela=1;
    wela=0;
    delay(10);

    P0=table_du[num%1000/100];
    dula=1;
    dula=0;
    P0=0xfd;
    wela=1;
    wela=0; 
    delay(10);

    P0=table_du[num%100/10] | 0x80;
    dula=1;
    dula=0;
    P0=0xfb;
    wela=1;
    wela=0;
    delay(10);

    P0=table_du[num%10];
    dula=1;
    dula=0;
    P0=0xf7;
    wela=1;
    wela=0;
    delay(10);
}

void uart_init()
{
    SCON = 0x50;
    PCON = 0x00;
    TMOD &= 0x0F;
    TMOD |= 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1;
}

void send_char(uchar ch)
{
    SBUF = ch;
    while(!TI);
    TI = 0;
}

void send_distance(uint distance)
{
    uint integer_part = distance / 10;
    uchar decimal_part = distance % 10;
    
    if(integer_part >= 100)
    {
        send_char(integer_part/100 + '0');
        send_char((integer_part/10)%10 + '0');
        send_char(integer_part%10 + '0');
    }
    else if(integer_part >= 10)
    {
        send_char(' ');
        send_char(integer_part/10 + '0');
        send_char(integer_part%10 + '0');
    }
    else
    {
        send_char(' ');
        send_char(' ');
        send_char(integer_part + '0');
    }
    
    send_char('.');
    send_char(decimal_part + '0');
    send_char(' ');
    send_char('c');
    send_char('m');
    send_char('\r');
    send_char('\n');
}

void Delay_us(uint us)
{
    while(us--);
}

void measure_distance(void)
{
    uint time = 0;
    float temp_distance = 0;
    
    Trig = 0;
    Delay_us(5);
    Trig = 1;
    Delay_us(20);
    Trig = 0;
    
    while(!Echo);
    
    TR0 = 1;
    while(Echo)
    {
        if(TH0 > 0x38)
        {
            TR0 = 0;
            distance = 4000;
            return;
        }
    }
    TR0 = 0;
    
    time = (TH0 * 256 + TL0);
    
    temp_distance = ((float)time * 0.034) / 2;
    distance = (uint)(temp_distance * 10);
    
    if(distance < 40)
        distance = 40;
    else if(distance > 4000)
        distance = 4000;
    
    if(distance < 150)
    {
        fm_flag = 1;
    }
    else
    {
        fm_flag = 0;
    }
    
    TH0 = 0;
    TL0 = 0;
}

void delay(uchar x)
{
    uint i, j;
    for(i = x; i > 0; i--)
        for(j = 110; j > 0; j--);
}
