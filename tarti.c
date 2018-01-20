/*
 * File:   tarti.c
 * Author: Monster
 *
 * Created on 20 Ocak 2018 Cumartesi, 10:40
 */


#include <xc.h>
#include "config.h"
#include<stdint.h>

#define _XTAL_FREQ  20000000
#define BUFFSIZE    25

uint32_t timetick=0;

void T0_init(void);
void serial_init(void);
char hell;

struct Serial_Ops{
    char command[BUFFSIZE];
    int wr_index;
    int rd_index;
};

struct Serial_Ops tarti;

void main(void) {
    TRISB=0b00000110;
    TRISA=0x00;
    T0_init();
    serial_init();
    
    tarti.wr_index=0;
    tarti.rd_index=0;
    
    PORTBbits.RB7 =1;
    
    hell=RCREG;
    
    while (1) {
        if(timetick>=10000){
            
            timetick=0;
            //while(!TXSTAbits.TRMT);
            //TXREG='F';            
        }
        
        /*if(tarti.rd_index != tarti.wr_index){
            if(tarti.command[tarti.rd_index]=='a'){
                PORTBbits.RB7 =1;
            }else{
                if(tarti.command[tarti.rd_index]=='k'){
                    PORTBbits.RB7=0;
                }
            }
            tarti.rd_index++;
        }*/
        
        if(tarti.rd_index>=24)
            tarti.rd_index=0;
        
        if(tarti.wr_index>=24)
            tarti.wr_index=0;
        
        if(tarti.wr_index%4==0)
            PORTBbits.RB7 ^=1;
        
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
}

void interrupt kes(void){
    if(INTCONbits.T0IF && INTCONbits.T0IE){
        timetick++;
        INTCONbits.T0IF=0;
        
    }
    
    if(PIR1bits.RCIF){
        tarti.command[tarti.wr_index]=RCREG;
        while(!TXSTAbits.TRMT);
        TXREG='F';
        tarti.wr_index++;
        
      
    }   
}
