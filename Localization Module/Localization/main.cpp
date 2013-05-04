// the State machine for each of the robot(Slave)
#include "m3pi.h"
#include "mbed.h"
#include "Vision.h"
#include "ZigBee.h"
#include "Localization.h"

#define WAITING 1;
#define WORKING 2;
#define CHARGING 3;

double position[3];
MRF24J40 mrf(p11, p12, p13, p14, p21);
Serial pc(USBTX,USBRX);
char txBuffer[128];
char rxBuffer[128];
int rxLen;
int v_data[12];
int active;


int main(){

    clock_init();
    cam_init();
    pc.baud(115200);
    active = 1;
    mrf.SetChannel(15);
    for (int i=0; i<3; i++){        //load the tile it is under;
        rxLen = rf_receive(rxBuffer, 128);
        while (rxLen <= 0){
            rxLen = rf_receive(rxBuffer, 128);
        }
        sscanf(rxBuffer,"active: %d\r\n", &active);
    }
    while(1){
        Locate(position);
        pc.printf("x: %f, y: %f, dir: %f\r\n", position[0], position[1], position[2]);
    }
 
}