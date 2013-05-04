//The controller code that switches localization tiles
#include "ZigBee.h"
char txBuffer[128];
char rxBuffer[128];
int rxLen;
int active;
int status;     //1 is working, 0 is searching
DigitalOut Tile1(p15);
DigitalOut Tile2(p16);
DigitalOut Tile3(p17);
DigitalOut Tile0(p18);
MRF24J40 mrf(p11, p12, p13, p14, p21);

void TurnOn(int num){
    //Turn on a certain tile
    num = num%4;
    switch (num){
        case 0:
            Tile1.write(0);
            Tile2.write(0);
            Tile3.write(0);
            Tile0.write(1);
            break;
        case 1:
            Tile1.write(1);
            Tile2.write(0);
            Tile3.write(0);
            Tile0.write(0);
            break;
        case 2:
            Tile1.write(0);
            Tile2.write(1);
            Tile3.write(0);
            Tile0.write(0);
            break;
        case 3:
            Tile1.write(0);
            Tile2.write(0);
            Tile3.write(1);
            Tile0.write(0);
            break;        
    }
}


int main (void)
{
    mrf.SetChannel(15);
    active = 1;
    TurnOn(active);
    while (1){     
        rxLen = rf_receive(rxBuffer, 128);
        if (rxLen>0){
            if (rxBuffer[0] == 'N'){
                status = 0; 
                active = (active+1)%4;
                TurnOn(active); 
            }
        }else{ 
            sprintf(txBuffer, "active: %d\r\n", active);
            rf_send(txBuffer, strlen(txBuffer) + 1);       
        }
        
        
        
        /*
        pc.printf("Now No. %d is On.\r\n", active);
        TurnOn(active);
        wait(2);
        active = (active+1)%4;
        */
        //Tile3.write(1);
        
    }
}
