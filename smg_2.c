/*********************************************

README:
If you see that the Chinese part is garbled, 
please change your encoding to UTF-8, thanks!




亲爱的学长,您好!
                
／￣￣￣￣￣￣￣￣￣￣￣￣￣￣＼
|　祝学长学业有成,科科90+　   |
＼                          /
　￣￣∨￣￣￣￣￣￣￣￣￣￣￣
　 ∧＿∧
　(　・∀・)　
　(　 つつヾ
　 | ｜ |　吧唧吧唧
　(＿_)＿)


*********************************************/

/*
 硬件连接：
 数码管显示模块连接到P0口
 超声波模块：Trig->P1.0, Echo->P1.1
 蜂鸣器连接到P2.3
 */
#include<reg52.h>
#define uchar unsigned char
#define uint unsigned int


void uart_init(void);         
void measure_distance(void);  
void display_float(uint num); 
void send_distance(uint distance); // 串口发送距离
void send_char(uchar ch);     
void Delay_us(uint us);       // 我的晶振是11.0592MHz
void delay(uchar x);          

// 全局变量
int fm_flag=0;               
uchar num;                   
uint disnum;                 // 显示数字
uint distance = 0;           


sbit dula=P2^6;             
sbit wela=P2^7;             
sbit fm=P2^3;               
sbit Trig = P1^0;           
sbit Echo = P1^1;           

// 数码管显示段选表
uchar code table_du[]={
0x3f,0x06,0x5b,0x4f,0x66,0x6d,
0x7d,0x07,0x7f,0x6f,0x77,0x7c,
0x39,0x5e,0x79,0x71};

void main()
{
    // 定时器0工作模式1，定时器1工作模式2（用于串口）
    TMOD = 0x21;
    TH0 = 0;
    TL0 = 0;
    
    uart_init();            
    
    EA = 1;                 
    ET0 = 1;               // 开定时器0中断
    
    while(1)
    {
        measure_distance(); // 测量距离
        display_float(distance);
        
        // 蜂鸣器报警
        if(fm_flag == 1)
        {
            fm = 0;         // 蜂鸣器开
        }
        else
        {
            fm = 1;         // 蜂鸣器关
        }
        
        send_distance(distance);        
    }
}

// 50ms定时
void time0() interrupt 1
{
    TH0=(65536-50000)/256;
    TL0=(65536-50000)%256;
    num++;
}

// 在数码管上显示数字(带小数点)
void display_float(uint num)
{
    // 千位
    P0=table_du[num/1000];
    dula=1;
    dula=0;
    P0=0xfe;
    wela=1;
    wela=0;
    delay(10);

    // 百位
    P0=table_du[num%1000/100];
    dula=1;
    dula=0;
    P0=0xfd;
    wela=1;
    wela=0; 
    delay(10);

    // 十位（带小数点）
    P0=table_du[num%100/10] | 0x80;
    dula=1;
    dula=0;
    P0=0xfb;
    wela=1;
    wela=0;
    delay(10);

    // 个位
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
    TMOD |= 0x20;          // 定时器1工作在模式2
    TH1 = 0xFD;            // 波特率9600
    TL1 = 0xFD;
    TR1 = 1;               
}

void send_char(uchar ch)
{
    SBUF = ch;
    while(!TI);            
    TI = 0;                // 清除发送标志
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

// 微秒延时
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
    
    
    TR0 = 1;              // 启动计时
    while(Echo)
    {
        if(TH0 > 0x38)    // 超时
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
    
    // 距离限幅
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

// 毫秒延时
void delay(uchar x)
{
    uint i, j;
    for(i = x; i > 0; i--)
        for(j = 110; j > 0; j--);
}
