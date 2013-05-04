import serial
import pylab 
from math import cos
from math import sin

ser = serial.Serial("/dev/ttyACM0", 115200)

ser.open()
ser.flushInput()

def receive():
    result = []
    buf = ser.read(ser.inWaiting())
    lines = buf.split('\r\n')
    for item in lines[-2].split(','):
      result.append(float(item))
    return result

def plot():
    pylab.ion()
    while True:
      try:
        data = receive()
        pylab.clf()
        pylab.axis([-500,300,-100,500])
        pylab.plot(data[0],data[1],'ro')
        pylab.plot(data[0]+20*cos(data[2]), data[1]+20*sin(data[2]), 'bo')
      #pylab.step( data[0], data[1],data[0]+20*cos(data[2]), data[1]+20*sin(data[2]), color='b', linewidth=2.0)
      except(IndexError):
        pass
      except(ValueError):
        pass
      pylab.draw()

while True:
    plot()
