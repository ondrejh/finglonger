#! /usr/bin/env python3

from tkinter import *
from serial import Serial
from time import sleep

#portname = '/dev/ttyACM0'
portname = '/dev/ttyUSB0'

def open_port(portname):
    #open port
    port = Serial(portname,baudrate=115200,timeout=0.001)
    port.write('\n'.encode('ascii'))

    #arduino bootloader timeout
    sleep(3.0)

    #return port
    return port

def close_port(port):
    port.close()

def get_position(port):
    #send request
    port.write('s?\n'.encode('ascii'))

    #read answer
    answ = port.readlines()
    servo_position = 0
    try:
        servo_position = int((answ[-1].decode('ascii').strip())[2:])
        return servo_position
    except:
        print('cant get servo position')
        return

def set_position(port,position):
    #send request
    port.write('s{}\n'.format(position).encode('ascii'))
    #read answer
    answ = port.readlines()
    try:
        return(answ[-1].decode('ascii').strip())
    except:
        print('no echo found')
        return

#application class
class runapp_gui(Frame):
    ''' some gui '''

    def __init__(self,master=None):
        self.root = Tk()
        self.root.title('Servo TEST')
        Frame.__init__(self,master)
        self.createWidgets()

    def createWidgets(self):
        #scrollbar frame
        self.frmScale = Frame(self.root, height=2, bd=1, relief=GROOVE)
        self.frmScale.pack(fill=Y, padx=2, pady=2, side=LEFT)
        #scrollbar
        self.sclServo = Scale(self.frmScale,
                              from_=-500,to=500,
                              length=1010,sliderlength=10,
                              orient=HORIZONTAL,
                              command=self.sliderMove)
        self.sclServo.grid(row=0, column=0, pady=3)
        #quit button
        self.btnConnect = Button(self.frmScale,text='Connect',command=self.connect)
        self.btnConnect.grid(row=0, column=1, pady=3)

    def sliderMove(self,pos):
        if self.btnConnect['text']=='Disconnect':
            set_position(self.port,pos)
        print(pos)

    def connect(self):
        if self.btnConnect['text']=='Connect':
            self.port = open_port(portname)
            self.btnConnect['text']='Disconnect'
        else:
            close_port(self.port)
            self.btnConnect['text']='Connect'

#run application
if __name__ == "__main__":
    app = runapp_gui()
    app.mainloop()
    
'''port = open_port(portname)
pos = get_position(port)
print(pos)
sleep(1)
set_position(port,500)
sleep(1)
set_position(port,-500)
sleep(1)
close_port(port)'''
