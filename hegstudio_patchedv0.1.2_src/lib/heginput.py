#!/usr/bin/env python


################################################################################
# heginput.py
# A fairly lightweight threaded library for parsing HEG input from Pocket 
# Neurobics devices.
#
# Copyright 2005-2011, Jonathan Toomim (jtoomim@jtoomim.org)
# This software is available under the terms of the GNU Lesser Public License,
# version 3.0. For a copy of this license, see the file LGPL.txt, or visit 
# http://www.gnu.org/licenses/lgpl-3.0.txt.
#
# A copy of the specification for the protocol implemented herein can be found 
# at http://www.pocket-neurobics.com/simplex_spec.htm.
# This is not a complete implementation of the protocol. This is not an elegant
# implementation of the protocol. This is a good enough implementation of the 
# protocol for the uses of software I've written. This was the first major
# python programming project I ever engaged in, so the coding style is pretty
# atrocious at places. Sorry.
#
# Want to adapt this software for your project? Want to make your biofeedback
# hardware supported by this software? Please do! Email me, and I might even
# help.
################################################################################

version = '0.1.2'

import serial
import os, sys, time, thread, struct, traceback
import fakeserial

use_autodetect = True
USE_ARRAY = False
if USE_ARRAY:
    import array

class NoPort(Exception):
    pass
class NoData(Exception):
    pass
class InvalidData(Exception):
    pass
class DataError(Exception):
    pass

_frameFormat = struct.Struct("BbbbbBbbbbBbbbbBbbbb")
SAMPLING_RATES = tuple([1/.008192, ] + [2.**i for i in range(7,14)])
def _decodeFrame(bytes):
    # signal type byte (bytes[0])
    bytes = _frameFormat.unpack(bytes)
    lowbat = bool(bytes[0] & 32)
    resolution = (8, 9, 11, 16)[ (bytes[0]&24)>>3 ]
    sps = SAMPLING_RATES[ bytes[0]&7 ]
    
    # we are ignoring the status byte (bytes[10) except for the ALC data, which
    # is also used for the 3 LSB for HEG
    alc = (bytes[10]&224)>>5
    if resolution == 1: # 11-bit using ALC for 3 LSB
        ch1 = bytes[11] + alc/8.
    
def LinearFIRFilterFactory(taps):
    def LinearFIRFilter(data):
        if taps > len(data):
            newtaps = len(data)
        else:
            newtaps = taps
        newdata = data[-newtaps:]
        result = 0.
        for i in range(newtaps):
            result += (i+.5) * newdata[i]
        return 2.*result/float(newtaps*newtaps)
    return LinearFIRFilter

def enumerate_serial_ports():
    if sys.platform == 'win32':
        import _winreg as winreg

        path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
        try:
            key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
        except WindowsError:
            return []
        
        i = 0
        comports = []
        while 1:
            try:
                val = winreg.EnumValue(key, i)
                comports.append((str(val[1]), str(val[0])))
                i += 1
            except EnvironmentError:
                break
        
    elif sys.platform == 'darwin':
        comports = [('/dev/'+d, d) for d in os.listdir('/dev/') if d.startswith('cu.usbserial')]
    
    elif sys.platform == 'linux':
        comports = [('/dev/'+d, d) for d in os.listdir('/dev/') if d.startswith('ttyUSB')]

    else:
        comports = []

    return comports


#DATATYPES = {'NONE':0, 'HEG':1, 'pirHEG':2, 'EEG':3, 'EEGALC':4}

def twomsb(byte):
    return ord(byte) >> 6
def twomsbpairsync(one, two):
    return ((ord(one)/64 +1) % 4) == (ord(two)/64)

def findsync(vec):
    """\
    Searches a sequence of at least 20 bytes for the simplex serial
    stream synchronization markers. Returns the offset of the start
    of the first frame found via those sync markers. (Returns 0 if 
    the stream is already synced.) Returns -1 if no sync is found.
    """
    
    # optimization: quickly test to see if we're already synced at byte 0
    try:
        if ord(vec[0])>>6 == 0 and ord(vec[5])>>6 == 1 and ord(vec[10])>>6 == 2 and ord(vec[15])>>6 == 3:
            return 0
    except: # an exception will be raised if we don't have enough data; we deal with that below.
        pass
    # end optimization
    
    if (len(vec) < 20):
        #print "in findsync, len(vec) is only ", len(vec)
        raise DataError
    
    isFrameByte = [True]*5 
    for a in range(0, 3):
        isFrameByte = map((lambda x,y: x&y), map(twomsbpairsync, vec[5*a:5*(a+1)], vec[5*(a+1):5*(a+2)]), isFrameByte)
    frameByte = -1
    for a in range(0, 5):
        if (isFrameByte[a]):
            frameByte = a
    if (frameByte != -1):
        frameByte = frameByte + 5*((4 - twomsb(vec[frameByte]))%4)
    return frameByte


class PNBCommon:
    def hegvalue(self, vec):
        resolution = (8, 9, 11, 16)[ (ord(vec[0])&24)>>3 ]
        if resolution == 16:
            if self.channel == 1:
                result = ord(vec[1]) + ord(vec[3])/256. - 128.
            elif self.channel == 2:
                result = ord(vec[2]) + ord(vec[4])/256. - 128.
        else: #resolution != 16
            if (self.channel == 1):
                result = ord(vec[6])  +   (ord(vec[5]) % 8)  / 8. - 128
            elif (self.channel == 2):
                result = ord(vec[7])  +  (ord((vec[5]) % 64)/8) / 8. - 128
            elif (self.channel == 3):
                result = ord(vec[18]) +  (ord(vec[15]) % 8)     / 8. - 128
            elif (self.channel == 4):
                result = ord(vec[19]) + ((ord(vec[15]) % 64)/8) / 8. - 128
        if (result < 0):
            result = 256 + result
        return result    

class PNBFrame(PNBCommon):
    def __init__(self, raw, types = (1, 0, 3, 3)):
        self.raw = raw
        if not len(self.raw) == 20:
            self.sync = False
            self.values = [False, False, False, False]
            self.types = []
    
class hegstream(PNBCommon):
    class SyncError(Exception):
        pass
    class DataError(Exception):
        pass
    def __init__(self):
        self.byterate = 610.
        self.channel = -1
        self.live = True
        self.lograw = False
        self.logdata = False
        self.rawlog = False
        self.datalog = False
        self.comport = False
        if USE_ARRAY:
            self.rawdata = array.array('B') # C: unsigned char, python: int
            self.unfilteredhegdata = array.array('f')
            self.hegdata = array.array('f')
        else:
            self.rawdata = ""
            self.unfilteredhegdata = []
            self.hegdata = []
        self.syncerrors = []
        self.times = []
        self.logdir = '.'
        self.running = True
        self.streaminterrupted = False
        self.hegave = 36
        self.logmutex = thread.allocate_lock()
        self.filter = lambda x: x[-1]
    def flushInput(self):
        if self.live:
            self.comport.flushInput()
        
    def datareadthread(self):
        try:
            lastread = time.time()
            #if not self.live:
            #    self.rawdata.extend(map(ord, self.comport.read()))
            #else:
            newdata = ""
            while self.running:
                if self.paused:
                    self.flushInput()
                    time.sleep(.05)
                else:
                    self.logmutex.acquire()
                    while self.inWaiting()>20:
                        self.streaminterrupted = False
                        try:
                            newbytes = self.comport.read(20)
                        except ValueError: # file/port has been closed
                            return
                        newdata = newbytes
                        self.rawdata += newdata
                        lastread = time.time()
                    if time.time() > lastread + 60./self.byterate:
                        self.streaminterrupted = True
                    if self.lograw and newdata:
                        try:
                            self.rawlog.write(newbytes)
                            self.rawlog.flush()
                        except ValueError:
                            return
                        newdata = ""
                        newbytes = ""
                    time.sleep(1./self.byterate)
                    self.logmutex.release()
            #thread.exit()
        except:
            pass#traceback.print_exc()
        
    def logRawToFile(self, log):
        self.logmutex.acquire()
        if self.lograw:
            self.rawlog.close()
        self.lograw = True
        self.rawlog = file(log, 'w')
        self.rawlog.write(self.rawdata)
        self.logmutex.release()

    def waitForBuffer(self, samples, waittimeout=1):
        while self.dataWaiting() < samples:
            time.sleep(.01) 
            waittimeout -= .01 

    def inWaiting(self):
        #if self.live:
            return self.comport.inWaiting()
        #else:
        #    return int((time.time() - self.starttime) * self.byterate - len(self.rawdata))

    def dataWaiting(self):
        return max(0, len(self.rawdata) / 20 - len(self.hegdata))
        #if len(self.rawdata) / 20 <= len(self.hegdata):
        #    return 0
        #else: return len(self.rawdata) / 20 - len(self.hegdata)
            

    def initialize(self, port, channel=1, rawlog=False, datalog=False, timeout=0, skip=0, fast=0, pause=False):
        self.devicetype = 'unknown' # we'll detect this and modify it later
        self.channel = channel
        self.port = port
        self.paused = pause
        time1 = self.starttime = time.time()
        if type(port) in (str, unicode) and not port.startswith('/dev'):
            if fast:
                self.comport = fakeserial.FakeSerial(port, 'rb', byterate=0)
            else:
                self.comport = fakeserial.FakeSerial(port, 'rb', byterate=610)
            self.live = False
            rawlog, datalog = False, False
        else:
            if type(port) == int:
                port = port-1  # COM1 is 0; COMn is n - 1
            self.comport = serial.Serial(port=port, baudrate=38400, timeout=.4)#parity=serial.PARITY_NONE, timeout=.4)
            #self.comport.flushInput()
            self.rawdata += self.comport.read(60)
            self.live = True
            
        if not self.live:
            self.comport.seek(skip)
        ct = time.localtime() 
        if self.live:
            if (rawlog == True):        #initialize log files
                self.lograw = rawlog
                rawlog = "%4i.%2i.%2i_" % (ct[0], ct[1], ct[2]) + "%2i".zfill(2) %ct[3] + "%2i".zfill(2) %ct[4] + "_raw.pnb"
                rawlog = os.path.join(self.logdir, rawlog)
                self.rawlog = file(rawlog, 'w')
            elif (rawlog):
                self.lograw = rawlog
                self.rawlog = file(rawlog, 'w')
            if (datalog == True):
                self.logdata = datalog
                datalog = "%4i.%2i.%2i_" % (ct[0], ct[1], ct[2]) + "%2i".zfill(2) %ct[3] + "%2i".zfill(2) %ct[4] +"_hegdata.txt"
                datalog = os.path.join(self.logdir, datalog)
                self.datalog = file(datalog, 'w')
            elif (datalog):
                self.logdata = datalog
                self.datalog = file(datalog, 'w')
        if fast:
            self.rawdata += self.comport.read()
        elif use_autodetect:
            self.rawdata += self.comport.read(400)
            #print len(self.rawdata)
        else:
            self.rawdata += self.comport.read(60)
        firstbyte = findsync(self.rawdata)
        if not firstbyte == 0:                                       # if the data stream's not synced, sync it
            self.streaminterrupted = True
            self.rawdata = self.rawdata[firstbyte:]#del self.rawdata[0:firstbyte]
            self.rawdata += self.comport.read(firstbyte)
        
        read_attempts = 0
        i = 0
        while (i+19 < len(self.rawdata)) and read_attempts < 20:                            # check/fix sync on each 20-byte 'word', then extract HEG value
            firstbyte = findsync(self.rawdata[i:20+i])
            #print firstbyte
            if (firstbyte == -1):
                self.rawdata = self.rawdata[:i] + self.rawdata[i+20:]
                self.rawdata +=  self.comport.read(20)
                self.syncerrors.append(len(self.hegdata))
                if self.live: read_attempts += 1
            elif firstbyte:
                self.streaminterrupted = True
                self.rawdata = self.rawdata[:i] + self.rawdata[i+firstbyte:] #del self.rawdata[i:i + firstbyte]
                self.rawdata += self.comport.read(firstbyte)
                self.syncerrors.append(len(self.hegdata))
                if self.live: read_attempts += 1
                #print "Sync error, i = %1i, len(rawdata) = %i" % (i, len(self.rawdata))
            else:
                newdatum = self.hegvalue(self.rawdata[i:i+20])
                self.unfilteredhegdata.append(newdatum)
                self.hegdata.append(newdatum)
                i += 20
        if read_attempts and len(self.rawdata):
            raise InvalidData
        if use_autodetect:
            while self.dataWaiting() and len(self.hegdata) < 30:
                self.readheg(1)
            self.devicetype = self.autodetectDevice()
            if not self.devicetype in ('unknown', 'ambiguous', 'pocket', 'peanut'):
                self.setFilter(LinearFIRFilterFactory(64))
        if fast:
            while self.dataWaiting():
                self.readheg(1)
            self.devicetype = self.autodetectDevice()
            if not self.devicetype in ('unknown', 'ambiguous', 'pocket', 'peanut'):
                self.setFilter(LinearFIRFilterFactory(32))
                self.recalculateValues()
        time2 = time.time()
        iw  = self.inWaiting()/610.
        hdl = len(self.hegdata)
        self.times.extend([time1 + i*(time2-time1)/ hdl - iw for i in range(len(self.hegdata))])
        
        if self.lograw:
            self.rawlog.write(self.rawdata)
        if self.logdata:
            for i in range(len(self.hegdata)):
                self.datalog.write(str(self.hegdata[i]) + '\n')
        if not fast:
            thread.start_new_thread(self.datareadthread, ())
        
    def recalculateValues(self):
        self.logmutex.acquire()
        while self.dataWaiting():
            self.readheg(1)
        self.logmutex.release()
    
    def setFilter(self, filter):
        self.filter = filter
        for i in range(1, len(self.unfilteredhegdata)):
            self.hegdata[i] = self.filter(self.unfilteredhegdata[:i])
        
    def autodetectDevice(self):
        hegdatalen = len(self.hegdata)
        if hegdatalen < 10:
            return 'unknown'
        
        data = self.hegdata[-30:]
        
        for datum in data:
            if (datum*8) - int(datum*8):
                return 'peanut'
            
        dl = len(data)
        dataave = sum(data)/dl
        if not dataave:
            return 'unkown'
        slope = sum([(data[i]-dataave)*(i-dl/2.) for i in range(dl)])/sum([(i-dl/2)**2 for i in range(dl)])/dl
        residuals = [data[i] - dataave - slope*(i-dl/2.) for i in range(dl)]
        std = sum([r**2 for r in residuals])**.5/dl
        
        diffs = [data[i] - data[i-1] for i in range(1, dl)]
        rmsdiff = sum([(d/dataave)**2 for d in diffs])**.5
        if rmsdiff < .05:
            return 'pocket'
        elif rmsdiff < .08:
            if '--debug' in sys.argv: print "Warning: couldn't determine if the device is a pendant or a pocket; assuming pocket."
            return 'ambiguous' # pockets usually produce rmsdiffs around .003
        if dataave < 1.:
            return 'pendant1'
        else:
            return 'pendant2'
            
        
    def readheg(self, samples, channel=1):
        expectSyncError = False
        if self.live:
            if (self.inWaiting() > 800):
                self.comport.flushInput()
                expectSyncError = True
        ipos = len(self.hegdata)     #ipos is the starting position, for use with the return statement
        i = ipos * 20                #i is the current position in self.rawdata
        target = 20 * samples + i     #Make target the desired length of self.rawdata
        if not self.dataWaiting():
            self.waitForBuffer(samples)
            #self.rawdata.extend(map(ord, self.comport.read(20)))
            #if ((len(self.rawdata) < i + 20)):
                #    print "Serial port problems"
        if self.dataWaiting():
            unprocesseddata = len(self.rawdata[i:20+i])
            if unprocesseddata < 20: #this shouldn't be necessary; safety check
                print "insufficient unprocessed data"
            #    print "Missing %i bytes of data in queue in function readheg().  Reading it now." % (20 - unprocesseddata)
            #    self.rawdata.extend(map(ord, self.comport.read(20 - unprocesseddata)))
            firstbyte = findsync(self.rawdata[i:20+i])
            while firstbyte:
                if (firstbyte == -1):
                    self.rawdata = self.rawdata[:i] + self.rawdata[i+20:]
                    self.waitForBuffer(samples)
                    # self.rawdata.extend(map(ord, self.comport.read(20)))
                else:
                    self.rawdata = self.rawdata[:i] + self.rawdata[i+firstbyte:]
                    self.waitForBuffer(samples)
                if not expectSyncError:
                    self.syncerrors.append(len(self.hegdata))
#                    print "Sync error, i = %1i, firstbyte = %2i" % (i, firstbyte)
                else:
                    expectSyncError = False
                self.waitForBuffer(samples)
                firstbyte = findsync(self.rawdata[i:20+i])
            self.unfilteredhegdata.append(self.hegvalue(self.rawdata[i:i+20]))
            self.hegdata.append(self.filter(self.unfilteredhegdata))
            i += 20
            expectSyncError = False
            if self.logdata:
                for i in range(ipos, len(self.hegdata)):
                    self.datalog.write(str(self.hegdata[i]) + '\n')
                self.datalog.flush()
        return self.hegdata[ipos:]

    def hegmean(self, samples, offset=0):
        result = 0
        if (len(self.hegdata) >= samples + offset):
            for i in range(-samples - offset, -offset):
                result += self.hegdata[i]
            result = result / samples
        else: result = self.hegdata[-1]
        return result
    
    def pause(self, pause=None):
        if pause == None:
            self.paused = not self.paused
        else:
            self.paused = pause
    
    def close(self):
        self.running = False
        if (self.lograw):
            self.rawlog.close()
        if (self.logdata):
            self.datalog.close()
        time.sleep(.1)
        i=0
        while i<20:
            try:
                if not type(self.comport) == bool:
                    self.comport.close()
                i=20
            except IOError: # concurrent read prevented close() attempt
                i+=1
                time.sleep(.1)
        

