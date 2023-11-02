#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define sec 0x00
#define min 0x01
#define hour 0x02
#define channel 0
#define led 38

//void hen_gio(uint8_t gio, uint8_t phut, uint8_t giay)
uint8_t matran[10]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};


int hh, mm, ss; 
void send_data(uint8_t address, uint8_t value)
{
    uint8_t data[2];
    data[0]=address;
    data[1]=value;
    wiringPiSPIDataRW(channel, data, 2);
    //return 0; //gui tra du lieu tu ic ve (neu ic co chuc nang gui tra du lieu)
}
uint8_t hex2dec(uint8_t h)
{
    uint8_t  d;
    int ch = h >>4;
    int dv = h & 0x0f;
    d = ch*10 + dv;
    return d;
}
void hienthitime(uint8_t gio,uint8_t phut,uint8_t giay )
{
    send_data(1,matran[giay%10]);
    send_data(2,matran[giay/10]);
    send_data(3,1);
    send_data(4,matran[phut%10]);
    send_data(5,matran[phut/10]);
    send_data(6,1);
    send_data(7,matran[gio%10]);
    send_data(8,matran[gio/10]);
}

void Initmax7219()
{
    //decode mode: 0x09FF
    send_data(0x09, 0x00);  //nhap so 
    //intensity: 0x0A08
    send_data(0x0A,0x08);
    //scanlimit:
    send_data(0x0B,7);  //cho phep so luong so 0->7 so
    //no shutdown, display test off
    send_data(0x0C,1); //shutdown = 0
    send_data(0x0F,0); 
}
uint8_t dec2hex(uint8_t d)
{
    uint8_t h;
    h=((d/10)<<4)|(d%10);
    return h;
}

void hengio(void)  //uint8_t gio,uint8_t phut,uint8_t giay
{
    if(hex2dec(hh)==12 &&  hex2dec(mm)==25 && hex2dec(ss)==45 ) // 12gio25phut45giay
        {
            for(int i=0; i<10; i++)
            {
                digitalWrite(led,HIGH);
                delay(200);
                digitalWrite(led,LOW);
                delay(200);
            }
        } 
}

int main(void)
{   
    wiringPiSetupPhys();
    //detup SPI interface
    wiringPiSPISetup(channel, 8000000);  //spi0 , clock speed = 8MHz
    //setup operational mode for max7219
    Initmax7219();
    //setup giao tiep I2C 
    pinMode(38, OUTPUT);   
    digitalWrite(38,HIGH);
    delay(500);
    digitalWrite(38,LOW);
    delay(500);   
    while TRUE
    { 
        digitalWrite(38,0); 
        int ds = wiringPiI2CSetup(0x68);
        // read raspberry time
        time_t T = time(NULL);
        struct tm tm = *localtime(&T);
        //printf("system date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        //printf("system time is: %02d/%02d/%02d\n", tm.tm_hour, tm.tm_min , tm.tm_sec);
        
        //write time to ds3231
        wiringPiI2CWriteReg8(ds, sec, dec2hex(tm.tm_sec));
        wiringPiI2CWriteReg8(ds, min, dec2hex(tm.tm_min));
        wiringPiI2CWriteReg8(ds, hour, dec2hex(tm.tm_hour));
        // check ds3231 time
        int hh, mm, ss; 
        ss = wiringPiI2CReadReg8(ds, sec);
        mm = wiringPiI2CReadReg8(ds, min);
        hh = wiringPiI2CReadReg8(ds, hour);
        printf("ds3231 time is: %x/%x/%x\n", hh, mm, ss);
        delay(1000);
        hienthitime(hex2dec(hh),hex2dec(mm),hex2dec(ss));
        hengio(); //ko chay dc
    }
       return 0;
}
