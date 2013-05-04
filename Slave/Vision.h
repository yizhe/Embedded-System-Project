//Vision Module. Reading from the wiimote infrared camera

PwmOut camClock(p24);
I2C i2c(p28, p27);        // sda, scl
const int addr = 0xB0;   // define the I2C Address of camera


void i2c_write2(int addr, char a, char b)   //write on the i2c port of mbed
{
    char cmd[2];    
    cmd[0] = a;
    cmd[1] = b;
    i2c.write(addr, cmd, 2);
    wait(0.07); // delay 70ms    
}


void clock_init()
{
    LPC_PWM1->TCR = (1 << 1);               // Reset counter, disable PWM
    LPC_SC->PCLKSEL0 &= ~(0x3 << 12);  
    LPC_SC->PCLKSEL0 |= (1 << 12);          // Set peripheral clock divider to /1, i.e. system clock
    LPC_PWM1->MR0 = 4;                     // Match Register 0 is shared period counter for all PWM1
    LPC_PWM1->MR3 = 2;                      // Pin 24 is PWM output 3, so Match Register 3
    LPC_PWM1->LER |= 1;                     // Start updating at next period start
    LPC_PWM1->TCR = (1 << 0) || (1 << 3);   // Enable counter and PWM    
}


void cam_init()
{
    // Init IR Camera sensor
    i2c_write2(addr, 0x30, 0x01);  //start camera
    wait(0.1);
    i2c_write2(addr, 0x30, 0x08);    
    wait(0.1);
    i2c_write2(addr, 0x06, 0x90); 
    wait(0.1);
    i2c_write2(addr, 0x08, 0xC0);
    wait(0.1);
    i2c_write2(addr, 0x1A, 0x40);
    wait(0.1);
    i2c_write2(addr, 0x33, 0x33);  //mode selection
    wait(0.1);
    i2c_write2(addr, 0x30, 0x08); 
    wait(0.1);
}


void read_data(int res[]) {     
    //for this project, the mode used is the Normal Mode, there are 12 bytes of data in total
    char cmd[8];
    char buf[36];
    int s;
    cmd[0] = 0x36;
    i2c.write(addr, cmd, 1);  //send the read request to camear
    i2c.read(addr, buf, 16);  // read the 16-byte result
    s = buf[3];
    res[0] = buf[1] + ((s & 0x30) <<4);  //x1
    res[1] = buf[2] + ((s & 0xC0) <<2);  //y1
    res[2] = s & 0x0F;  //size of point1
    s = buf[6];
    res[3] = buf[4] + ((s & 0x30) <<4);  //x2
    res[4] = buf[5] + ((s & 0xC0) <<2);  //y2
    res[5] = s & 0x0F;  //size of point2      
    s = buf[9];
    res[6] = buf[7] + ((s & 0x30) <<4);  //x3
    res[7] = buf[8] + ((s & 0xC0) <<2);  //y3
    res[8] = s & 0x0F;  //size of point3  
    s = buf[12];
    res[9]  = buf[10] + ((s & 0x30) <<4);  //x4
    res[10] = buf[11] + ((s & 0xC0) <<2);  //y4
    res[11] = s & 0x0F;  //size of point4          
    wait(0.050); 
}