/*the Motion Module.*/
extern Serial pc;
extern m3pi robot;
extern double position[3];
extern Timer t_battery;
extern int Battery_Level;
extern const double speed;
Timer t_rotate;


void rotate(int final_dir){
    //rotate to a certain angle from 0 to 360
    pc.baud(115200);
    Locate(position);
    int angle = int(position[2]*180/PI);
    angle = (angle+360)%360;
    final_dir = (final_dir+360)%360;
    int diff = (angle - final_dir)%360;
    t_rotate.reset();
    t_rotate.start();
    while (abs(diff)> 5 && t_rotate.read()<3){     //time out after 5 seconds
        Locate(position);        
        angle = int(position[2]*180/PI);
        angle = (angle+360)%360;
        if (((angle<final_dir) && (final_dir-angle<180))||((angle>final_dir) && (angle-final_dir>180))){
            robot.left(0.1);
        }
        else{
            robot.right(0.1);
        }
        diff = (angle - final_dir)%360;
    }
    t_rotate.stop();
}


void move_y_till (int y){
    //move on the y direction till y
    t_battery.start();
    Locate(position);
    rotate(90);
    sprintf(txBuffer, "x: %3.2f, y: %3.2f\r\n",position[0],position[1]);
    rf_send(txBuffer, strlen(txBuffer) + 1);
    int pos_y = int(position[1]);
    int diff = abs(pos_y - y);
    int corr = 0;
    while (diff > 5){
        Locate(position);
        pos_y = int(position[1]);
        if (pos_y < y){
            robot.forward(speed);
        }else{
            robot.backward(speed);
        }
        if (!corr){
            rotate(90);
        }
        diff = abs(pos_y - y);
        corr = (corr+1)%15;
    }
    rotate(90);
    robot.stop();
    Battery_Level -= t_battery.read();
    t_battery.stop();
    t_battery.reset();
    sprintf(txBuffer, "Battery: %d\r\n",Battery_Level);
    rf_send(txBuffer, strlen(txBuffer) + 1);
}


void move_x_till(int x){
    //move on the x direction till x
    t_battery.start();
    rotate(90);
    sprintf(txBuffer, "x: %3.2f, y: %3.2f\r\n",position[0],position[1]);
    rf_send(txBuffer, strlen(txBuffer) + 1);
    Locate(position);
    int pos_x = int(position[0]);
    int distance = x - pos_x;
    rotate(0);
    Locate(position);
    pos_x = int(position[0]);
    int dest = pos_x + distance;
    int diff = abs(distance);
    int corr = 0;
    while (diff > 5){
        if (distance >0){
            robot.forward(speed);
        }else{
            robot.backward(speed);
        }
        if (!corr){
            rotate(0);
        }
        Locate(position);
        distance = dest - int(position[0]);
        diff = abs(distance);
        corr = (corr+1)%15;
    }
    rotate(90);    
    robot.stop();
    Battery_Level -= t_battery.read();
    t_battery.stop();
    t_battery.reset();
    sprintf(txBuffer, "Battery: %d\r\n",Battery_Level);
    rf_send(txBuffer, strlen(txBuffer) + 1);
}


int zig_zag(int ymax, int ymin, int x_coord){
    //do a zigzag between ymin and ymax. do it until reaching x_coord
    Locate(position);
    int direction;
    int curr_x = int(position[0]);
    int diff = abs(curr_x - x_coord);
    if (x_coord > curr_x){
        direction = 1;
    }else{
        direction = -1;
    }
    int keep_going = 1;
    int swt = 0;
    while (diff > 5 && Battery_Level > 30 && keep_going > 0){
        t_battery.start();
        if (swt){
            move_y_till(ymax);
        }else{
            move_y_till(ymin);
        }        
        rotate(0);
        if (direction){
            robot.forward(speed);
            wait(0.6);
        }else{
            robot.backward(speed);
            wait(0.6);
        }
        rotate(90);
        Locate (position);
        robot.stop();
        swt = (swt+1)%2;
        curr_x = int(position[0]);
        diff = abs(x_coord - curr_x);
        keep_going = direction*(x_coord - curr_x);    
    }
    robot.stop();
    if (Battery_Level > 30){
        return 1;
    }else{
        return -1;      
    }
}
    
    
void go_to(int x_coord, int y_coord){
    //go to a certain position on the field
    move_y_till(y_coord);
    move_x_till(x_coord);
    move_y_till(y_coord);
    move_x_till(x_coord);
    move_y_till(y_coord);
}
