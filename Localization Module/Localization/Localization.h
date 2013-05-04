/*Multi-tile Localization Module. The vision code is incorporated.
It first locates the robot in the world system of a single tile, and 
then adds the relative position vector (indicating the relative position
of different tiles).
When the robot gets out of a tile, it sends a signal to the localization 
controller. The latter will turn on another tile and let the robot know
which tile it is under
*/
#include <math.h>
#define PI 3.14159265
#define dir_correctin 154*PI/180
#define DIST 35.00

extern int active;      //the tile that the robot is under
extern MRF24J40 mrf;
extern const int com_channel;
extern char txBuffer[128];
extern char rxBuffer[128];
extern int rxLen;
/*the relative position of the tiles, the origin is set at the origin of tile on.
the unit here is camera pixels*/
const double dir_vec[4] = {-0.11, 0, -0.2, -0.25};           
const double x_vec[4] = {-75.0, 0, -300.0, -350.0};
const double y_vec[4] = {485.0, 0, -30.0, 460.0};


int distance_square (int x1, int y1, int x2, int y2){
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);  //calculate the square of the distance
}


int minimum (int list[6]){  //to find the max and min in an array of 6 integers (for the distances)
    int i;
    int res = list[0];
    for (i=0; i<=5; i++){
        if (list[i]<res){
            res = list[i];
        }
    }
    return res;
}    


int maximum (int list[6]){  //to find the max and min in an array of 6 integers (for the distances)
    int i;
    int res = list[0];
    for (i=0; i<=5; i++){
        if (list[i]>res){
            res = list[i];
        }
    }
    return res;
}    


void load_led_position(int cam_data[12], int LEDposition[8]){
    /*In each tile there are 4 IR leds. The shape is unsymmetric. 
    This program determines which one is which based on the distance
    between these leds*/
    int x1,y1,x2,y2,x3,y3,x4,y4;  //positions of the IR LEDs from the camera
    int dis[6];  //array storing the distances between each 2 points
    int dismin,dismax;  //the shortest and longest distance
    int LED1x,LED1y,LED2x,LED2y,LED3x,LED3y,LED4x,LED4y;  //Reasigned positions of LEDs (The relative position between LED1,2,3,4 should be fixed)
    // credit 1,2,3,4 are values that will be used to indicate the LED that a detected point corresponds to 
    int credit1 = 0;
    int credit2 = 0;
    int credit3 = 0;
    int credit4 = 0;
    //load the detected points from camera
    x1 = cam_data[0];  
    y1 = cam_data[1];
    x2 = cam_data[3];
    y2 = cam_data[4];
    x3 = cam_data[6];
    y3 = cam_data[7];
    x4 = cam_data[9];
    y4 = cam_data[10];
    //calculate the distance between detected points
    dis[0] = distance_square (x1,y1,x2,y2);  //dis12
    dis[1] = distance_square (x1,y1,x3,y3);  //dis13
    dis[2] = distance_square (x1,y1,x4,y4);  //dis14
    dis[3] = distance_square (x2,y2,x3,y3);  //dis23
    dis[4] = distance_square (x4,y4,x2,y2);  //dis24
    dis[5] = distance_square (x4,y4,x3,y3);  //dis34
    dismin = minimum(dis);
    dismax = maximum(dis);
    //One LED is in both the longest and the shortest line. It will be LED4 (with a credit of 3)
    //One LED is nether in the longest nor the shortest line. It will be LED1 (with a credit of 0)
    //One LED is only in the longest line. It will be LED3 (with a credit of 2)
    //One LED is only in the shortest line. It will be LED2 (with a credit of 1)
    if (dismin==dis[0]){
        credit1 += 1;
        credit2 += 1;
    }
    if (dismax==dis[0]){
        credit1 += 2;
        credit2 += 2;
    }
    if (dismin==dis[1]){
        credit1 += 1;
        credit3 += 1; 
    }
    if (dismax==dis[1]){
        credit1 += 2;
        credit3 += 2;
    }
    if (dismin==dis[2]){
        credit1 += 1;
        credit4 += 1; 
    }
    if (dismax==dis[2]){
        credit1 += 2;
        credit4 += 2;
    }
    if (dismin==dis[3]){
        credit2 += 1;
        credit3 += 1; 
    }
    if (dismax==dis[3]){
        credit2 += 2;
        credit3 += 2;
    }
    if (dismin==dis[4]){
        credit4 += 1;
        credit2 += 1; 
    }
    if (dismax==dis[4]){
        credit4 += 2;
        credit2 += 2;
    }
    if (dismin==dis[5]){
        credit4 += 1;
        credit3 += 1; 
    }
    if (dismax==dis[5]){
        credit4 += 2;
        credit3 += 2;
    }
    //assiging the LED1,2,3,4 to each detected point
    switch (credit1){
        case 0:
            LED1x = x1;
            LED1y = y1;
            break;
        case 1:
            LED2x = x1;
            LED2y = y1;
            break;
        case 2:
            LED3x = x1;
            LED3y = y1;
            break;
        case 3:
            LED4x = x1;
            LED4y = y1;
            break;    
    }
    switch (credit2){
        case 0:
            LED1x = x2;
            LED1y = y2;
            break;
        case 1:
            LED2x = x2;
            LED2y = y2;
            break;
        case 2:
            LED3x = x2;
            LED3y = y2;
            break;
        case 3:
            LED4x = x2;
            LED4y = y2;
            break;    
    }
    switch (credit3){
        case 0:
            LED1x = x3;
            LED1y = y3;
            break;
        case 1:
            LED2x = x3;
            LED2y = y3;
            break;
        case 2:
            LED3x = x3;
            LED3y = y3;
            break;
        case 3:
            LED4x = x3;
            LED4y = y3;
            break;    
    }
    switch (credit4){
        case 0:
            LED1x = x4;
            LED1y = y4;
            break;
        case 1:
            LED2x = x4;
            LED2y = y4;
            break;
        case 2:
            LED3x = x4;
            LED3y = y4;
            break;
        case 3:
            LED4x = x4;
            LED4y = y4;
            break;    
    }
    LEDposition[0] = LED1x;
    LEDposition[1] = LED1y;
    LEDposition[2] = LED2x;
    LEDposition[3] = LED2y;
    LEDposition[4] = LED3x;
    LEDposition[5] = LED3y;
    LEDposition[6] = LED4x;
    LEDposition[7] = LED4y;
}


bool check(int data[8]){
    /*this function communicates to the localization controller 
    when the robot gets lost*/
    bool result = true;
    for (int i=0; i<8; i++){
        if((data[i]<=10) || (data[i]>=1000)){
            result = false;            
        }
    }
    return result;
}

void calibrate(){       //change between localization tiles.
    mrf.SetChannel(15);
    int cam[12];
    int LEDs[8];
    read_data(cam);
    load_led_position(cam, LEDs);
    while(!check(LEDs)){
        sprintf(txBuffer, "No\r\n");
        rf_send(txBuffer, strlen(txBuffer) + 1);
        wait(0.1);
        read_data(cam);
        load_led_position(cam, LEDs);      
    }
    for (int i=0; i<3; i++){
        rxLen = rf_receive(rxBuffer, 128);
        while (rxLen <= 0){
            rxLen = rf_receive(rxBuffer, 128);
        }
        sscanf(rxBuffer,"active: %d\r\n", &active);   
    } 
    mrf.SetChannel(com_channel);
}    


void Locate(double pos[]){     
    /*This functino returns the position of the robot in a world system*/
    int cam[12];
    int LEDs[8];
    read_data(cam);
    load_led_position(cam, LEDs);
    if (!check(LEDs)){      //if cannot receive legit data, request to change the tile
        calibrate();
        return;
    }
    double LED1x = double(LEDs[0]);
    double LED1y = double(LEDs[1]);
    double LED2x = double(LEDs[2]);
    double LED2y = double(LEDs[3]);
    double LED3x = double(LEDs[4]);
    double LED3y = double(LEDs[5]);
    double LED4x = double(LEDs[6]);
    double LED4y = double(LEDs[7]);
    double r, rx, ry;  //distance to the origin
    double betax, betay;
    double theta = 0;  //the angle of robot's position in the world system
    double alpha = 0;  //the direction of world origin in the local system
    double beta = 0;  //the difference between word and local system
    //find the cross point of the longest and the shortest line
    double k12 = (LED2y - LED1y)/(LED2x - LED1x);
    double k34 = (LED4y - LED3y)/(LED4x - LED3x);
    double xcenter = (LED3y - LED1y - k34*LED3x + k12*LED1x)/(k12 - k34);
    double ycenter = k12*(xcenter - LED1x) + LED1y;
    //To localize, LED4 is the orgin, the vertor from LED4 to LED3 is positive x
    rx = xcenter-510;   //the camera resolution is 1024*768. Thus the center of the screen is (510, 384)
    ry = ycenter-384;
    r = rx*rx + ry*ry;
    r = sqrt(r);
    alpha  = atan2(rx, ry);
    betax = double(LED3x-LED4x);
    betay = double(LED3y-LED4y);
    beta = atan2(betax, betay);
    beta += dir_correctin;
    theta = alpha - beta + PI;
    pos[0] = r*cos(theta)+x_vec[active];
    pos[1] = r*sin(theta)+y_vec[active];
    pos[2] = -beta+PI+dir_vec[active];
}