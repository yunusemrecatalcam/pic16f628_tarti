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
#define BUFFSIZE    24
#define SCK         PORTBbits.RB2
#define DATA        PORTBbits.RB5

uint32_t timetick=0;

void T0_init(void);
void serial_init(void);

void Read_tarti(void);
void tarti_init(void);
int tarti_data[BUFFSIZE]={1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int counter=BUFFSIZE;


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
    tarti_init();
    
    tarti.wr_index=0;
    tarti.rd_index=0;
    
    PORTBbits.RB7 =1;
    
    while (1) {

        if(tarti.rd_index != tarti.wr_index){
            if(tarti.command[tarti.rd_index]=='a'){
                PORTBbits.RB7 =1;
            }else{
                if(tarti.command[tarti.rd_index]=='k'){
                    PORTBbits.RB7=0;
                }
            }
            

            if(tarti.command[tarti.rd_index]=='o'){
                Read_tarti();
                PORTBbits.RB7 ^=1;
                
                for(int k=0;k<24;k++){
                while(!TXSTAbits.TRMT);
                TXREG=(tarti_data[k]+'0');
                }
                
            }
            tarti.rd_index++;
            
            if(tarti.rd_index>=24)
            tarti.rd_index=0;
        
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
    //DATA=1; //data pin input
    //SCK=0; //serial clock output
    TRISBbits.TRISB2=0;
    TRISBbits.TRISB5=1;
}

void Read_tarti(){
    int i=-1;
    counter=23;
    while(counter>=0){
        tarti_data[counter]=0;
        counter--;
    }
    DATA=1;
    SCK=0;
    while(DATA);
    while(i<24){
        SCK=0;
        i++;
        SCK=1;
        if(DATA)
            tarti_data[i]=1;
        
        
                    
    }
    
    SCK=1;
    tarti_data[23] ^=1;
    SCK=0;
    
   
} 
