// the State machine for each of the robot(Slave)
#include "m3pi.h"
#include "mbed.h"
#include "Vision.h"
#include "ZigBee.h"
#include "Localization.h"
#include "Motion.h"

#define WAITING 1;
#define WORKING 2;
#define CHARGING 3;

double position[3];
MRF24J40 mrf(p11, p12, p13, p14, p21);
m3pi robot(p23, p9, p10);
Serial pc(USBTX,USBRX);
char txBuffer[128];
char rxBuffer[128];
int rxLen;
const int Charging_X = 50;      //the x cordinate of the charging station
const int Charging_Y = 50;      //the y cordinate of the charging station
const int Battery_Time = 80;    //fake battery that dies after 80 seconds
const int com_channel = 8;      //use different channel on different slaves
const double speed = 0.12;
Timer t_battery;
int State;
int Battery_Level;
int v_data[12];
int active;
DigitalOut Buzzer(p20);


int main(){
    Buzzer.write(0);
    mrf.SetChannel(com_channel); 
    clock_init();
    cam_init();
    pc.baud(115200);
    State = WAITING;
    Battery_Level = Battery_Time;
    t_battery.reset();
    int start_x, start_y, y_min, y_max, dest_x;
    active = 1;
    mrf.SetChannel(15);
    for (int i=0; i<3; i++){        //load the tile it is under;
        rxLen = rf_receive(rxBuffer, 128);
        while (rxLen <= 0){
            rxLen = rf_receive(rxBuffer, 128);
        }
        sscanf(rxBuffer,"active: %d\r\n", &active);
    }
    pc.printf("the active value is: %d\r\n", active);   
    mrf.SetChannel(com_channel);
    while(1){
        switch(State){      //use a state machine to control the behavior or a slave
            default:
                State = WAITING;
                break;
            
            case 1:     //Waiting
                sprintf(txBuffer, "Ready\r\n");
                rf_send(txBuffer, strlen(txBuffer) + 1);
                robot.stop();
                rxLen = rf_receive(rxBuffer, 128);
                if (rxLen > 0){
                    int res = sscanf(rxBuffer, "%d %d %d %d %d\r\n", &start_x, &start_y, &y_min, &y_max, &dest_x);
                    if (res){
                        sprintf(txBuffer, "Yes\r\n");
                        rf_send(txBuffer, strlen(txBuffer) + 1);
                        State = WORKING;
                    }
                }else{
                    State = WAITING;
                }
                break;
            
            case 2:     //Working
                Buzzer.write(1);
                wait(0.2);
                Buzzer.write(0);
                go_to(start_x, start_y);
                Buzzer.write(1);
                wait(0.6);
                Buzzer.write(0);
                if (zig_zag(y_max, y_min, dest_x) > 0){
                    robot.stop();
                    sprintf(txBuffer, "Finished\r\n");
                    rf_send(txBuffer, strlen(txBuffer) + 1);
                    State = WAITING;
                }else{
                    robot.stop();       
                    rotate(90);
                    Locate(position);
                    sprintf(txBuffer, "Died: %3.2f,%3.2f\r\n",position[0],position[1]);
                    rf_send(txBuffer, strlen(txBuffer) + 1);
                    for (int b=0; b<3; b++){
                        Buzzer.write(1);
                        wait(0.2);
                        Buzzer.write(0);
                        wait(0.2);
                    }
                    go_to(Charging_X, Charging_Y);
                    State = CHARGING;
                }
                break;
                
            case 3:     //charging
                sprintf(txBuffer, "Charging: %d\r\n",Battery_Level);
                rf_send(txBuffer, strlen(txBuffer) + 1);
                rxLen = rf_receive(rxBuffer, 128);
                if (rxLen > 0){
                    int res = sscanf(rxBuffer, "%d %d %d %d %d\r\n", &start_x, &start_y, &y_min, &y_max, &dest_x);
                    if (res){
                        State = WORKING;
                     }
                }else{
                    State = CHARGING;
                }
                wait(1);
                Battery_Level += 3;
                if (Battery_Level >= Battery_Time){
                    Battery_Level = Battery_Time;
                    State = WAITING;
                }
                break;
        }       
    }
}