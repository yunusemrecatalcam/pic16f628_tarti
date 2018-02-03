/*
 * File:   tarti.c
 * Author: Monster
 *
 * Created on 20 Ocak 2018 Cumartesi, 10:40
 */


#include <xc.h>
#include "config.h"
#include<stdint.h>
#include<stdio.h>

#define _XTAL_FREQ  20000000
#define BUFFSIZE    24
#define SCK         PORTAbits.RA0
#define DATA        PORTAbits.RA3
#define LED         PORTBbits.RB7
#define AVERAGECNT  5
#define DECARRSIZE  10

uint32_t timetick=0;

void T0_init(void);
void serial_init(void);

void read_tarti(void);
void tarti_init(void);
void bin_to_dec();
void set_offset(void);
void measure_force(void);
void write_dec(int32_t);
void send_data(void);
void check_command(void);
char return_n_before(char[],int,unsigned int);

int24_t measure_val = 0;
int24_t offset=0;
int24_t force=0;

uint8_t dec_array[DECARRSIZE];

int measure_flag=0;

struct Serial_Ops{
    uint8_t     command[BUFFSIZE];
    uint16_t    wr_index;
    uint16_t    rd_index;
};

struct Serial_Ops tarti;

void main(void) {
    CMCON = 0x07;
    TRISB = 0b00000110;
    
    T0_init();
    serial_init();
    tarti_init();
    
    tarti.wr_index = 0;
    tarti.rd_index = 0;
    
    SCK = 0;
    LED = 1;
    
    read_tarti();
    set_offset();

    while (1) {
        
        check_command();
        
        if(measure_flag==1){
        measure_force();
        send_data();
        measure_flag=0;
        }else{
            if(measure_flag==2){
                set_offset();
                measure_flag=0;
            }
        }
           
        
        //__delay_ms(500);
        
    }

    return;
}

void T0_init(){
    INTCONbits.GIE=1;
    INTCONbits.T0IE=1;
    INTCONbits.T0IF=0;
    
    OPTION_REGbits.T0CS=0;
    OPTION_REGbits.T0SE=0;
    OPTION_REGbits.PSA=0;//0 for assigned
    OPTION_REGbits.PS0=0;
    OPTION_REGbits.PS1=0;
    OPTION_REGbits.PS2=0;
    
}

void serial_init(){
    TXSTAbits.TX9=0;//8 bit
    TXSTAbits.TXEN=1;//transmit enabled
    TXSTAbits.SYNC=0;//async
    TXSTAbits.BRGH=1;
    
    RCSTAbits.SPEN=1;
    RCSTAbits.RX9=0;
    RCSTAbits.CREN=1;
    
    TRISBbits.TRISB1=1;
    TRISBbits.TRISB2=1;
    
    PIE1bits.RCIE=1;
    SPBRG=129;
    
    PIR1bits.RCIF=0;
}

void interrupt kes(void){
    if(INTCONbits.T0IF && INTCONbits.T0IE){
        timetick++;
        INTCONbits.T0IF=0;
        
    }
    
    if(timetick>=10000){
            timetick=0;
        }
    
    if(PIR1bits.RCIF){
        tarti.command[tarti.wr_index]=RCREG;
        //while(!TXSTAbits.TRMT);
        tarti.wr_index++; 
    }   
    
    if(tarti.wr_index>=24)
        tarti.wr_index=0;
    
}

void tarti_init(){
    
    TRISAbits.TRISA0=0;
    TRISAbits.TRISA3=1;
    TRISAbits.TRISA4=0;
    SCK=0;
   
}

void read_tarti(){
    
    SCK=0;
    while(DATA==1);
    
    for(int kl=0;kl<25;kl++){//channel a 128
        SCK=1;
        SCK=0;
    }
 
    while(DATA==1);
    measure_val = 0;
    for(int i=0; i<24; i++){
        SCK=1;
        measure_val <<= 1;
        
        SCK=0;  
        measure_val |= DATA;
    }
    
    SCK=1;
    measure_val ^=0x800000;
    SCK=0;
  
}

void bin_to_dec(){
    //sprintf(dec_array,"%d",measure_val);   
    write_dec(measure_val);
    
}

void set_offset(){
    int32_t sum=0;
    for(int i=0;i<AVERAGECNT;i++){
    read_tarti();
    sum += -measure_val;//-5 * (int32_t)measure_val;
    }
    sum /= AVERAGECNT;
    offset=sum;
    write_dec(offset);
}

void measure_force(){
    int32_t sum=0;
    for(int i=0;i<AVERAGECNT;i++){
    read_tarti();
    //( 5 * (int32_t)measure_val + offset);
    sum += measure_val;
    }
    sum /=AVERAGECNT;
    force=sum + offset;
    write_dec(force);
}

void check_command(){
    if(tarti.rd_index != tarti.wr_index){
            if(tarti.command[tarti.rd_index]==']' && (return_n_before(tarti.command,tarti.rd_index,1)=='O' || return_n_before(tarti.command,tarti.rd_index,1)=='o' ) && return_n_before(tarti.command,tarti.rd_index,2)=='['){
                measure_flag=1;
                PORTBbits.RB7 ^=1;
            }else{
                if(tarti.command[tarti.rd_index]==']' && (return_n_before(tarti.command,tarti.rd_index,1)=='D' || return_n_before(tarti.command,tarti.rd_index,1)=='d' ) && return_n_before(tarti.command,tarti.rd_index,2)=='['){
                    measure_flag=2;
                    PORTBbits.RB7 ^=1;
                }    
            }
            
            tarti.rd_index=tarti.wr_index;
            
            if(tarti.rd_index>=24)
            tarti.rd_index=0;
        
        }
}

void write_dec(int32_t value){
    int i=0;
    value *=22;
    value /=10000;
    if(value < 0)
    {
        dec_array[0] = '-';
        value = -value;
    }
    else
        dec_array[0] = '+';
    
    dec_array[DECARRSIZE - 1] = NULL;
    i=DECARRSIZE - 2;
    
    while(value){
        dec_array[i--]= value%10 + '0';
        value /= 10;
    }
    
    for(;i>0;i--)
        dec_array[i] = '0';
}

void send_data(){
    for(int cnt=5; cnt<DECARRSIZE; cnt++){
                if(cnt==8){
                    break;
                }
                
                while(!TXSTAbits.TRMT);
                TXREG=(dec_array[cnt]);
                if(cnt==5){
                    while(!TXSTAbits.TRMT);
                    TXREG='.';
                }
        }
        while(!TXSTAbits.TRMT);
        TXREG='\r';
        while(!TXSTAbits.TRMT);
        TXREG='\n';
}
 

char return_n_before(char array[],int position,unsigned int n){
    while(n>0){
        if(position==0){
            position=sizeof(array)/sizeof(array[0]);
        }else{
            position--;
        }
        n--;
    }
    return array[position];
}