//The State Machine for the Controller

#include "ZigBee.h"
#define Done 1;
#define Picking 2;
#define Waiting 3;

const int Offline = 0;
const int Working = 1;
const int Charging = 2;
const int channel1 = 1;     //mrf channel for the first robot
const int channel2 = 8;     //mrf channel for the second robot
const int y_min = 50;
const int y_max = 250;
int x_min = -300;
int x_max = 25;
Serial pc(USBTX, USBRX);
char txBuffer[128];
char rxBuffer[128];
char temp[30];
int rxLen;
DigitalOut led1(LED1);
MRF24J40 mrf(p11, p12, p13, p14, p21);
int State;
int RoboState1;
int RoboState2;
int Robot1_battery;
int Robot2_battery;
int active_robot;
double Robot1_pos[3];
double Robot2_pos[3];


int main (void)
{
    pc.baud(115200);
    RoboState1 = Charging;
    RoboState2 = Charging;
    State = Picking;
    int xpos = x_min;
    int ypos = x_max;
    float xre = 0;
    float yre = 0;
    while(true){
        switch(State){
            default:
                State = Picking;
                break;
            
            case 1:     //Done
                State = Done;
                break;
            
            case 2:     //Picking
                mrf.SetChannel(channel1);
                rxLen = rf_receive(rxBuffer, 128);
                State = Picking;
                if (rxLen>0 && rxBuffer[0] == 'R'){     //if robot1 is ready
                    wait(0.5);
                    RoboState1 = Working;
                    sprintf(txBuffer, "%d %d %d %d %d\r\n",xpos,ypos,y_max,y_min,x_max);
                    rf_send(txBuffer, strlen(txBuffer) + 1); 
                    State = Waiting;                
                }else{                
                    mrf.SetChannel(channel2);
                    rxLen = rf_receive(rxBuffer, 128);
                    if (rxLen>0 && rxBuffer[0] == 'R'){     //else if robot2 is ready
                        wait(0.5);
                        sprintf(txBuffer, "%d %d %d %d %d\r\n",xpos,ypos,y_max,y_min,x_max);
                        RoboState2 = Working;
                        rf_send(txBuffer, strlen(txBuffer) + 1); 
                        State = Waiting;
                    }
                }
                break;
            
            case 3:     //Waiting
                rxLen = rf_receive(rxBuffer, 128);
                State = Waiting;
                if(rxLen > 0) {
                    led1 = led1^1;
                    pc.printf("%s", rxBuffer);
                    if(rxBuffer[0] == 'D'){
                        sscanf(rxBuffer, "Died: %f,%f\r\n", &xre, &yre);
                        xpos = int(xre);
                        ypos = int(yre);                     
                        State = Waiting;
                    }
                    if(rxBuffer[0] == 'F'){
                        State = Done;
                    }
                    if(rxBuffer[0] == 'C'){
                        if (RoboState1 == Working){
                            RoboState1 = Charging;
                        }
                        if (RoboState2 == Working){
                            RoboState2 == Charging;
                        }
                        State = Picking;
                    }
                }            
                break;    
        }        
        
    }      
}   