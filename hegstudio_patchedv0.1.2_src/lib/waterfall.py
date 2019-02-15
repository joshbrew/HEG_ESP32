import serial, os, sys, time, traceback

DEBUG = '--debug' in sys.argv

class WaterfallController:
    def __init__(self, port=None):
        self.starttime = time.time()
        self.initialized = False
        self.dy = 0
        self.rgb = [11, 11, 11]
        self.port = port
        self.ser = None
        self.InitSerialPort(port)
        self.decimation = 1
        self.i = 0
    
    def InitSerialPort(self, port=None):
        if not port:
            if self.port:
                port = self.port
            elif '--wport' in sys.argv:
                try:
                    port = int(sys.argv[sys.argv.index('--wport')+1])
                    self.port = port
                except:
                    traceback.print_exc()
        if not port:
            print "No serial port configured for the waterfall. Hope that's okay with you."
            return
        try:
            self.ser = serial.Serial(port=port-1, baudrate=57600, timeout=1)
            self.ser.flushInput()
            self.ser.write('*')
            #self.ser = sys.stdout
            self.initialized = True
        except:
            print "Couldn't open the serial port for the waterfall. Extended error message:"
            traceback.print_exc()
        
    def SetDY(self, dy):
        #dy = self.dy + max(-5, min(5, dy - self.dy))
            
        commands = 0


        if (time.time() - self.starttime < 2.):
            return
        self.i += 1
        if dy == 0 or (dy < 4 and dy > -4 and (dy - self.dy)**2 > 8):
            self.dy = 0
            self.ser.write('*')
            commands += 1

        if (self.i % self.decimation):
            return
        #if not (self.i % (self.decimation**self.decimation)):
        #    self.ser.write('g*')
        #    self.dy = 0
        #    #self.rgb[0] = 55
        if not self.initialized: return
        
        while commands < 3 and not self.dy == dy:
            if   dy - self.dy >= 7:
                self.ser.write("'")
                self.dy  += 10
                commands += 1
            elif dy - self.dy >= 3:
                self.ser.write(']')
                self.dy  += 3
                commands += 1
            elif dy - self.dy > 0:
                self.ser.write('+')
                self.dy  += 1
                commands += 1
            elif dy - self.dy <= -7:
                self.ser.write(';')
                self.dy  -= 10
                commands += 1
            elif dy - self.dy <= -3:
                self.ser.write('[')
                self.dy  -= 3
                commands += 1
            elif dy - self.dy < 0:
                self.ser.write('-')
                self.dy  -= 1
                commands += 1
            
        if DEBUG: print self.dy, commands#, self.rgb
    
    def SetRGB(self, rgb):
        if 1:#((self.i + self.decimation/2) % self.decimation):
            return
        if not self.initialized: return
        i = ((self.i/self.decimation) % 4)-1
        #for i in (0,1,2):
        if 1:
            rgb[i] = self.rgb[i] + max(-2, min(2, rgb[i] - self.rgb[i]))
            rgb[i] = min(60, max(1, rgb[i]))
            if self.rgb[i] != rgb[i]:
                self.ser.write('rgb'[i])
                while self.rgb[i] < rgb[i]:
                    self.ser.write('3')
                    self.rgb[i] += 1
                while self.rgb[i] > rgb[i]:
                    self.ser.write('5')
                    self.rgb[i] -= 1
    
    def Close(self):
        print "Shutting down the waterfall COM port..."
        if not self.initialized:
            print "Waterfall not initialized to start with. Nothing to do here."
            return
        self.ser.close()
        self.initialized = False
