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

uint32_t timetick=0;

void T0_init(void);
void serial_init(void);

void main(void) {
    TRISB=0x00;
    TRISA=0x00;
    T0_init();
    serial_init();
    /*while (1) {
        PORTB=0xFF;
        PORTA=0xFF;
    }*/

    //PORTB=0xFF;
    //PORTA=0xFF;
    PORTBbits.RB7 =1;
    while (1) {
        if(timetick>=10000){
            PORTBbits.RB7 ^=1;
            timetick=0;
            while(!TXSTAbits.TRMT);
            TXREG='F';
            
        }
        
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
    
    SPBRG=129;
}

void interrupt kes(void){
    if(INTCONbits.T0IF && INTCONbits.T0IE){
        timetick++;
        INTCONbits.T0IF=0;
        
    }
}
